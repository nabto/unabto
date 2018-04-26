#include "fp_acl_ae_test.h"
#include <modules/fingerprint_acl/fp_acl_ae.h>
#include <modules/fingerprint_acl/fp_acl_memory.h>

struct fp_acl_db db;

#define DEFAULT_USER_PERMISSIONS (FP_ACL_PERMISSION_LOCAL_ACCESS | FP_ACL_PERMISSION_REMOTE_ACCESS)
#define FIRST_USER_PERMISSIONS (FP_ACL_PERMISSION_ALL)
#define SYSTEM_PERMISSIONS (FP_ACL_SYSTEM_PERMISSION_ALL)

bool init_empty_users()
{
    struct fp_acl_settings defaultSettings;
    defaultSettings.systemPermissions = SYSTEM_PERMISSIONS;
    defaultSettings.defaultUserPermissions = DEFAULT_USER_PERMISSIONS;
    defaultSettings.firstUserPermissions = FIRST_USER_PERMISSIONS;
    fp_mem_init(&db, &defaultSettings, NULL);
    fp_acl_ae_init(&db);
    return true;
}

bool init_users()
{
    if (!init_empty_users()) {
        return false;
    }
    {
        struct fp_acl_user user;
        memset(&user, 0, sizeof(struct fp_acl_user));
        user.fp.hasValue = 1;
        memset(user.fp.value.data, 42, 16);
        const char* name = "admin";
        memcpy(user.name, name, strlen(name)+1);
        user.permissions = FP_ACL_PERMISSION_ALL;
        db.save(&user);
    }
    int i;
    for (i = 0; i < 5; i++) {
        struct fp_acl_user user;
        memset(&user, 0, sizeof(struct fp_acl_user));
        user.fp.hasValue = 1;
        memset(user.fp.value.data, 128+i, 16);
        const char* name = "foobar";
        memcpy(user.name, name, strlen(name)+1);
        user.permissions = 0x42424242;
        db.save(&user);
    }
    return true;
}



void init_connection(nabto_connect* connection) {
    memset(connection, 0, sizeof(nabto_connect));
    connection->isLocal = true;
    connection->fingerprint.hasValue = true;
    memset(connection->fingerprint.value.data, 42, FP_ACL_FP_LENGTH);
}

void init_request(application_request* request, nabto_connect* connection)
{
    init_connection(connection);
    request->connection = connection;
    request->isLocal = connection->isLocal;
}

bool fp_acl_test_list_users()
{
    init_users();
    application_request request;
    nabto_connect connection;
    init_request(&request, &connection);

    uint8_t input[42];
    uint8_t output[256];

    memset(input, 0, 16);

    unabto_buffer in, out;

    unabto_buffer_init(&in, input, 42);
    unabto_buffer_init(&out, output, 512);

    unabto_query_request queryRequest;
    unabto_query_response queryResponse;

    unabto_query_request_init(&queryRequest, &in);
    unabto_query_response_init(&queryResponse, &out);

    application_event_result res = fp_acl_ae_users_get(&request,
                                                       &queryRequest,
                                                       &queryResponse);

    if (res != AER_REQ_RESPONSE_READY) {
        NABTO_LOG_ERROR(("expected AER_REQ_RESPONSE_READY, got %i", res));
        return false;
    }

    {
        // test the output
        unabto_query_request testOutput;
        unabto_query_request_init(&testOutput, &out);
        uint16_t listLength;
        unabto_query_read_uint16(&testOutput, &listLength);
        if (listLength != 6) {
            NABTO_LOG_ERROR(("expected listLength to be 6 it was, %i", listLength));
            return false;
        }
    }

    return true;
}




