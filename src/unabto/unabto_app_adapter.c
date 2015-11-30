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

#include <string.h>
#include <stdlib.h>

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/// The data included in a naf_handle
struct naf_handle_s {
    application_request applicationRequest;           ///< the request part seen by the application
    nabto_packet_header header;                       ///< the request packet header
    nabto_connect*      connection;                   ///< the connection
    uint32_t            spnsi;                        ///< the connection consistency id
    buffer_t            applicationRequestBuffer;     ///< pointer to application request
    buffer_read_t       readBuffer;                   ///< read access to r_buf
};

//struct queue_entry_s;
typedef struct queue_entry_s queue_entry;

typedef struct {
    /* inupt */
    const char* clientId;
    uint16_t reqId;
    /* output */
    struct naf_handle_s* found;
} client_req_query;

typedef struct {
    /* inupt */
    const application_request *req;
    /* output */
    struct naf_handle_s* found;
} request_query;

/* Local prototypes for ASYNC model */
static bool is_client_request_cb(struct naf_handle_s*, void *); //client_req_query *
static bool find_request_cb(struct naf_handle_s*, void *); //request_query *
static queue_entry* find_any_request_in_queue(void);
#endif

/************************************************/
/* Local prototypes (both SYNC and ASYNC model) */
/************************************************/

/**
 * Try to handle application event the first time.
 * Initialize request data in handle and call #application_event().
 * Returns AER_REQ_RESPONSE_READY if a response is ready for the given
 * request. In that case w_b will contain the response.
 * If this function fails all buffers are unmodified. */
static application_event_result framework_first_event(application_request*       req,
                                                      buffer_read_t*             r_b,
                                                      buffer_write_t*            w_b,
                                                      nabto_connect*             con,
                                                      const nabto_packet_header* hdr);

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/**
 * Try to handle application event the first time.
 * Initialize request data in handle and call #application_event().
 * Also keep queue state up to date.
 * Returns AER_REQ_RESPONSE_READY if a response is ready for the given
 * request. In that case w_b will contain the response.
 * Returns AER_REQ_ACCEPTED if no response is ready. In that case all
 * buffers are unmodified.
 * All other return values are error codes. In that case all buffers
 * are unmodified. */
static application_event_result framework_try_event(queue_entry                *entry,
                                                    buffer_read_t              *r_b,
                                                    buffer_write_t             *w_b,
                                                    nabto_connect*             con,
                                                    const nabto_packet_header* hdr);
#endif

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/**
 * Poll for a response for a request in the application.
 * Recreate request data from handle and call #application_poll().
 * Returns AER_REQ_RESPONSE_READY if a response is ready for the given
 * request. In that case buf will contain the response and olen the number
 * of bytes written to buf.
 * All other return values are error codes. In that case all buffers
 * are unmodified. */
static application_event_result framework_poll_event(struct naf_handle_s *handle,
                                                     uint8_t             *buf,
                                                     uint16_t             size,
                                                     uint16_t            *olen);
#endif

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/**
 * Update the state of a pending request after calling #application_event()
 * depending on the result from #application_event().
 */
static application_event_result framework_update_state(queue_entry *            entry,
                                                       application_event_result ret);
#endif

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/**
 * Build/reconstruct the first part of the response (nabto packet header
 * and crypto payload header).
 * @param  buf     the start of destination buffer
 * @param  hdr     the nabto packet header stored in the application queue
 * @return         pointer to the end of the written header. NULL on error.
 */
static uint8_t* reconstruct_header(uint8_t* buf, const nabto_packet_header * hdr);
#endif

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/**
 * Writes a exception packet into given buffer.
 * Returns true if the exception packet has been written to the buffer. In
 * that case buf will contain the response and olen the number of bytes
 * written to buf.
 * Returns false on error. In that case all buffers are unmodified. */
static bool framework_write_exception(application_event_result   code,
                                      const nabto_packet_header *hdr,
                                      uint8_t                     *buf,
                                      uint16_t                     size,
                                      uint16_t                    *olen);
#endif

/******************************************************************/
/* Logging of application request info (both SYNC and ASYNC model */
/******************************************************************/
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#define PRI_client_id           "%" PRIptr " ('%s'),%" PRIu16
#define CLIENT_ID_ARGS(c,s)     (c), (c), (s)
#define PRI_client_id2          "%" PRIptr " ('%s'), reqId/seq=%" PRIu16
#define CLIENT_ID_ARGS2(c,s)    (c), (c), (s)
#define PRI_client_id_q         "%" PRIptr " ('%s'), req=%" PRIu16 ", queryId=%" PRIu32
#define CLIENT_ID_Q_ARGS(c,s,q) (c), (c), (s), (q)
#endif //DOXYGEN_SHOULD_SKIP_THIS


/*****************************************************************************/
/*****  Implementation of a FIFO queue - made inline for speed and space  ****/
/*****************************************************************************/
#if NABTO_APPLICATION_EVENT_MODEL_ASYNC

/** The states of an application request ressource */
typedef enum {
    APPREQ_FREE,    /**< the entry is not in use (MUST have value 0)                     */
    APPREQ_WAITING, /**< the entry is holding a request awaiting being handed to the app */
    APPREQ_IN_APP   /**< the entry has been given to the app for treatment               */
} queue_state;

/** An entry in the FIFO queue */
struct queue_entry_s {
    struct naf_handle_s data;        /**< the data part (must be first - see queue_index) */
    queue_state  state;              /**< the state of the data in the queue */
};

#if UNABTO_PLATFORM_PIC18
#pragma udata big_mem
#endif

/** The FIFO queue */
static NABTO_THREAD_LOCAL_STORAGE queue_entry queue[NABTO_APPREQ_QUEUE_SIZE];

#if UNABTO_PLATFORM_PIC18
#pragma udata
#endif

