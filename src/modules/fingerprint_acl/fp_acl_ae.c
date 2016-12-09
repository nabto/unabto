#include "fp_acl_ae.h"
#include "fp_acl.h"

#include <unabto/unabto_util.h>

struct fp_acl_db aclDb;

void fp_acl_ae_init(struct fp_acl_db* db)
{
    aclDb = *db;
}

bool read_fingerprint(buffer_read_t* read_buffer, fingerprint fp)
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
bool read_string_null_terminated(buffer_read_t* read_buffer, char* out, size_t outlen)
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

bool write_fingerprint(buffer_write_t* write_buffer, fingerprint fp)
{
    return unabto_query_write_uint8_list(write_buffer, fp, sizeof(fingerprint));
}

bool write_string(buffer_write_t* write_buffer, const char* string)
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

application_event_result write_user(buffer_write_t* write_buffer, uint8_t status, struct fp_acl_user* user)
{
    if (unabto_query_write_uint8(write_buffer, status) &&
        write_string(write_buffer, user->un) &&
        write_fingerprint(write_buffer, user->fp) &&
        unabto_query_write_uint32(write_buffer, user->perm))
    {
        return AER_REQ_RESPONSE_READY;
    } else {
        return AER_REQ_TOO_LARGE;
    } 
}

application_event_result write_empty_user(buffer_write_t* write_buffer, uint8_t status)
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

// getUser.json?fingerprint=<hex>
application_event_result fp_acl_ae_get_user(application_request* request,
                                            buffer_read_t* read_buffer,
                                            buffer_write_t* write_buffer)
{
    // all users with an acl entry can list users
    struct fp_acl_user requestUser;
    if (!get_user(request, &requestUser)) {
        return AER_REQ_NO_ACCESS;
    }

    // check acl
    
    fingerprint fp;
    if (!read_fingerprint(read_buffer, fp)) {
        return AER_REQ_TOO_SMALL;
    }

    void* it = aclDb.find(fp);
    struct fp_acl_user user;
    if (it != 0 || aclDb.load(it, &user) != FP_ACL_DB_OK) {
        return write_empty_user(write_buffer, FP_ACL_STATUS_NO_SUCH_USER);
    }

    return write_user(write_buffer, FP_ACL_STATUS_OK, &user);
}

//pairWithDevice.json?userName=<string>
application_event_result fp_acl_ae_pair_with_device(application_request* request,
                                                    buffer_read_t* read_buffer,
                                                    buffer_write_t* write_buffer)
{

    // require that you have a fingerprint to get paired
    if (! (request->connection && request->connection->hasFingerprint)) {
        return AER_REQ_NO_ACCESS;
    }
    
    struct fp_acl_settings aclSettings;
    if (!aclDb.load_settings(&aclSettings) != FP_ACL_DB_OK) {
        return AER_REQ_SYSTEM_ERROR;
    }

    if (!aclSettings.openForPairing) {
        return AER_REQ_NO_ACCESS;
    }
    
    struct fp_acl_user user;
        
    if (!read_string_null_terminated(read_buffer, user.un, USERNAME_MAX_LENGTH)) {
        return AER_REQ_TOO_SMALL;
    }
    
    memcpy(user.fp, request->connection->fingerprint, FP_LENGTH);
    user.perm = aclSettings.defaultPairingPermissions;

    fp_acl_db_status status = aclDb.save(&user);

    if (status == FP_ACL_DB_OK) {
        return write_user(write_buffer, FP_ACL_STATUS_OK, &user);
    } else {
        return write_empty_user(write_buffer, FP_ACL_STATUS_USER_DB_FULL);
    } 
}