bool fp_acl_test_get_user()
{
    init_users();
    application_request req;
    nabto_connect connection;
    init_request(&req, &connection);

    uint8_t input[42];
    uint8_t output[256];

    // write fingerprint in input
    unabto_buffer in, out;
    unabto_buffer_init(&out, output, 256);
    unabto_buffer_init(&in, input, 42);

    {
        uint8_t fp[16];
        memset(fp, 42, 16);
        // write test data to input
        unabto_query_response writer;
        unabto_query_response_init(&writer, &in);
        unabto_query_write_uint8_list(&writer, fp, 16);
    }

    unabto_query_request queryRequest;
    unabto_query_response queryResponse;

    unabto_query_request_init(&queryRequest, &in);
    unabto_query_response_init(&queryResponse, &out);

    application_event_result res = fp_acl_ae_user_get(&req,
                                                      &queryRequest,
                                                      &queryResponse);
    if (res != AER_REQ_RESPONSE_READY) {
        NABTO_LOG_ERROR(("expected response to be ready"));
        return false;
    }

    {
        // test the output
        unabto_query_request testOutput;
        unabto_query_request_init(&testOutput, &out);
        uint8_t status;
        unabto_query_read_uint8(&testOutput, &status);
        if (status != 0) {
            NABTO_LOG_ERROR(("expected to return 0 users it was, %i", status));
            return false;
        }
    }

    return true;
}

bool fp_acl_test_get_me()
{
    init_users();
    application_request req;
    nabto_connect connection;
    init_request(&req, &connection);

    uint8_t buffer[256];
    memset(buffer,0, 256);
    unabto_buffer inout;
    unabto_buffer_init(&inout, buffer, 256);

    unabto_query_request queryRequest;
    unabto_query_response queryResponse;

    unabto_query_request_init(&queryRequest, &inout);
    unabto_query_response_init(&queryResponse, &inout);

    application_event_result res = fp_acl_ae_user_me(&req,
                                                      &queryRequest,
                                                      &queryResponse);
    if (res != AER_REQ_RESPONSE_READY) {
        NABTO_LOG_ERROR(("expected response to be ready"));
        return false;
    }

    {
        // test the output
        unabto_query_request testOutput;
        unabto_query_request_init(&testOutput, &inout);
        uint8_t status;
        unabto_query_read_uint8(&testOutput, &status);
        if (status != 0) {
            NABTO_LOG_ERROR(("expected to return 0 it was, %i", status));
            return false;
        }
    }

    return true;
}

bool fp_acl_test_user_remove()
{
    init_users();
    application_request req;
    nabto_connect connection;
    init_request(&req, &connection);

    uint8_t buffer[256];
    memset(buffer,0, 256);
    unabto_buffer inout;
    unabto_buffer_init(&inout, buffer, 256);

    {
        uint8_t fp[16];
        memset(fp, 42, 16);
        // write test data to input
        unabto_query_response writer;
        unabto_query_response_init(&writer, &inout);
        unabto_query_write_uint8_list(&writer, fp, 16);
    }
    

    unabto_query_request queryRequest;
    unabto_query_response queryResponse;

    unabto_query_request_init(&queryRequest, &inout);
    unabto_query_response_init(&queryResponse, &inout);

    application_event_result res = fp_acl_ae_user_remove(&req,
                                                         &queryRequest,
                                                         &queryResponse);
    if (res != AER_REQ_RESPONSE_READY) {
        NABTO_LOG_ERROR(("expected response to be ready"));
        return false;
    }

    {
        // test the output
        unabto_query_request testOutput;
        unabto_query_request_init(&testOutput, &inout);
        uint8_t status;
        unabto_query_read_uint8(&testOutput, &status);
        if (status != 0) {
            NABTO_LOG_ERROR(("expected to return 0 it was, %i", status));
            return false;
        }
    }

    return true;
}

