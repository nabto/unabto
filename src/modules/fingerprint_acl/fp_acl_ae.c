#include "fp_acl_ae.h"
#include "fp_acl.h"

#include <unabto/unabto_util.h>

struct fp_acl_db aclDb;

void fp_acl_ae_init(struct fp_acl_db* db)
{
    aclDb = *db;
}

static bool read_fingerprint(unabto_query_request* read_buffer, fingerprint fp)
{
    uint8_t* list;
    uint16_t length;
    if (!unabto_query_read_uint8_list(read_buffer, &list, &length)) {
        return false;
    }
    if (length != sizeof(fingerprint)) {
        return false;
    }
    
    memcpy(fp, list, sizeof(fingerprint));
    return true;
}

// copy a string to out ensure it's null terminated.
static bool read_string_null_terminated(unabto_query_request* read_buffer, char* out, size_t outlen)
{
    uint8_t* list;
    uint16_t length;
    if (!unabto_query_read_uint8_list(read_buffer, &list, &length)) {
        return false;
    }

    memset(out, 0, outlen);
    
    memcpy(out, list, MIN(length, outlen-1));
    return true;
}

static bool write_fingerprint(unabto_query_response* write_buffer, fingerprint fp)
{
    return unabto_query_write_uint8_list(write_buffer, fp, sizeof(fingerprint));
}

static bool write_string(unabto_query_response* write_buffer, const char* string)
{
    return unabto_query_write_uint8_list(write_buffer, (uint8_t*)string, strlen(string));
}

bool get_user(application_request* request, struct fp_acl_user* user)
{
    if (request->connection && request->connection->hasFingerprint) {
        void* it = aclDb.find(request->connection->fingerprint);
        if (aclDb.load(it, user) == FP_ACL_DB_OK) {
            return true;
        } 
    }
    return false;
}

application_event_result write_user(unabto_query_response* write_buffer, uint8_t status, struct fp_acl_user* user)
{
    if (unabto_query_write_uint8(write_buffer, status) &&
        write_fingerprint(write_buffer, user->fp) &&
        write_string(write_buffer, user->name) &&
        unabto_query_write_uint32(write_buffer, user->permissions))
    {
        return AER_REQ_RESPONSE_READY;
    } else {
        return AER_REQ_TOO_LARGE;
    } 
}

application_event_result write_empty_user(unabto_query_response* write_buffer, uint8_t status)
{
    if (unabto_query_write_uint8(write_buffer, status) &&
        write_string(write_buffer, "") &&
        write_string(write_buffer, "") &&
        unabto_query_write_uint32(write_buffer, 0))
    {
        return AER_REQ_RESPONSE_READY;
    } else {
        return AER_REQ_TOO_LARGE;
    }
}

application_event_result write_settings(unabto_query_response* response, struct fp_acl_settings* settings)
{
    if (unabto_query_write_uint8(response, FP_ACL_STATUS_OK) &&
        unabto_query_write_uint32(response, settings->systemPermissions) &&
        unabto_query_write_uint32(response, settings->defaultPermissions))
    {
        return AER_REQ_RESPONSE_READY;
    } else {
        return AER_REQ_TOO_LARGE;
    }
    
}

// getUsers.json?count=nn&start=nn
// if count is 0 fill as many as possible into the response.
// if start is 0 start from the beginning
// if more users exists than returned set next to != 0.
application_event_result fp_acl_ae_users_get(application_request* request,
                                             unabto_query_request* read_buffer,
                                             unabto_query_response* write_buffer)
{
    if (!fp_acl_is_request_allowed(request, FP_ACL_PERMISSION_NONE)) {
        return AER_REQ_NO_ACCESS;
    }

    uint8_t count;
    uint32_t start;

    if (! (unabto_query_read_uint8(read_buffer, &count) &&
           unabto_query_read_uint32(read_buffer, &start)))
    {
        return AER_REQ_TOO_SMALL;
    }

    if (count == 0) {
        count = 255;
    }

    // rough max extimate of the size of an encoded user.
    size_t aclResponseSize = 2 + FP_ACL_USERNAME_MAX_LENGTH + 2 + FP_ACL_FP_LENGTH + 4 + 4;
    
    void* it = aclDb.first();

    uint32_t offset = 0;
    // move pointer forward
    while (it != NULL && offset < start) {
        it = aclDb.next(it);
        offset++;
    } 

    {
        unabto_list_ctx listCtx;
        uint16_t elements = 0;
        if (!unabto_query_write_list_start(write_buffer, &listCtx)) {
            return AER_REQ_TOO_LARGE;
        }
        
        while (it != NULL &&
               elements < count &&
               (unabto_query_write_free_bytes(write_buffer) > aclResponseSize))
        {
            struct fp_acl_user user;
            if (aclDb.load(it, &user) != FP_ACL_DB_OK) {
                return AER_REQ_SYSTEM_ERROR;
            }
            if (! (write_fingerprint(write_buffer, user.fp) &&
                   write_string(write_buffer, user.name) &&
                   unabto_query_write_uint32(write_buffer, user.permissions)))
            {
                return AER_REQ_TOO_SMALL;
            }
            elements++;
            offset++;
            it = aclDb.next(it);
        }

        if (!unabto_query_write_list_end(write_buffer, &listCtx, elements)) {
            return AER_REQ_SYSTEM_ERROR;
        }

        if (it == NULL) {
            offset = 0;
        }
        if (!unabto_query_write_uint32(write_buffer, offset)) {
            return AER_REQ_TOO_LARGE;
        }
    }
    
    return AER_REQ_RESPONSE_READY;
    
}




