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



/************************************************/
/* Local prototypes (both SYNC and ASYNC model) */
/************************************************/

/**
 * Try to handle application event the first time.
 * Initialize request data in handle and call #application_event().
 * Returns AER_REQ_RESPONSE_READY if a response is ready for the given
 * request. In that case w_b will contain the response.
 * If this function fails all buffers are unmodified. */
static application_event_result framework_first_event(struct naf_handle_s*       handle,
                                                      unabto_query_request*      queryRequest,
                                                      unabto_query_response*     queryResponse,
                                                      nabto_connect*             con,
                                                      const nabto_packet_header* hdr);

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/**
 * Poll for a response for a request in the application.
 * Recreate request data from handle and call #application_poll().
 * Returns AER_REQ_RESPONSE_READY if a response is ready for the given
 * request. In that case buf will contain the response and olen the number
 * of bytes written to buf.
 * All other return values are error codes. In that case all buffers
 * are unmodified. */
static bool framework_poll_event(struct naf_handle_s *handle);
#endif

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/**
 * Build/reconstruct the first part of the response (nabto packet header
 * and crypto payload header).
 * @param  buf     the start of destination buffer
 * @param  hdr     the nabto packet header stored in the application queue
 * @return         pointer to the end of the written header. NULL on error.
 */
static uint8_t* reconstruct_header(uint8_t* buf, uint8_t* end, const nabto_packet_header * hdr);
#endif

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/**
 * Writes a exception packet into given buffer.
 * Returns true if the exception packet has been written to the buffer. In
 * that case buf will contain the response and olen the number of bytes
 * written to buf.
 * Returns false on error. In that case all buffers are unmodified. */
static bool framework_write_exception(nabto_connect             *con,
                                      application_event_result   code,
                                      const nabto_packet_header *hdr,
                                      uint8_t                   *buf,
                                      uint8_t                   *end);
#endif

/******************************************************************/
/* Logging of application request info (both SYNC and ASYNC model */
/******************************************************************/
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#define PRI_client_id_q         "%" PRIptr " ('%s'), req=%" PRIu16 ", queryId=%" PRIu32
#define CLIENT_ID_Q_ARGS(c,s,q) (c), (c), (s), (q)
#endif //DOXYGEN_SHOULD_SKIP_THIS


static NABTO_THREAD_LOCAL_STORAGE struct naf_handle_s handles[NABTO_APPREQ_QUEUE_SIZE];


// query list of requests
struct naf_handle_s* find_existing_request_handle(nabto_connect* con, uint16_t reqId);
struct naf_handle_s* find_handle(application_request *req);
struct naf_handle_s* find_free_handle();

/******************************/
/* Logging of app/queue state */
/******************************/
#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_TRACE)
static text result_s(application_event_result result);
#endif


void init_application_event_framework(void) {
    memset(handles, 0, sizeof(struct naf_handle_s));
}

/******************************************************************************/
/**********  ASYNCHRONIOUS EVENT HANDLING IMPLEMENTATION   ********************/
/******************************************************************************/

/* Ask for an event handle corresponding to the given client request (in
 * ASYNC mode). The event handle must be released again using the
 * framework_release_handle function.
 * The return value is NAF_QUERY_NEW if this is the first time the client
 * request is seen (i.e. a new event handle is created).
 * The return value is NAF_QUERY_QUEUED if the client request is already
 * known (and is pending to be processed).
 * The return value is NAF_QUERY_OUT_OF_RESOURCES if no more memory is
 * available for a new client request.
 */
naf_query framework_event_query(nabto_connect* con, nabto_packet_header* hdr, struct naf_handle_s** handle)
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

    (*handle)->state = APPREQ_USED;
    (*handle)->connection = con;
    (*handle)->header = *hdr;

    return NAF_QUERY_NEW;
}

/* Release the event handle returned by framework_event_query.
 * In the ASYNC model we must remove the event handle from the FIFO request
 * queue. */
void framework_release_handle(struct naf_handle_s* handle)
{
    if (!handle) {
        return;
    }

    handle->state = APPREQ_FREE;
}

/* Handle application event.
 * Initialize the handle and call applicaion_event. */