bool fp_acl_pair(struct unabto_fingerprint* fp, const char* name, struct fp_acl_user* result)
{
    application_request req;
    nabto_connect connection;
    init_request(&req, &connection);
    memcpy(connection.fingerprint.value.data, fp->data, 16);

    uint8_t buffer[256];
    memset(buffer,0, 256);
    unabto_buffer inout;
    unabto_buffer_init(&inout, buffer, 256);
    {
        // write test data to input
        unabto_query_response writer;
        unabto_query_response_init(&writer, &inout);
        unabto_query_write_uint8_list(&writer, (uint8_t*)name, strlen(name));
    }

    unabto_query_request queryRequest;
    unabto_query_response queryResponse;

    unabto_query_request_init(&queryRequest, &inout);
    unabto_query_response_init(&queryResponse, &inout);

    application_event_result res = fp_acl_ae_pair_with_device(&req,
                                                              &queryRequest,
                                                              &queryResponse);
    if (res != AER_REQ_RESPONSE_READY) {
        NABTO_LOG_ERROR(("expected response to be ready it was %i", res));
        return false;
    }

    {
        // test the output
        unabto_query_request testOutput;
        unabto_query_request_init(&testOutput, &inout);
        uint8_t status;
        unabto_query_read_uint8(&testOutput, &status);
        if (status != 0) {
            NABTO_LOG_ERROR(("expected to return 0 it was, %i", status));
            return false;
        }
        uint8_t* list;
        uint16_t length;
        if (!unabto_query_read_uint8_list(&testOutput, &list, &length)) {
            return false;
        }
        if (length != FINGERPRINT_LENGTH) {
            return false;
        }
        memcpy(result->fp.value.data, list, FINGERPRINT_LENGTH);

        // read name
        if (!unabto_query_read_uint8_list(&testOutput, &list, &length)) {
            return false;
        }
        memcpy(result->name, list, length);
        
        if (!unabto_query_read_uint32(&testOutput, &result->permissions)) {
            return false;
        }
    }
    return true;
}

bool fp_acl_test_pair()
{
    init_empty_users();

    struct fp_acl_user firstUser;
    struct fp_acl_user secondUser;

    memset(&firstUser, 0, sizeof(struct fp_acl_user));
    memset(&secondUser, 0, sizeof(struct fp_acl_user));
    
    {
        // pair user 1
        struct unabto_fingerprint fp;
        memset(fp.data, 41, 16);
        
        if (!fp_acl_pair(&fp, "first", &firstUser)) {
            return false;
        }
    }

    {
        // pair second user
        struct unabto_fingerprint fp;
        memset(fp.data, 40, 16);
        
        if (!fp_acl_pair(&fp, "second", &secondUser)) {
            return false;
        }
    }

    if ( (firstUser.permissions != FP_ACL_PERMISSION_ALL) ||
         (strcmp(firstUser.name, "first") != 0) ||
         (firstUser.fp.value.data[0] != 41))
    {
        return false;
    }

    if ( (secondUser.permissions != (FP_ACL_PERMISSION_LOCAL_ACCESS | FP_ACL_PERMISSION_REMOTE_ACCESS)) ||
         (strcmp(secondUser.name, "second") != 0) ||
         (secondUser.fp.value.data[0] != 40))
    {
        return false;
    }
    
    return true;
}

bool fp_acl_test_set_name()
{
    init_users();
    application_request req;
    nabto_connect connection;
    init_request(&req, &connection);

    uint8_t buffer[256];
    memset(buffer,0, 256);
    unabto_buffer inout;
    unabto_buffer_init(&inout, buffer, 256);

    {
        uint8_t fp[16];
        memset(fp, 128+1, 16);
        
        const char* name = "newuser";
        // write test data to input
        unabto_query_response writer;
        unabto_query_response_init(&writer, &inout);
        unabto_query_write_uint8_list(&writer, fp, 16);
        unabto_query_write_uint8_list(&writer, (uint8_t*)name, strlen(name));
    }
    

    unabto_query_request queryRequest;
    unabto_query_response queryResponse;

    unabto_query_request_init(&queryRequest, &inout);
    unabto_query_response_init(&queryResponse, &inout);

    application_event_result res = fp_acl_ae_user_set_name(&req,
                                                           &queryRequest,
                                                           &queryResponse);
    if (res != AER_REQ_RESPONSE_READY) {
        NABTO_LOG_ERROR(("expected response to be ready it was %i", res));
        return false;
    }

    {
        // test the output
        unabto_query_request testOutput;
        unabto_query_request_init(&testOutput, &inout);
        uint8_t status;
        unabto_query_read_uint8(&testOutput, &status);
        if (status != 0) {
            NABTO_LOG_ERROR(("expected to return 0 it was, %i", status));
            return false;
        }
    }

    return true;
    
}