#if NABTO_APPREQ_QUEUE_SIZE > 1
/** The first application ressource that may be being handled by the application */
static NABTO_THREAD_LOCAL_STORAGE queue_entry* queue_first_used = queue;
/** The first application ressource candidate to receive a new request from a client */
static NABTO_THREAD_LOCAL_STORAGE queue_entry* queue_next_free = queue;
#endif
/** Tell whether last operation was receiving a request or handling a request */
static NABTO_THREAD_LOCAL_STORAGE bool queueLastAdd = false;

/*****************************/
/* Private API of FIFO queue */
/*****************************/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#define queue_index(p) (((queue_entry*)(p)) - queue)
#define PRI_index PRIptrdiff
#if NABTO_APPREQ_QUEUE_SIZE > 1
#define queue_inc(p) \
        (p)++; \
        if ((p) >= queue + NABTO_APPREQ_QUEUE_SIZE) \
            (p) = queue;
#endif
#define CLIENT_ID_ARGH(h)    CLIENT_ID_ARGS((h).applicationRequest.clientId, (h).header.seq)
#define CLIENT_ID_Q_ARGH(h)  CLIENT_ID_Q_ARGS((h).applicationRequest.clientId, (h).header.seq, (h).applicationRequest.queryId)
#endif //DOXYGEN_SHOULD_SKIP_THIS

/****************************/
/* Public API of FIFO queue */
/****************************/
#if NABTO_APPREQ_QUEUE_SIZE > 1
/** Initialize queue */
#define queue_init()  \
        memset(queue, 0, sizeof(queue)); \
        queue_first_used = queue; \
        queue_next_free = queue; \
        queueLastAdd = false;
/** Returns true if queue is empty. */
#define queue_empty()     (!queueLastAdd && (queue_first_used == queue_next_free))
/** Returns true if queue is full. */
#define queue_full()      (queueLastAdd && (queue_first_used == queue_next_free))
/** Returns (struct naf_handle_s*) top element in queue. NULL if queue is empty. */
#define queue_top_elem()  (queue_empty() ? NULL : &queue_first_used->data)
/** Returns top of queue. If the queue is empty the result is undefined. */
#define queue_top()       (queue_first_used) /* UNABTO_ASSERT(!queue_empty()) */
/** Returns a new entry. Returns NULL if queue is full. */
#define queue_alloc()     (queue_full() ? NULL : queue_next_free)
/** Removes the top element from the queue. If the queue is empty the result is undefined. */
#define queue_pop()  \
        queue_inc(queue_first_used); \
        queueLastAdd = false; /* now q is empty if queue_first_used == queue_next_free */ \
        NABTO_LOG_TRACE(("APPREQ queue_pop: first_used inc'd to %" PRI_index, queue_index(queue_first_used)));
/** Advances last element in the queue. If the queue is full the result is undefined. */
#define queue_push()  \
        queue_inc(queue_next_free); \
        queueLastAdd = true; /* now q is full if queue_first_used == queue_next_free */ \
        NABTO_LOG_TRACE(("APPREQ queue_push: next_free inc'd to %" PRI_index, queue_index(queue_next_free)));
#else //NABTO_APPREQ_QUEUE_SIZE == 1
/** Initialize queue */
#define queue_init()  \
        memset(queue, 0, sizeof(queue)); \
        queueLastAdd = false;
/** Returns true if queue is empty. */
#define queue_empty()     (!queueLastAdd)
/** Returns true if queue is full. */
#define queue_full()      (queueLastAdd)
/** Returns (struct naf_handle_s*) top element in queue. NULL if queue is empty. */
#define queue_top_elem()  (queue_empty() ? NULL : &queue[0].data)
/** Returns top of queue. If the queue is empty the result is undefined. */
#define queue_top()       (queue) /* UNABTO_ASSERT(!queue_empty()) */
/** Returns a new entry. Returns NULL if queue is full. */
#define queue_alloc()     (queue_full() ? NULL : queue)
/** Removes the top element from the queue. If the queue is empty the result is undefined. */
#define queue_pop()       queueLastAdd = false; /* now q is empty */
/** Advances last element in the queue. If the queue is full the result is undefined. */
#define queue_push()      queueLastAdd = true; /* now q is full */
#endif //NABTO_APPREQ_QUEUE_SIZE == 1
/** Find the queue entry holding the given handle */
#define queue_find_entry(h) ((queue_entry*)(h))
/** Function prototype for callback function called from queue_enum.
 * Return false to stop enumeration.
 * Return true to continue enumeration. */
typedef bool (*queue_enum_proc)(struct naf_handle_s*, void *);
/** Enumerates all entries in queue (from top to bottom).
 * Returns false immediately if and only if proc returns false.
 * Returns true if every call to proc returned true. */
static bool queue_enum(queue_enum_proc proc, void *data);

#endif //NABTO_APPLICATION_EVENT_MODEL_ASYNC
/*****************************************************************************/
/**  END: Implementation of a FIFO queue - made inline for speed and space  **/
/*****************************************************************************/


/******************************/
/* Logging of app/queue state */
/******************************/
#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_TRACE)
static text result_s(application_event_result result);
#endif

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
#if NABTO_ENABLE_LOGGING == 1
#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_TRACE)
static void log_app_state(char *state);
static text state_s(queue_state state);
#if NABTO_APPREQ_QUEUE_SIZE > 1
#define LOG_APPREQ_ERROR(w,e) \
       NABTO_LOG_TRACE(("APPREQ " w ": returns '" e "' size=%i", NABTO_APPREQ_QUEUE_SIZE))
#define LOG_APPREQ_STATE(w,s,p) \
       NABTO_LOG_TRACE(("APPREQ " w ": returns '" s "'  rec=%" PRI_index, queue_index(p)))