application_event_result framework_event(struct naf_handle_s* handle,
                                         uint8_t*             iobuf,
                                         uint8_t*             end,
                                         uint16_t             ilen)
{
    unabto_buffer                 w_buf;
    unabto_query_response         queryResponse;
    unabto_buffer                 r_buf;
    unabto_query_request          queryRequest;
    application_event_result      res;
    uint32_t                      query_id;
    nabto_connect*                con = handle->connection;
    application_request* req = &handle->applicationRequest;
    uint8_t* buf = nabtoCommunicationBuffer;

    handle->spnsi = handle->connection->spnsi;
    
    /* Set up a write buffer to write into iobuf. */
    unabto_buffer_init(&w_buf, iobuf, (end - iobuf));
    unabto_query_response_init(&queryResponse, &w_buf);

    /* Set up a read buffer that reads from iobuf. */
    unabto_buffer_init(&r_buf, iobuf, ilen);
    unabto_query_request_init(&queryRequest, &r_buf);


    if (unabto_query_request_size(&queryRequest) > NABTO_REQUEST_MAX_SIZE) {
        return AER_REQ_TOO_LARGE;
    }
    if (!unabto_query_read_uint32(&queryRequest, &query_id)) {
        return AER_REQ_NO_QUERY_ID;
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
            // do nothing let the request be handled in the async queue
            return res;
#endif

        default:
            if (con->cpAsync) {
                send_exception(con, &handle->header, res);
            }
            break;
    }
    return res;
}

#if !NABTO_APPLICATION_EVENT_MODEL_ASYNC
bool framework_event_poll()
{
    return false;
}
#else
/* Poll a pending event.
 * Returns true if a response is ready (from any event in the queue).
 * If this function fails (returns false) all buffers are unmodified. */
bool framework_event_poll()
{
    application_event_result ret;
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
        framework_release_handle(handle);
        return false;
    }

    return framework_poll_event(handle);
}
#endif

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/* Handle event by polling application - use data from header stored in data */
bool framework_poll_event(struct naf_handle_s *handle)
{
    unabto_buffer              w_buf;
    unabto_query_response      queryResponse;
    uint8_t                   *ptr;
    application_event_result   res;
    uint8_t*                   buf = nabtoCommunicationBuffer;
    uint8_t*                   end = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;
    ptr = buf;
    
    /* Write packet header and crypto payload header into buf. */
    ptr = reconstruct_header(ptr, end, &handle->header);
    if (!ptr) {
        NABTO_LOG_ERROR(("buffer too small to contain reconstructed header."));
        return AER_REQ_SYSTEM_ERROR;
    }

    /* Set up a write buffer to write into buf (after crypto payload header). */
    unabto_buffer_init(&w_buf, ptr, (int)(end - ptr));
    unabto_query_response_init(&queryResponse, &w_buf);

    res = application_poll(&handle->applicationRequest, &queryResponse);
    NABTO_LOG_TRACE(("APPREQ application_poll: result=%" PRItext, result_s(res)));

    if (res != AER_REQ_RESPONSE_READY) {
        return send_exception(handle->connection, &handle->header, res);
    } else {
        return send_and_encrypt_packet_con(handle->connection, buf, end, ptr, unabto_query_response_used(&queryResponse), ptr - 6);
    }
}
#endif


#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/* Reconstruct nabto header with crypto payload header. */
uint8_t* reconstruct_header(uint8_t* buf, uint8_t* end, const nabto_packet_header * hdr)
{
    uint8_t* ptr = buf;

    ptr = nabto_wr_header(buf, end, hdr);
    if (ptr == NULL) {
        return NULL;
    }

    add_flags(buf, NP_PACKET_HDR_FLAG_RESPONSE);

    // insert crypto header (len and code will be inserted later)
    WRITE_U8(ptr, NP_PAYLOAD_TYPE_CRYPTO);                    ptr += 1;
    WRITE_U8(ptr, NP_PAYLOAD_HDR_FLAG_NONE);                  ptr += 1;

    // Crypto len and code 2+2 bytes is patched by unabto_encrypt()
    ptr += 4;

    return ptr; /* return end of packet (so far) */
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
        if (&h->applicationRequest == req) {
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

/*****************************************************************************/
/********************  Local helper functions  ************** *****************/
/*****************************************************************************/

/******************************/
/* Logging of app/queue state */
/******************************/
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

/***********************************/
/* END: Logging of app/queue state */
/***********************************/

#endif