// getUser.json?fingerprint=<hex>
application_event_result fp_acl_ae_user_get(application_request* request,
                                            unabto_query_request* read_buffer,
                                            unabto_query_response* write_buffer)
{
    if (!fp_acl_is_request_allowed(request, FP_ACL_PERMISSION_NONE)) {
        return AER_REQ_NO_ACCESS;
    }
    
    fingerprint fp;
    if (!read_fingerprint(read_buffer, fp)) {
        return AER_REQ_TOO_SMALL;
    }

    void* it = aclDb.find(fp);
    struct fp_acl_user user;
    if (it == 0 || aclDb.load(it, &user) != FP_ACL_DB_OK) {
        return write_empty_user(write_buffer, FP_ACL_STATUS_NO_SUCH_USER);
    }

    return write_user(write_buffer, FP_ACL_STATUS_OK, &user);
}

application_event_result fp_acl_ae_user_me(application_request* request,
                                           unabto_query_request* read_buffer,
                                           unabto_query_response* write_buffer)
{
    if (!fp_acl_is_request_allowed(request, FP_ACL_PERMISSION_NONE)) {
        return AER_REQ_NO_ACCESS;
    }

    void* it = aclDb.find(request->connection->fingerprint);
    struct fp_acl_user user;
    if (it == 0 || aclDb.load(it, &user) != FP_ACL_DB_OK) {
        return write_empty_user(write_buffer, FP_ACL_STATUS_NO_SUCH_USER);
    }

    return write_user(write_buffer, FP_ACL_STATUS_OK, &user);    
}

application_event_result fp_acl_ae_user_remove(application_request* request,
                                               unabto_query_request* read_buffer,
                                               unabto_query_response* write_buffer)
{
    if (!fp_acl_is_request_allowed(request, FP_ACL_PERMISSION_ADMIN)) {
        return AER_REQ_NO_ACCESS;
    }

    fingerprint fp;
    if (!read_fingerprint(read_buffer, fp)) {
        return AER_REQ_TOO_SMALL;
    }

    void* it = aclDb.find(fp);
    struct fp_acl_user user;
    uint8_t status = FP_ACL_STATUS_OK;
    if (it == 0 || aclDb.load(it, &user) != FP_ACL_DB_OK) {
        status = FP_ACL_STATUS_NO_SUCH_USER;
    } else {
        if (aclDb.remove(it) != FP_ACL_DB_OK) {
            status = FP_ACL_STATUS_REMOVE_FAILED;
        }
    }
    if (!unabto_query_write_uint8(write_buffer, status)) {
        return AER_REQ_TOO_LARGE;
    }
    return AER_REQ_RESPONSE_READY;
}