#define LOG_APPREQ_BYTES(w,r,p,s) \
       NABTO_LOG_TRACE(("APPREQ " w ": returns " r " from rec=%" PRI_index " sending %" PRIu16 " bytes", queue_index(p), (s)))
#define LOG_APPREQ_WHERE(w,p) \
       NABTO_LOG_TRACE(("APPREQ " w ": rec=%" PRI_index, queue_index(p)))
#define LOG_APPREQ_LEAVE(w,r,h) \
       NABTO_LOG_TRACE(("APPREQ " w ": returns %" PRItext " using rec=%" PRI_index \
                        " with state %" PRItext, \
                        result_s(r), queue_index(h), state_s(queue_find_entry(h)->state)))
#define LOG_APPREQ_QUEUE() { \
       char state[NABTO_APPREQ_QUEUE_SIZE * 8]; /* size + 1 of max string returned by state_s */ \
       log_app_state(state); \
       NABTO_LOG_TRACE(("APPREQ: (%s)  next_free:%" PRI_index "  first_used:%" PRI_index "  last_add:%i %" PRItext "%" PRItext, \
                     state, \
                     queue_index(queue_next_free), \
                     queue_index(queue_first_used), \
                     (int)queueLastAdd, \
                     queue_full() ? " full" : "", \
                     queue_empty() ? " empty" : "")); \
    }
#define LOG_APPREQ_EWAIT(w,p) \
       NABTO_LOG_FATAL(("SW error: handle should be in 'WAIT' state. rec=%" PRI_index \
                        " is state=%" PRItext, \
                        queue_index(p), state_s((p)->state)))
#else //NABTO_APPREQ_QUEUE_SIZE == 1
#define LOG_APPREQ_ERROR(w,e) \
       NABTO_LOG_TRACE(("APPREQ " w ": returns '" e "'"))
#define LOG_APPREQ_STATE(w,s,p) \
       NABTO_LOG_TRACE(("APPREQ " w ": returns '" s "'"))
#define LOG_APPREQ_BYTES(w,r,p,s) \
       NABTO_LOG_TRACE(("APPREQ " w ": returns " r " sending %" PRIu16 " bytes", (s)))
#define LOG_APPREQ_WHERE(w,p) \
       NABTO_LOG_TRACE(("APPREQ " w))
#define LOG_APPREQ_LEAVE(w,r,h) \
       NABTO_LOG_TRACE(("APPREQ " w ": returns %" PRItext " with state %" PRItext, \
                        result_s(r), state_s(queue_find_entry(h)->state)))
#define LOG_APPREQ_QUEUE() { \
       char state[NABTO_APPREQ_QUEUE_SIZE * 8]; /* size + 1 of longest string returned by state_s */ \
       log_app_state(state); \
       NABTO_LOG_TRACE(("APPREQ: (%s)", state)); \
    }
#define LOG_APPREQ_EWAIT(w,p) \
       NABTO_LOG_FATAL(("SW error: handle should be in 'WAIT' state. state=%" PRItext, \
                        state_s((p)->state)))
#endif
#else // NABTO_LOG_ALL == 0
#define LOG_APPREQ_ERROR(w,e)
#define LOG_APPREQ_STATE(w,s,p)
#define LOG_APPREQ_BYTES(w,r,p,s)
#define LOG_APPREQ_WHERE(w,p)
#define LOG_APPREQ_LEAVE(w,r,h)
#define LOG_APPREQ_QUEUE() 
#define LOG_APPREQ_EWAIT(w,p)
#endif
#else // NABTO_ENABLE_LOGGING == 0
#define LOG_APPREQ_ERROR(w,e)
#define LOG_APPREQ_STATE(w,s,p)
#define LOG_APPREQ_BYTES(w,r,p,s)
#define LOG_APPREQ_WHERE(w,p)
#define LOG_APPREQ_LEAVE(w,r,h)
#define LOG_APPREQ_QUEUE() 
#define LOG_APPREQ_EWAIT(w,p)
#endif
#if NABTO_ENABLE_LOGGING == 1
#if NABTO_APPREQ_QUEUE_SIZE > 1
#define LOG_APPERR_0(p,f)     NABTO_LOG_ERROR((f " [rec=%" PRI_index "]: client=" PRI_client_id, queue_index(p), CLIENT_ID_ARGH((p)->data)))
#define LOG_APPERR_1(p,f,a)   NABTO_LOG_ERROR((f " [rec=%" PRI_index "]: client=" PRI_client_id, a, queue_index(p), CLIENT_ID_ARGH((p)->data)))
#define LOG_APPERR_2(p,f,a,b) NABTO_LOG_ERROR((f " [rec=%" PRI_index "]: client=" PRI_client_id, a, b, queue_index(p), CLIENT_ID_ARGH((p)->data)))
#else
#define LOG_APPERR_0(p,f)     NABTO_LOG_ERROR((f ": client=" PRI_client_id, CLIENT_ID_ARGH((p)->data)))
#define LOG_APPERR_1(p,f,a)   NABTO_LOG_ERROR((f ": client=" PRI_client_id, a, CLIENT_ID_ARGH((p)->data)))
#define LOG_APPERR_2(p,f,a,b) NABTO_LOG_ERROR((f ": client=" PRI_client_id, a, b, CLIENT_ID_ARGH((p)->data)))
#endif
#else // NABTO_ENABLE_LOGGING == 0
#define LOG_APPERR_0(p,f)     NABTO_LOG_ERROR((f))
#define LOG_APPERR_1(p,f,a)   NABTO_LOG_ERROR((f,a))
#define LOG_APPERR_2(p,f,a,b) NABTO_LOG_ERROR((f,a,b))
#endif
#endif //NABTO_APPLICATION_EVENT_MODEL_ASYNC

