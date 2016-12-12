#include "fp_acl_ae_test.h"
#include <modules/fingerprint_acl/fp_acl_ae.h>
#include <modules/fingerprint_acl/fp_acl_memory.h>

struct fp_acl_db db;
bool init_users()
{
    fp_mem_init(&db);
    fp_acl_ae_init(&db);

    {
        struct fp_acl_user user;
        memset(&user, 0, sizeof(struct fp_acl_user));
        memset(user.fp, 42, 16);
        const char* name = "admin";
        memcpy(user.name, name, strlen(name)+1);
        user.permissions = FP_ACL_PERMISSION_ALL;
        db.save(&user);
    }

    for (int i = 0; i < 5; i++) {
        struct fp_acl_user user;
        memset(&user, 0, sizeof(struct fp_acl_user));
        memset(user.fp, 128+i, 16);
        const char* name = "foobar";
        memcpy(user.name, name, strlen(name)+1);
        user.permissions = 0x42424242;
        db.save(&user);
    }
    return true;
}

void init_connection(nabto_connect* connection) {
    memset(connection, 0, sizeof(nabto_connect));
    connection->isLocal = false;
    connection->hasFingerprint = true;
    memset(connection->fingerprint, 42, FP_LENGTH);
}

bool fp_acl_test_list_users()
{
    application_request request;
    nabto_connect connection;
    init_connection(&connection);
    request.connection = &connection;

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




bool fp_acl_test_get_user() {
    application_request req;
    nabto_connect connection;
    init_connection(&connection);
    req.connection = &connection;

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

bool fp_acl_test_get_me() {
    application_request req;
    nabto_connect connection;
    init_connection(&connection);
    req.connection = &connection;

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


bool fp_acl_ae_test()
{
    init_users();

    return
        fp_acl_test_get_user() &&
        fp_acl_test_list_users() &&
        fp_acl_test_get_me();
}

