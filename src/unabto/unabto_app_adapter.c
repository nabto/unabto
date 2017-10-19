/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
/**
 * @file
 * Nabto uServer Application Framework Event Handling - Implementation.
 */

#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_UNABTO_APPLICATION

#include <unabto/unabto_env_base.h>

#if NABTO_ENABLE_CONNECTIONS

#include "unabto_connection.h"
#include "unabto_app_adapter.h"
#include "unabto_logging.h"
#include "unabto_util.h"
#include <unabto/unabto_memory.h>
#include <unabto/unabto_packet.h>

#include <string.h>
#include <stdlib.h>

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/**
 * Poll for a response for a request in the application.
 * Recreate request data from handle and call #application_poll().
 * Returns AER_REQ_RESPONSE_READY if a response is ready for the given
 * request. In that case buf will contain the response and olen the number
 * of bytes written to buf.
 * All other return values are error codes. In that case all buffers
 * are unmodified. */
static bool framework_get_async_response(struct naf_handle_s *handle);

/**
 * Release the event handle returned by #framework_event_query().
 * @param   handle returned by framework_event_query
 * 
 * Upon return of this function, the handle is invalid. Setting the handle
 * to 0 will prevent the handle from being misused afterwards.
 */
static void framework_release_handle(struct naf_handle_s* handle);

#endif

// Store the state for requests.
static NABTO_THREAD_LOCAL_STORAGE struct naf_handle_s handles[NABTO_APPREQ_QUEUE_SIZE];

// query list of requests
struct naf_handle_s* find_existing_request_handle(nabto_connect* con, uint16_t reqId);
struct naf_handle_s* find_handle(application_request *req);
struct naf_handle_s* find_free_handle();
void free_all_handles_for_connection(nabto_connect* connection);

/******************************/
/* Logging of app/queue state */
/******************************/
#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_TRACE)
static text result_s(application_event_result result);
#endif


void init_application_event_framework(void)
{
    memset(handles, 0, sizeof(struct naf_handle_s));
}

void framework_connection_released(nabto_connect* connection)
{
    free_all_handles_for_connection(connection);
}

naf_query_status framework_event_query(nabto_connect* con, nabto_packet_header* hdr, struct naf_handle_s** handle)
{
    uint16_t reqId = hdr->seq;
    NABTO_LOG_TRACE(("APPREQ framework_event_query: reqId: %" PRIu16, reqId));

    // if found do not allocate new request
    if (find_existing_request_handle(con, reqId) != NULL) {
        return NAF_QUERY_QUEUED;
    }

    /* Now the request was not found.
     * Reserve the next free entry in the queue. */
    *handle = find_free_handle();
    if (!*handle) {
        NABTO_LOG_TRACE(("Request queue is full"));
        return NAF_QUERY_OUT_OF_RESOURCES;
    }

    (*handle)->connection = con;
    (*handle)->header = *hdr;

    return NAF_QUERY_NEW;
}


/* Handle application event.
 * Initialize the handle and call applicaion_event. */
void framework_event(struct naf_handle_s* handle,
                     uint8_t*             iobuf,
                     uint16_t             ilen)
{
    unabto_buffer                 w_buf;
    unabto_query_response         queryResponse;
    unabto_buffer                 r_buf;
    unabto_query_request          queryRequest;
    application_event_result      res;
    uint32_t                      query_id;
    nabto_connect*                con = handle->connection;
    application_request*          req = &handle->applicationRequest;
    uint8_t* buf = nabtoCommunicationBuffer;
    uint8_t* end = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;

    /* Set up a write buffer to write into iobuf. */
    uint16_t availableForData = unabto_crypto_max_data(&con->cryptoctx, (uint16_t)(end - iobuf));
    unabto_buffer_init(&w_buf, iobuf, MIN(availableForData, NABTO_RESPONSE_MAX_SIZE));
    unabto_query_response_init(&queryResponse, &w_buf);

    /* Set up a read buffer that reads from iobuf. */
    unabto_buffer_init(&r_buf, iobuf, ilen);
    unabto_query_request_init(&queryRequest, &r_buf);


    if (unabto_query_request_size(&queryRequest) > NABTO_REQUEST_MAX_SIZE) {
        send_exception(con, &handle->header, AER_REQ_TOO_LARGE);
        return;
    }
    if (!unabto_query_read_uint32(&queryRequest, &query_id)) {
        send_exception(con, &handle->header, AER_REQ_NO_QUERY_ID);
        return;
    }

    /* Initialize application request info */
    req->isLocal = con->isLocal;
    req->isLegacy = false;
    req->connection = con;
    req->clientId = (const char *) con->clientId;
    req->queryId = query_id;

    NABTO_LOG_TRACE(("APPREQ application_event: %" PRIu16, query_id));
    res = application_event(req, &queryRequest, &queryResponse);
    NABTO_LOG_TRACE(("APPREQ application_event: result=%" PRItext, result_s(res)));

    /* Handle response */
    switch (res) {
        case AER_REQ_RESPONSE_READY:
            add_flags(buf, NP_PACKET_HDR_FLAG_RESPONSE);
            send_and_encrypt_packet_con(con, buf, end, iobuf, unabto_query_response_used(&queryResponse), iobuf - SIZE_CODE - NP_PAYLOAD_HDR_BYTELENGTH);
            break;

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
        case AER_REQ_ACCEPTED:
            handle->state = APPREQ_USED;
            break;
#endif

        default:
            if (con->cpAsync) {
                send_exception(con, &handle->header, res);
            }
            break;
    }
}