/* HACK: Enable trace logging in SYNC mode. */
#if NABTO_APPLICATION_EVENT_MODEL_ASYNC == 0
#if NABTO_ENABLE_LOGGING == 1
#if NABTO_LOG_ALL == 1
/* For some reason NABTO_LOG_TRACE doesn't work in SYNC mode. Use NABTO_LOG_INFO. */
#undef NABTO_LOG_TRACE
#define NABTO_LOG_TRACE(m) NABTO_LOG_INFO(m)
#endif
#endif
#endif //NABTO_APPLICATION_EVENT_MODEL_ASYNC == 0
/* END HACK: Enable trace logging in SYNC mode. */
/***********************************/
/* END: Logging of app/queue state */
/***********************************/



#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/******************************************************************************/
/**********  ASYNCHRONIOUS EVENT HANDLING IMPLEMENTATION   ********************/
/******************************************************************************/

/* Initialize event framework for ASYNC usage.
 * A FIFO request queue is initialized. */
void init_application_event_framework(void)
{
    NABTO_LOG_INFO(("Application event adapter using ASYNC model, queue length: %i", NABTO_APPREQ_QUEUE_SIZE));
    queue_init();
}

/* Ask for an event handle corresponding to the given client request (in
 * ASYNC mode). The event handle must be released again using the
 * framework_release_handle function.
 * The return value is NAF_QUERY_NEW if this is the first time the client
 * request is seen (i.e. a new event handle is created). In this case
 * the buffer pointed to by handle will be assigned the new event handle.
 * Note that this handle will never be 0 (to distinguish the case when
 * calling this function in the SYNC model).
 * The return value is NAF_QUERY_QUEUED if the client request is already
 * known (and is pending to be processed). In this case the buffer pointed
 * to by handle is left unctouched.
 * The return value is NAF_QUERY_OUT_OF_RESOURCES if no more memory is
 * available for a new client request. The buffer pointed to by handle is
 * left unchanged. */
naf_query framework_event_query(const char* clientId, uint16_t reqId, naf_handle* handle)
{
    client_req_query query;
    queue_entry* entry;

    NABTO_LOG_TRACE(("APPREQ framework_event_query: client=" PRI_client_id2, CLIENT_ID_ARGS2(clientId, reqId)));

    /* First look for the request in the queue of pending requests. */
    query.clientId = clientId;
    query.reqId = reqId;
    query.found = NULL;
    queue_enum(is_client_request_cb, &query);
    if (query.found) {
        //The client request was found in the queue
        LOG_APPREQ_STATE("framework_event_query", "QUEUED", query.found);
        LOG_APPREQ_QUEUE();
        return NAF_QUERY_QUEUED;
    }

    /* Now the request was not found.
     * Reserve the next free entry in the queue. */
    entry = queue_alloc();
    if (!entry) {
        //The queue is full
        LOG_APPREQ_ERROR("framework_event_query", "OUT_OF_RESOURCES");
        LOG_APPREQ_QUEUE();
        return NAF_QUERY_OUT_OF_RESOURCES;
    }

    UNABTO_ASSERT(entry->state == APPREQ_FREE);
    if (entry->state != APPREQ_FREE) {
        //Hmmm - that's strange!. The new entry should have been free.
        if (clientId == entry->data.applicationRequest.clientId && reqId == entry->data.header.seq) {
            // - and it seems to be ourself!!
            LOG_APPREQ_STATE("framework_event_query", "QUEUED?", entry);
            LOG_APPREQ_QUEUE();
            return NAF_QUERY_QUEUED;
        }
        // The new entry belongs to someone else. The queue must be full !?
        LOG_APPREQ_ERROR("framework_event_query", "OUT_OF_RESOURCES?");
        LOG_APPREQ_QUEUE();
        return NAF_QUERY_OUT_OF_RESOURCES;
    }

    // Now we have a new request

    // Be sure we "own" the entry - advance to next free entry in the queue
    entry->state = APPREQ_WAITING;
    queue_push();

    // *handle cannot be initialized yet, as the received packet has not
    // yet been decrypted.
    // See framework_event() for initialization of handle.
    *handle = &entry->data;

    LOG_APPREQ_STATE("framework_event_query", "NEW", entry);
    LOG_APPREQ_QUEUE();
    return NAF_QUERY_NEW;
}

/* Release the event handle returned by framework_event_query.
 * In the ASYNC model we must remove the event handle from the FIFO request
 * queue. */
void framework_release_handle(naf_handle handle)
{
    queue_entry* entry;

    if (!handle) {
        return;
    }
    if (queue_empty()) {
        /* Why do the application try to release a handle on an empty queue? */
        NABTO_LOG_FATAL(("SW error: Calling framework_release_handle on an empty queue"));
        return;
    }

    /* Find the entry containing the handle */
    entry = queue_find_entry(handle);
    /* The given handle must belong to the queue */
    UNABTO_ASSERT(entry);

    entry->state = APPREQ_FREE;
    LOG_APPREQ_WHERE("framework_release_handle", entry);

    /* Remove top entry from FIFO queue - and remove all consecutive
     * entries that have expired/finished in the mean time. */
    while (!queue_empty()) {
        if (queue_top()->state != APPREQ_FREE)
            break;
        queue_pop();
    }

    LOG_APPREQ_QUEUE();
}

/* Handle application event in ASYNC model.
 * Initialize the handle and call applicaion_event. */