bool fp_acl_test_add_permissions()
{
    init_users();
    application_request req;
    nabto_connect connection;
    init_request(&req, &connection);

    uint8_t buffer[256];
    memset(buffer,0, 256);
    unabto_buffer inout;
    unabto_buffer_init(&inout, buffer, 256);

    {
        uint8_t fp[16];
        memset(fp, 128+1, 16);

        unabto_query_response writer;
        unabto_query_response_init(&writer, &inout);
        unabto_query_write_uint8_list(&writer, fp, 16);
        unabto_query_write_uint32(&writer, 0xfffffffful);
    }
    

    unabto_query_request queryRequest;
    unabto_query_response queryResponse;

    unabto_query_request_init(&queryRequest, &inout);
    unabto_query_response_init(&queryResponse, &inout);

    application_event_result res = fp_acl_ae_user_add_permissions(&req,
                                                                  &queryRequest,
                                                                  &queryResponse);
    if (res != AER_REQ_RESPONSE_READY) {
        NABTO_LOG_ERROR(("expected response to be ready it was %i", res));
        return false;
    }

    {
        // test the output
        unabto_query_request testOutput;
        unabto_query_request_init(&testOutput, &inout);
        uint8_t status;
        uint8_t* buffer;
        uint16_t bufferLength;
        unabto_query_read_uint8(&testOutput, &status);
        if (status != 0) {
            NABTO_LOG_ERROR(("expected to return 0 it was, %i", status));
            return false;
        }

        unabto_query_read_uint8_list(&testOutput, &buffer, &bufferLength);
        unabto_query_read_uint8_list(&testOutput, &buffer, &bufferLength);
        uint32_t permissions;
        unabto_query_read_uint32(&testOutput, &permissions);
        if (permissions != 0xfffffffful) {
            NABTO_LOG_ERROR(("failed to set permissions"));
            return false;
        }
        
    }

    return true;
    
}

bool fp_acl_test_remove_permissions()
{
    init_users();
    application_request req;
    nabto_connect connection;
    init_request(&req, &connection);

    uint8_t buffer[256];
    memset(buffer,0, 256);
    unabto_buffer inout;
    unabto_buffer_init(&inout, buffer, 256);

    {
        uint8_t fp[16];
        memset(fp, 128+1, 16);

        unabto_query_response writer;
        unabto_query_response_init(&writer, &inout);
        unabto_query_write_uint8_list(&writer, fp, 16);
        unabto_query_write_uint32(&writer, 0xfffffffful);
    }
    

    unabto_query_request queryRequest;
    unabto_query_response queryResponse;

    unabto_query_request_init(&queryRequest, &inout);
    unabto_query_response_init(&queryResponse, &inout);

    application_event_result res = fp_acl_ae_user_remove_permissions(&req,
                                                                     &queryRequest,
                                                                     &queryResponse);
    if (res != AER_REQ_RESPONSE_READY) {
        NABTO_LOG_ERROR(("expected response to be ready it was %i", res));
        return false;
    }

    {
        // test the output
        unabto_query_request testOutput;
        unabto_query_request_init(&testOutput, &inout);
        uint8_t status;
        uint8_t* buffer;
        uint16_t bufferLength;
        unabto_query_read_uint8(&testOutput, &status);
        if (status != 0) {
            NABTO_LOG_ERROR(("expected to return 0 it was, %i", status));
            return false;
        }

        unabto_query_read_uint8_list(&testOutput, &buffer, &bufferLength);
        unabto_query_read_uint8_list(&testOutput, &buffer, &bufferLength);
        uint32_t permissions;
        unabto_query_read_uint32(&testOutput, &permissions);
        if (permissions != 0x00000000ul) {
            NABTO_LOG_ERROR(("failed to remove permissions"));
            return false;
        }
        
    }

    return true;
    
}