#if !NABTO_APPLICATION_EVENT_MODEL_ASYNC
bool framework_event_poll()
{
    return false;
}
#endif

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/* Poll a pending event.
 * Returns true iff a response is sent to the client
 */
bool framework_event_poll()
{
    application_request     *req;
    struct naf_handle_s     *handle;

    //Ask for the request in the application
    req = NULL;
    if (!application_poll_query(&req)) {
        // The application isn't ready with a response yet, poll again later
        return false;
    }

    handle = find_handle(req);

    if (!handle) {
        NABTO_LOG_ERROR(("Cannot find handle matching req"));
        application_poll_drop(req);
        return false;
    }

    bool status = framework_get_async_response(handle);
    framework_release_handle(handle);
    return status;
}
#endif

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/* Handle event by polling application - use data from header stored in handle */
bool framework_get_async_response(struct naf_handle_s *handle)
{
    unabto_buffer              w_buf;
    unabto_query_response      queryResponse;
    uint8_t                   *ptr;
    application_event_result   res;
    uint8_t*                   buf = nabtoCommunicationBuffer;
    uint8_t*                   end = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;
    nabto_connect*             con = handle->connection;
    ptr = buf;
    
    /* Write packet header and crypto payload header into buf. */
    ptr = nabto_wr_header(buf, end, &handle->header);
    if (ptr == NULL) {
        return false;
    }
    add_flags(buf, NP_PACKET_HDR_FLAG_RESPONSE);

    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_CRYPTO, NULL, 0);
    if (ptr == NULL) {
        return false;
    }
    ptr += 2; // skip over crypto code
    if (ptr > end) {
        return false;
    }
    
    /* Set up a write buffer to write into buf (after crypto payload header). */
    uint16_t availableForData = unabto_crypto_max_data(&con->cryptoctx, (uint16_t)(end - ptr));
    unabto_buffer_init(&w_buf, ptr, MIN(availableForData, NABTO_RESPONSE_MAX_SIZE));
    unabto_query_response_init(&queryResponse, &w_buf);

    res = application_poll(&handle->applicationRequest, &queryResponse);
    NABTO_LOG_TRACE(("APPREQ application_poll: result=%" PRItext, result_s(res)));

    bool status;
    if (res != AER_REQ_RESPONSE_READY) {
        status = send_exception(handle->connection, &handle->header, res);
    } else {
        status = send_and_encrypt_packet_con(handle->connection, buf, end, ptr, unabto_query_response_used(&queryResponse), ptr - 6);
    }

    return status;
}
#endif


struct naf_handle_s* find_existing_request_handle(nabto_connect* con, uint16_t reqId) {
    int i;
    for (i = 0; i < NABTO_APPREQ_QUEUE_SIZE; i++) {
        struct naf_handle_s* h = &handles[i];
        if (h->state == APPREQ_USED && h->connection == con && h->header.seq == reqId) {
            return h;
        }
    }
    return NULL;
}

struct naf_handle_s* find_handle(application_request *req)
{
    int i;
    for (i = 0; i < NABTO_APPREQ_QUEUE_SIZE; i++) {
        struct naf_handle_s* h = &handles[i];
        if (h->state == APPREQ_USED && &h->applicationRequest == req) {
            return h;
        }
    }
    return NULL;
}

struct naf_handle_s* find_free_handle()
{
    int i;
    for (i = 0; i < NABTO_APPREQ_QUEUE_SIZE; i++) {
        struct naf_handle_s* h = &handles[i];
        if (h->state == APPREQ_FREE) {
            return h;
        }
    }
    return NULL;
}

void free_all_handles_for_connection(nabto_connect* connection) {
#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
    int i;
    for (i = 0; i < NABTO_APPREQ_QUEUE_SIZE; i++) {
        struct naf_handle_s* h = &handles[i];
        if (h->state == APPREQ_USED && h->connection == connection) {
            framework_release_handle(h);
        }
    }
#endif
}


#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/**
 * Release the event handle returned by framework_event_query.
 */
void framework_release_handle(struct naf_handle_s* handle)
{
    if (!handle) {
        return;
    }
    application_poll_drop(&handle->applicationRequest);
    handle->state = APPREQ_FREE;
}
#endif


#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_TRACE)
text result_s(application_event_result result)
{
    switch (result) {
        case AER_REQ_RESPONSE_READY:   return "RESPONSE_READY";
#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
        case AER_REQ_ACCEPTED:         return "ACCEPTED";
#endif
        case AER_REQ_NOT_READY:        return "NOT_READY";
        case AER_REQ_NO_ACCESS:        return "NO_ACCESS";
        case AER_REQ_TOO_SMALL:        return "TOO_SMALL";
        case AER_REQ_TOO_LARGE:        return "TOO_LARGE";
        case AER_REQ_INV_QUERY_ID:     return "INV_QUERY_ID";
        case AER_REQ_RSP_TOO_LARGE:    return "RSP_TOO_LARGE";
        case AER_REQ_OUT_OF_RESOURCES: return "OUT_OF_RESOURCES";
        case AER_REQ_SYSTEM_ERROR:     return "SYSTEM_ERROR";
        case AER_REQ_NO_QUERY_ID:      return "NO_QUERY_ID";
    }
    return "??";
}
#endif

#endif