application_event_result framework_event(naf_handle           handle,
                                         uint8_t*             iobuf,
                                         uint16_t             size,
                                         uint16_t             ilen,
                                         uint16_t*            olen,
                                         nabto_connect*       con,
                                         nabto_packet_header* hdr)
{
    queue_entry*             entry;
    buffer_t                 w_buf;
    buffer_write_t           w_b;
    application_event_result ret;

    /* Find the entry containing the handle */
    entry = queue_find_entry(handle);
    /* The given handle must belong to the queue */
    UNABTO_ASSERT(entry);

    /* Handle must have been returned by framework_event_query(). */
    if (entry->state != APPREQ_WAITING) {
        LOG_APPREQ_QUEUE();
        LOG_APPREQ_EWAIT("framework_event", entry);
    }

    /* Initialize entry in queue. */
    entry->data.connection = con;
    if (con && hdr) {
        memcpy(&entry->data.header, (const void*)hdr, sizeof(entry->data.header));
        entry->data.spnsi  = con->spnsi;
    } else {
        memset(&entry->data.header, 0, sizeof(entry->data.header));
        entry->data.spnsi  = 0;
    }

    /* Set up a write buffer to write into iobuf. */
    buffer_init(&w_buf, iobuf, size);
    buffer_write_init(&w_b, &w_buf);

    /* Set up a read buffer that reads from local space or iobuf. */
    buffer_init(&entry->data.applicationRequestBuffer, iobuf, ilen);
    buffer_read_init(&entry->data.readBuffer, &entry->data.applicationRequestBuffer, ilen);

    ret = framework_try_event(entry, &entry->data.readBuffer, &w_b, con, hdr);

    /* Handle errors */
    switch (ret) {
        case AER_REQ_RESPONSE_READY:
            //ok - do nothing
            break;

        case AER_REQ_ACCEPTED:
            LOG_APPREQ_LEAVE("framework_event", ret, handle);
            LOG_APPREQ_QUEUE();
            return ret;

        default:
            // pass the result to caller as an exception
            buffer_write_reset(&w_b);
            buffer_write_uint32(&w_b, ret);
            hdr->flags |= NP_PACKET_HDR_FLAG_EXCEPTION; // caller copies flags to send buffer!
            NABTO_LOG_TRACE(("Inserting EXCEPTION %i in buffer: %" PRItext, ret, result_s(ret)));
            break;
    }

    *olen = (uint16_t) buffer_write_used(&w_b);

    LOG_APPREQ_LEAVE("framework_event", ret, handle);
    LOG_APPREQ_QUEUE();
    return ret;
}

/* Poll a pending event.
 * Returns true if a response is ready (from any event in the queue).
 * If this function fails (returns false) all buffers are unmodified. */
bool framework_event_poll(uint8_t* buf, uint16_t size, uint16_t* olen, nabto_connect** con)
{
    queue_entry             *entry;
    application_event_result ret;
    application_request     *req;
    struct naf_handle_s     *handle;
    request_query            find;

    if (queue_empty()) {
        // no requests queued in this framework,
        // hence no requests pending in application
        return false;
    }

    //Loop through the queue and find the running (IN_APP) request,
    //if any - and find the first waiting request. Then do the below.
    //OLD: entry = queue_top();
    entry = find_any_request_in_queue();
    if (!entry) {
        return false;
    }

    switch (entry->state) {
        case APPREQ_FREE:
            // first request has been released,
            // hence no requests pending in application
            return false;
    
        case APPREQ_IN_APP:
            // A request is pending in the application, continue 
            break;

      default:
        // Unknown state - ???
        UNABTO_ASSERT(0);
        return false;
    }

    //The application is waiting for this framework to call it.

    //Ask for the request in the application
    req = NULL;
    if (!application_poll_query(&req)) {
        // The application isn't ready with a response yet, poll again later
        return false;
    }
    //If application_poll_query returns true - it must return a request handle
    if (!req) {
        LOG_APPERR_0(entry, "Response dropped because the application returned a NULL request !?");
        goto drop_poll;
    }

    //Find the handle holding the given request
    find.req = (const application_request*) req;
    find.found = NULL;
    queue_enum(find_request_cb, &find);
    handle = find.found;
    if (!handle) {
        LOG_APPERR_0(entry, "Response dropped because the application returned a request that isn't known by the framework");
        goto drop_poll;
    }
    UNABTO_ASSERT(req == &handle->applicationRequest);

    entry = queue_find_entry(handle);
    //The request must be part of a handle in the queue
    if (!entry) {
        LOG_APPERR_0(entry, "Response dropped because the application returned a request that isn't known by the framework queue");
        goto drop_poll;
    }
    UNABTO_ASSERT(handle == &entry->data);

    if (handle->connection->spnsi != handle->spnsi) {
        // the connection has changed, the request/response is dropped
        NABTO_LOG_TRACE(("Response dropped because the connection (" PRI_client_id ") is gone", CLIENT_ID_ARGH(*handle)));
        goto drop_poll;
    }
 
    NABTO_LOG_TRACE(("APPREQ: polling=%" PRI_index, queue_index(entry)));

    ret = framework_poll_event(&entry->data, buf, size, olen);

  if (ret != AER_REQ_RESPONSE_READY) {
      // pass the result to caller to become an exception
      if (!framework_write_exception(ret, &entry->data.header, buf, size, olen)) {
            LOG_APPERR_1(entry, "Response dropped because an exception packet with error code %i couldn't be generated", (int)ret);
            goto drop_poll;
      }
  }

    *con = entry->data.connection;
    framework_release_handle(handle);
    LOG_APPREQ_BYTES("framework_event_poll", "true", entry, *olen);
    LOG_APPREQ_QUEUE();
    return true;

drop_poll:
    application_poll_drop(req);
    framework_release_handle(&entry->data);
    LOG_APPREQ_BYTES("framework_event_poll", "false", entry, 0);
    LOG_APPREQ_QUEUE();
    return false;
}

#else //NABTO_APPLICATION_EVENT_MODEL_ASYNC == 0
/******************************************************************************/
/************  STUBS FOR SYNCHRONIOUS EVENT HANDLING **************************/
/******************************************************************************/

/* Initialize event framework for SYNC usage. */
void init_application_event_framework()
{
    NABTO_LOG_INFO(("Application event framework using SYNC model"));
}