bool fp_acl_test_system_get_acl_settings()
{
    init_users();
    application_request req;
    nabto_connect connection;
    init_request(&req, &connection);

    uint8_t buffer[256];
    memset(buffer,0, 256);
    unabto_buffer inout;
    unabto_buffer_init(&inout, buffer, 256);

    unabto_query_request queryRequest;
    unabto_query_response queryResponse;

    unabto_query_request_init(&queryRequest, &inout);
    unabto_query_response_init(&queryResponse, &inout);

    application_event_result res = fp_acl_ae_system_get_acl_settings(&req,
                                                                     &queryRequest,
                                                                     &queryResponse);
    if (res != AER_REQ_RESPONSE_READY) {
        NABTO_LOG_ERROR(("expected response to be ready it was %i", res));
        return false;
    }

    {
        // test the output
        unabto_query_request testOutput;
        unabto_query_request_init(&testOutput, &inout);
        uint8_t status;
        uint32_t systemPermissions;
        uint32_t defaultUserPermissions;
        unabto_query_read_uint8(&testOutput, &status);
        if (status != 0) {
            NABTO_LOG_ERROR(("expected to return 0 it was, %i", status));
            return false;
        }

        
        
        unabto_query_read_uint32(&testOutput, &systemPermissions);
        unabto_query_read_uint32(&testOutput, &defaultUserPermissions);

        if (systemPermissions != SYSTEM_PERMISSIONS) {
            return false;
        }

        if (defaultUserPermissions != DEFAULT_USER_PERMISSIONS) {
            return false;
        }
    }

    return true;
    
}

bool fp_acl_test_system_set_acl_settings()
{
    init_users();
    application_request req;
    nabto_connect connection;
    init_request(&req, &connection);

    uint8_t buffer[256];
    memset(buffer,0, 256);
    unabto_buffer inout;
    unabto_buffer_init(&inout, buffer, 256);

    {
        unabto_query_response writer;
        unabto_query_response_init(&writer, &inout);
        unabto_query_write_uint32(&writer, 0x00000000ul);
        unabto_query_write_uint32(&writer, 0x00000000ul);
    }
    
    unabto_query_request queryRequest;
    unabto_query_response queryResponse;

    unabto_query_request_init(&queryRequest, &inout);
    unabto_query_response_init(&queryResponse, &inout);

    application_event_result res = fp_acl_ae_system_set_acl_settings(&req,
                                                                     &queryRequest,
                                                                     &queryResponse);
    if (res != AER_REQ_RESPONSE_READY) {
        NABTO_LOG_ERROR(("expected response to be ready it was %i", res));
        return false;
    }

    {
        // test the output
        unabto_query_request testOutput;
        unabto_query_request_init(&testOutput, &inout);
        uint8_t status;
        uint32_t systemPermissions;
        uint32_t defaultUserPermissions;
        unabto_query_read_uint8(&testOutput, &status);
        if (status != 0) {
            NABTO_LOG_ERROR(("expected to return 0 it was, %i", status));
            return false;
        }

        
        
        unabto_query_read_uint32(&testOutput, &systemPermissions);
        unabto_query_read_uint32(&testOutput, &defaultUserPermissions);

        if (systemPermissions != 0x00000000ul) {
            return false;
        }

        if (defaultUserPermissions != 0x00000000ul) {
            return false;
        }
    }

    return true;
    
}

bool fp_acl_ae_test()
{
    init_users();

    return
        fp_acl_test_get_user() &&
        fp_acl_test_list_users() &&
        fp_acl_test_get_me() &&
        fp_acl_test_user_remove() &&
        fp_acl_test_pair() &&
        fp_acl_test_set_name() &&
        fp_acl_test_add_permissions() &&
        fp_acl_test_remove_permissions() &&
        fp_acl_test_system_get_acl_settings() &&
        fp_acl_test_system_set_acl_settings();
}