application_event_result fp_acl_ae_user_set_name(application_request* request,
                                                 unabto_query_request* read_buffer,
                                                 unabto_query_response* write_buffer)
{
    if (!fp_acl_is_request_allowed(request, FP_ACL_PERMISSION_ADMIN)) {
        return AER_REQ_NO_ACCESS;
    }

    fingerprint fp;
    if (!read_fingerprint(read_buffer, fp)) {
        return AER_REQ_TOO_SMALL;
    }

    void* it = aclDb.find(fp);
    struct fp_acl_user user;
    if (it == 0 || aclDb.load(it, &user) != FP_ACL_DB_OK) {
        return write_empty_user(write_buffer, FP_ACL_STATUS_NO_SUCH_USER);
    }
        
    if (!read_string_null_terminated(read_buffer, user.name, FP_ACL_USERNAME_MAX_LENGTH)) {
        return AER_REQ_TOO_SMALL;
    }
    
    uint8_t status = FP_ACL_STATUS_OK;
    if (aclDb.save(&user) != FP_ACL_DB_OK) {
        status = FP_ACL_STATUS_SAVE_FAILED;
    }
    // reload user
    it = aclDb.find(fp);
    if (it == 0 || aclDb.load(it, &user) != FP_ACL_DB_OK) {
        return write_empty_user(write_buffer, FP_ACL_STATUS_NO_SUCH_USER);
    }

    return write_user(write_buffer, status, &user);
}

application_event_result fp_acl_ae_user_alter_permissions(application_request* request,
                                                          unabto_query_request* read_buffer,
                                                          unabto_query_response* write_buffer,
                                                          void (*alterPermissionsFunction)(struct fp_acl_user* user, uint32_t permissions))
{
    if (!fp_acl_is_request_allowed(request, FP_ACL_PERMISSION_ADMIN)) {
        return AER_REQ_NO_ACCESS;
    }

    fingerprint fp;
    if (!read_fingerprint(read_buffer, fp)) {
        return AER_REQ_TOO_SMALL;
    }

    uint32_t permissions;
    if (!unabto_query_read_uint32(read_buffer, &permissions)) {
        return AER_REQ_TOO_SMALL;
    }

    void* it = aclDb.find(fp);
    struct fp_acl_user user;
    if (it == 0 || aclDb.load(it, &user) != FP_ACL_DB_OK) {
        return write_empty_user(write_buffer, FP_ACL_STATUS_NO_SUCH_USER);
    }

    alterPermissionsFunction(&user, permissions);

    uint8_t status = FP_ACL_STATUS_OK;
    
    if (aclDb.save(&user) != FP_ACL_DB_OK) {
        status = FP_ACL_STATUS_SAVE_FAILED;
    }

    // reload user
    it = aclDb.find(fp);
    if (it == 0 || aclDb.load(it, &user) != FP_ACL_DB_OK) {
        return write_empty_user(write_buffer, FP_ACL_STATUS_NO_SUCH_USER);
    }

    return write_user(write_buffer, status, &user);
}


application_event_result fp_acl_ae_user_add_permissions(application_request* request,
                                                        unabto_query_request* read_buffer,
                                                        unabto_query_response* write_buffer)
{
    return fp_acl_ae_user_alter_permissions(request, read_buffer, write_buffer, &fp_acl_user_add_permissions);
}

application_event_result fp_acl_ae_user_remove_permissions(application_request* request,
                                                           unabto_query_request* read_buffer,
                                                           unabto_query_response* write_buffer)
{
    return fp_acl_ae_user_alter_permissions(request, read_buffer, write_buffer, &fp_acl_user_remove_permissions);
}

application_event_result fp_acl_ae_user_set_permissions(application_request* request,
                                                           unabto_query_request* read_buffer,
                                                           unabto_query_response* write_buffer)
{
    return fp_acl_ae_user_alter_permissions(request, read_buffer, write_buffer, &fp_acl_user_set_permissions);
}


// pairing functions
//pairWithDevice.json?userName=<string>
application_event_result fp_acl_ae_pair_with_device(application_request* request,
                                                    unabto_query_request* read_buffer,
                                                    unabto_query_response* write_buffer)
{
    
    if (!fp_acl_is_pair_allowed(request)) {
        return AER_REQ_NO_ACCESS;
    }
    
    struct fp_acl_user user;
        
    if (!read_string_null_terminated(read_buffer, user.name, FP_ACL_USERNAME_MAX_LENGTH)) {
        return AER_REQ_TOO_SMALL;
    }
    
    struct fp_acl_settings aclSettings;
    if (aclDb.load_settings(&aclSettings) != FP_ACL_DB_OK) {
        return AER_REQ_SYSTEM_ERROR;
    }

    memcpy(user.fp, request->connection->fingerprint, FP_ACL_FP_LENGTH);
    user.permissions = aclSettings.defaultPermissions;

    if (aclDb.first() == NULL) {
        // this is the first user make her admin!
        user.permissions |= FP_ACL_PERMISSION_ADMIN;
    }

    fp_acl_db_status status = aclDb.save(&user);

    if (status == FP_ACL_DB_OK) {
        return write_user(write_buffer, FP_ACL_STATUS_OK, &user);
    } else {
        return write_empty_user(write_buffer, FP_ACL_STATUS_USER_DB_FULL);
    } 
}