/* Ask for an event handle corresponding to the given client request.
 * Since we are using the SYMC model a new event handle is always ready -
 * but be aware the handle is always 0, since it is not used by the SYNC
 * framework implementation. */
naf_query framework_event_query(const char* clientId, uint16_t requestId, naf_handle* handle)
{
    NABTO_NOT_USED(clientId);
    NABTO_NOT_USED(requestId);
    *handle = 0;
    return NAF_QUERY_NEW;
}

/* Release the event handle returned by framework_event_query.
 * In the SYNC model nothing needs to be done, since we dont use event
 * handles. */
void framework_release_handle(naf_handle handle)
{
    NABTO_NOT_USED(handle);
}

/* Handle application event in SYNC model.
 * Call application_event. */
application_event_result framework_event(naf_handle             handle,
                                         uint8_t*               iobuf,
                                         uint16_t               size,
                                         uint16_t               ilen,
                                         uint16_t*              olen,
                                         nabto_connect*       con,
                                         nabto_packet_header* hdr)
{
    application_request      req;
    buffer_t                 w_buf;
    buffer_write_t           w_b;
    buffer_t                 r_buf;
    buffer_read_t            r_b;
    application_event_result ret;

    /* framework_event_query always returns a zero handle in SYNC mode */
    UNABTO_ASSERT(handle == 0);

    // When trying to respond immediately, the position to write the response is exactly
    // the same as the position of the request (we use the same buffer, i.e. iobuf).
    buffer_init(&w_buf, iobuf, size);
    buffer_write_init(&w_b, &w_buf);

    buffer_init(&r_buf, iobuf, ilen);
    buffer_read_init(&r_b, &r_buf, ilen);

    ret = framework_first_event(&req, &r_b, &w_b, con, hdr);

    if (ret != AER_REQ_RESPONSE_READY) {
        // pass the result to caller as an exception packet
        buffer_write_reset(&w_b);
        buffer_write_uint32(&w_b, ret);
        hdr->flags |= NP_PACKET_HDR_FLAG_EXCEPTION; // caller copies flags to send buffer!
        NABTO_LOG_TRACE(("Inserting EXCEPTION %i in buffer: %" PRItext, ret, result_s(ret)));
    }
    *olen = buffer_write_used(&w_b);

    NABTO_LOG_TRACE(("APPREQ framework_event: returns %" PRItext, result_s(ret)));
    return ret;
}

/* Poll a pending event.
 * Since we are using the SYNC model, nothing can be polled. Hence this
 * function always fails (returns false) and all buffers are unmodified. */
bool framework_event_poll(uint8_t* buf, uint16_t buflen, uint16_t* olen, struct nabto_connect_s** con)
{
    NABTO_NOT_USED(buf);
    NABTO_NOT_USED(buflen);
    NABTO_NOT_USED(olen);
    NABTO_NOT_USED(con);
    return false;
}

#endif // NABTO_APPLICATION_EVENT_MODEL_ASYNC == 0

/* Handle event the first time - application request is initialized.
 * Common function for SYNC and ASYNC model. */