application_event_result fp_acl_ae_system_get_acl_settings(application_request* request,
                                                           unabto_query_request* read_buffer,
                                                           unabto_query_response* write_buffer)
{
    if (!fp_acl_is_request_allowed(request, FP_ACL_PERMISSION_ADMIN)) {
        return AER_REQ_NO_ACCESS;
    }

    struct fp_acl_settings aclSettings;
    if (aclDb.load_settings(&aclSettings) != FP_ACL_DB_OK) {
        return AER_REQ_SYSTEM_ERROR;
    }

    return write_settings(write_buffer, &aclSettings);
}


application_event_result fp_acl_ae_system_set_acl_settings(application_request* request,
                                                           unabto_query_request* read_buffer,
                                                           unabto_query_response* write_buffer)
{
    if (!fp_acl_is_request_allowed(request, FP_ACL_PERMISSION_ADMIN)) {
        return AER_REQ_NO_ACCESS;
    }

    struct fp_acl_settings aclSettings;
    
    if (! (unabto_query_read_uint32(read_buffer, &aclSettings.systemPermissions) &&
           unabto_query_read_uint32(read_buffer, &aclSettings.defaultPermissions)))
    {
        return AER_REQ_TOO_SMALL;
    }

    if (aclDb.save_settings(&aclSettings) != FP_ACL_DB_OK) {
        return AER_REQ_SYSTEM_ERROR;
    }
    
    if (aclDb.load_settings(&aclSettings) != FP_ACL_DB_OK) {
        return AER_REQ_SYSTEM_ERROR;
    }

    return write_settings(write_buffer, &aclSettings);
}


// Helper function
bool fp_acl_is_request_allowed(application_request* request, uint32_t requiredPermissions)
{
    if (! (request->connection && request->connection->hasFingerprint)) {
        return false;
    }
    
    struct fp_acl_user user;
    void* it = aclDb.find(request->connection->fingerprint);
    if (it == NULL || aclDb.load(it, &user) != FP_ACL_DB_OK) {
        return false;
    }

    return fp_acl_check_user_permissions(&user, request->isLocal, requiredPermissions);
}

bool fp_acl_is_pair_allowed(application_request* request)
{
    if (! (request->connection && request->connection->hasFingerprint)) {
        return false;
    }

    struct fp_acl_settings aclSettings;
    if (aclDb.load_settings(&aclSettings) != FP_ACL_DB_OK) {
        return false;
    }
    
    if (request->connection->isLocal) {
        if (fp_acl_check_system_permissions(&aclSettings, FP_ACL_SYSTEM_PERMISSION_PAIRING)) {
            return true;
        }
    }
    return false;
}

bool fp_acl_is_connection_allowed(nabto_connect* connection)
{
    if (! (connection && connection->hasFingerprint)) {
        return false;
    }

    struct fp_acl_settings aclSettings;
    if (aclDb.load_settings(&aclSettings) != FP_ACL_DB_OK) {
        return false;
    }

    void* user = aclDb.find(connection->fingerprint);

    uint32_t requiredSystemPermissions = FP_ACL_SYSTEM_PERMISSION_NONE;
    uint32_t requiredUserPermissions = FP_ACL_PERMISSION_NONE;
    
    if (connection->isLocal) {
        requiredSystemPermissions |= FP_ACL_SYSTEM_PERMISSION_LOCAL_ACCESS;
        if (!user) {
            // if no user exists require that we are in pairing mode.
            requiredSystemPermissions |= FP_ACL_SYSTEM_PERMISSION_PAIRING;
        }
    } else {
        requiredSystemPermissions |= FP_ACL_SYSTEM_PERMISSION_REMOTE_ACCESS;
        if (!user) {
            return false;
        }
    }

    if (!fp_acl_check_system_permissions(&aclSettings, requiredSystemPermissions)) {
        return false;
    }

    if (user) {
        struct fp_acl_user u;
        if (aclDb.load(user, &u) != FP_ACL_DB_OK) {
            return false;
        }
        return fp_acl_check_user_permissions(&u, connection->isLocal, requiredUserPermissions);
    }
    return true;
}

    

    