application_event_result framework_first_event(application_request       *req,
                                                 buffer_read_t               *r_b,
                                                 buffer_write_t              *w_b,
                                                 nabto_connect*             con,
                                                 const nabto_packet_header* hdr)
{
    application_event_result res;
    uint32_t                 query_id;

    if (buffer_read_available(r_b) > NABTO_REQUEST_MAX_SIZE) {
        return AER_REQ_TOO_LARGE;
    }
    if (!buffer_read_uint32(r_b, &query_id)) {
        return AER_REQ_NO_QUERY_ID;
    }

    /* Initialize application request info */
    req->isLocal = con->isLocal;
    req->isLegacy = false;
    if (con && hdr) {
        req->clientId = (const char *) con->clientId;
    } else {
        req->clientId = 0;
    }
    req->queryId = query_id;

    NABTO_LOG_TRACE(("APPREQ application_event: client=" PRI_client_id_q, CLIENT_ID_Q_ARGS(req->clientId, hdr ? hdr->seq : 0, req->queryId)));
    res = application_event(req, r_b, w_b);
    NABTO_LOG_TRACE(("APPREQ application_event: result=%i %" PRItext, res, result_s(res)));

    return res;
}

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/* Handle event the first time - application request is initialized. */
application_event_result framework_try_event(queue_entry                *entry,
                                             buffer_read_t              *r_b,
                                             buffer_write_t             *w_b,
                                             nabto_connect*             con,
                                             const nabto_packet_header* hdr)
{
    application_event_result res;

    /* Call application (the first time) */
    res =  framework_first_event(&entry->data.applicationRequest, r_b, w_b, con, hdr);

    /* Update queue entry state according to result */
    res = framework_update_state(entry, res);

    return res;
}
#endif

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/* Handle event by polling application - use data from header stored in data */
application_event_result framework_poll_event(struct naf_handle_s *handle,
                                              uint8_t             *buf,
                                              uint16_t             size,
                                              uint16_t            *olen)
{
    buffer_t                   w_buf;
    buffer_write_t             w_b;
    uint16_t                   expected_len;
    uint8_t                   *ptr;
    uint32_t                   query_id;
    application_event_result res;
    uint16_t                   dlen;

    expected_len = handle->header.hlen + OFS_DATA;
    if (size <= expected_len) {
        NABTO_LOG_ERROR(("buffer too small to contain reconstructed header. size=%" PRIu16 " needed=%" PRIu16, size, expected_len));
        return AER_REQ_SYSTEM_ERROR;
    }

    /* Write packet header and crypto payload header into buf. */
    ptr = reconstruct_header(buf, &handle->header);
    if (!ptr) {
        //log message already given (header length mismatch)
        return AER_REQ_SYSTEM_ERROR;
    }
    {
        ptrdiff_t diff = ptr - buf;
        if (expected_len != (uint16_t)(diff)) {
            NABTO_LOG_ERROR(("header (with crypto) length mismatch: %" PRIu16 " != %" PRIptrdiff " !!!!!!!!!!!!!!!!!!!", expected_len, diff));
            return AER_REQ_SYSTEM_ERROR;
        }
    }

    /* Set up a write buffer to write into buf (after crypto payload header). */
    buffer_init(&w_buf, buf + expected_len, (int)(size - expected_len));
    buffer_write_init(&w_b, &w_buf);

    /* Start reading input buffer from start again. */
    buffer_read_reset(&handle->readBuffer);
    /* Skip query id. */
    if (!buffer_read_uint32(&handle->readBuffer, &query_id)) {
        return AER_REQ_NO_QUERY_ID;
    }

  /* Call application */
    NABTO_LOG_TRACE(("APPREQ application_poll: client=" PRI_client_id_q, CLIENT_ID_Q_ARGH(*handle)));

    res = application_poll(&handle->applicationRequest, NULL, &w_b);
    NABTO_LOG_TRACE(("APPREQ application_poll: result=%i %" PRItext, res, result_s(res)));

    if (res != AER_REQ_RESPONSE_READY)
    {
        return res;
    }

    dlen = unabto_crypto_max_data(&handle->connection->cryptoctx, (uint16_t)(size - expected_len));

    if (!unabto_encrypt(&handle->connection->cryptoctx, ptr, buffer_write_used(&w_b), ptr, dlen, &dlen)) {
        NABTO_LOG_TRACE((PRInsi" Encryption failure", MAKE_NSI_PRINTABLE(0, handle->header.nsi_sp, 0)));
        return AER_REQ_SYSTEM_ERROR;
    }

    /* Update packet length and flag fields */
    dlen += expected_len;
    insert_length(buf, dlen);
    if (!unabto_insert_integrity(&handle->connection->cryptoctx, buf, dlen)) {
        NABTO_LOG_TRACE((PRInsi" Signing failure", MAKE_NSI_PRINTABLE(0, handle->header.nsi_sp, 0)));
        return AER_REQ_SYSTEM_ERROR;
    }

    *olen = dlen;
    return AER_REQ_RESPONSE_READY;
}
#endif

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/* Update the state of the queue entry after calling application. */
application_event_result framework_update_state(queue_entry* entry, application_event_result ret)
{
    if (entry->state != APPREQ_WAITING) {
        LOG_APPREQ_QUEUE();
        NABTO_LOG_FATAL(("SW error: handle should be in 'WAIT' state"));
    }
    switch (ret) {
        case AER_REQ_RESPONSE_READY:
              entry->state = APPREQ_FREE;
              break;
        case AER_REQ_ACCEPTED:
              entry->state = APPREQ_IN_APP;
              break;
        default:
              //error
              entry->state = APPREQ_FREE;
              break;
    }
    return ret;
}
#endif

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/* Reconstruct nabto header with crypto payload header. */
uint8_t* reconstruct_header(uint8_t* buf, const nabto_packet_header * hdr)
{
    uint8_t* ptr;

    // insert copy of request header (Length will be inserted later)

    ptr = buf;

    /* Write fixed part of packet header */
    WRITE_U32(ptr, hdr->nsi_cp);      ptr += 4;
    WRITE_U32(ptr, hdr->nsi_sp);      ptr += 4;
    WRITE_U8(ptr,  hdr->type);        ptr += 1;
    WRITE_U8(ptr,  hdr->version);     ptr += 1;
    WRITE_U8(ptr,  hdr->rsvd);        ptr += 1;
    WRITE_U8(ptr,  hdr->flags | NP_PACKET_HDR_FLAG_RESPONSE); ptr += 1;
    WRITE_U16(ptr, hdr->seq);         ptr += 2;
    /*WRITE_U16(ptr, len);*/          ptr += 2; /* Length to be packed later */

    /* Write NSI.co to packet header (optional) */
    if (hdr->flags & NP_PACKET_HDR_FLAG_NSI_CO) {
        memcpy(ptr, (const void*) hdr->nsi_co, 8);  ptr += 8;
    }

    /* Write tag to packet header (optional) */
    if (hdr->flags & NP_PACKET_HDR_FLAG_TAG) {
        WRITE_U16(ptr, hdr->tag);     ptr += 2;
    }

    {
        ptrdiff_t diff = ptr - buf;
        if (hdr->hlen != (uint16_t)(diff)) {
            NABTO_LOG_ERROR(("header length mismatch: %" PRIu16 " != %" PRIptrdiff " !!!!!!!!!!!!!!!!!!!", hdr->hlen, diff));
            return NULL;
        }
    }

    // insert crypto header (len and code will be inserted later)
    WRITE_U8(ptr, NP_PAYLOAD_TYPE_CRYPTO);                    ptr += 1;
    WRITE_U8(ptr, NP_PAYLOAD_HDR_FLAG_NONE);                /*ptr += 1;
    WRITE_U16(ptr, len);                                      ptr += 2;
    WRITE_U16(ptr, code);                                     ptr += 2;*/
    /* len and code is patched by unabto_encrypt() */         ptr += 5;

    return ptr; /* return end of packet (so far) */
}

/* Write error code to exception packet. */
bool framework_write_exception(application_event_result   code,
                               const nabto_packet_header *hdr,
                               uint8_t                     *buf,
                               uint16_t                     size,
                               uint16_t                    *olen)
{
    buffer_t       w_buf;
    buffer_write_t w_b;
    uint16_t       expected_len;
    uint8_t       *ptr;

    expected_len = hdr->hlen + OFS_DATA;
    if (size <= expected_len) {
        return false;
    }

    /* Write packet header and crypto payload header into buf. */
    ptr = reconstruct_header(buf, hdr);
    if (!ptr) {
        return false;
    }
    if (expected_len != (uint16_t)(ptr - buf)) {
        return false;
    }

    /* Set up a write buffer to write into buf (after crypto payload header). */
    buffer_init(&w_buf, buf + expected_len, (int)(size - expected_len));
    buffer_write_init(&w_b, &w_buf);

    if (!buffer_write_uint32(&w_b, code)) {
        return false;
    }

    add_flags(buf, NP_PACKET_HDR_FLAG_EXCEPTION);
    NABTO_LOG_TRACE(("Inserting EXCEPTION %i in buffer: %" PRItext, code, result_s(code)));

    *olen = (uint16_t) (expected_len + buffer_write_used(&w_b));
    return true;
}

#endif

/*****************************************************************************/
/********************  Local helper functions  ************** *****************/
/*****************************************************************************/
#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/** Callback to queue_enum that looks for a certain client request.
 * data must be of type struct client_req_query_s*.
 * Returns false when found (to stop enumeration). */
bool is_client_request_cb(struct naf_handle_s* entry, void * data)
{
    register client_req_query* query = (client_req_query*) data;

    if (query->clientId == entry->applicationRequest.clientId && query->reqId == entry->header.seq) {
        query->found = entry;
        return false; //found it - stop enumeration
    }
    return true; //not found - continue enumeration
}
#endif

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/** Callback to queue_enum that looks for a certain application request.
 * data must be of type struct request_query_s*.
 * Returns false when found (to stop enumeration). */
bool find_request_cb(struct naf_handle_s* entry, void * data)
{
    register request_query* query = (request_query*) data;

    if (query->req == &entry->applicationRequest) {
        query->found = entry;
        return false; //found it - stop enumeration
    }
    return true; //not found - continue enumeration
}
#endif

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/** Search for the first running or first pending requrest in the requesst
 * queue. Returns the first running requrest if found. Otherwise the first
 * pending request is returned. If neither is found NULL is returned.
 */
queue_entry* find_any_request_in_queue(void)
{
#if NABTO_APPREQ_QUEUE_SIZE > 1
    queue_entry* first_pending;
    queue_entry* first_running;
    queue_entry* entry;

    //At least one record must be present in the queue (for loop to be
    //finite).
    //queue may be full. queue_first_used == queue_next_free means full.
    UNABTO_ASSERT(!queue_empty());

    first_running = NULL;
    first_pending = NULL;
    entry = queue_first_used;
    do {
        if (!first_pending && entry->state == APPREQ_WAITING) {
            first_pending = entry;
            if (first_running)
                break;
        }
        if (!first_running && entry->state == APPREQ_IN_APP) {
            first_running = entry;
            if (first_pending)
                break;
        }
        queue_inc(entry);
    } while (entry != queue_next_free);

    if (first_running)
        return first_running;
    if (first_pending)
        return first_pending;
    return NULL;
#else //NABTO_APPREQ_QUEUE_SIZE == 1
    UNABTO_ASSERT(!queue_empty());
    return queue_top();
#endif
}
#endif

/*****************************************************************************/
/****************  More implementation of a FIFO queue  **********************/
/*****************************************************************************/
#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
/** Enumerates all entries in queue in order (from first to last item).
 * Returns false immediately iff proc returns false.
 * Returns true if every call to proc returned true. */
bool queue_enum(queue_enum_proc proc, void *data)
{
#if NABTO_APPREQ_QUEUE_SIZE > 1
    queue_entry* entry;
    if (queueLastAdd) {
        //At least one record in queue.
        //queue may be full. queue_first_used == queue_next_free means full.
        UNABTO_ASSERT(!queue_empty());
        UNABTO_ASSERT(queue_first_used->state != APPREQ_FREE);
        entry = queue_first_used;
        do {
            // If queue->state == APPREQ_FREE, we have an empty (already answered) slot
            // in the queue. Don't use (requests are inserted in strict order to start
            // treatment in the same order).
            if (entry->state != APPREQ_FREE) {
                  // Let caller determine whether to proceed.
                  if (!(*proc)(&entry->data, data)) {
                      return false;
                  }
            }
            queue_inc(entry);
        } while (entry != queue_next_free);
    } else {
        //queue may be empty. queue_first_used == queue_next_free means empty.
        UNABTO_ASSERT(!queue_full());
        entry = queue_first_used;
        while (entry != queue_next_free) {
            // If queue->state == APPREQ_FREE, we have an empty (already answered) slot
            // in the queue. Don't use (requests are inserted in strict order to start
            // treatment in the same order).
            if (entry->state != APPREQ_FREE) {
                  // Let caller determine whether to proceed.
                  if (!(*proc)(&entry->data, data)) {
                      return false;
                  }
            }
            queue_inc(entry);
        }
    }
#else //NABTO_APPREQ_QUEUE_SIZE == 1
    if (!queue_empty()) {
         // Let caller determine whether to proceed.
         if (!(*proc)(&queue[0].data, data)) {
             return false;
         }
    }
#endif
    return true;
}
#endif
/*****************************************************************************/
/***********  END: More implementation of a FIFO queue  **********************/
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

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_TRACE)
void log_app_state(char *state)
{
    int i;
    int len = 0;
    for (i = 0; i < NABTO_APPREQ_QUEUE_SIZE; i++) {
        if (i > 0)
            state[len++] = ',';
        len += sprintf(state + len, "%" PRItext, state_s(queue[i].state));
    }
    state[len] = 0;
}
text state_s(queue_state state)
{
    switch (state) {
        case APPREQ_FREE:    return "FREE";
        case APPREQ_WAITING: return "WAIT";
        case APPREQ_IN_APP:  return "IN_APP";
    }
    return "??";
}
#endif
#endif //NABTO_APPLICATION_EVENT_MODEL_ASYNC
/***********************************/
/* END: Logging of app/queue state */
/***********************************/

#endif
