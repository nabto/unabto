#include "async_application_event.h"
#include <unabto/unabto_next_event.h>

#include "unabto/unabto_common_main.h"
#include "unabto/unabto_logging.h"
#include "unabto/unabto_app.h"

/**
 * This is a test to show the unabto request/response async model. The
 * test consists of a queue with timeouts. when a request is accepted
 * it's added to the queue. It's timed out when the time is passed.
 */

struct queued_request {
    application_request* request;
    nabto_stamp_t expire;
    nabto_stamp_t created;
};

#define QUEUE_SIZE 10
struct queued_request queue[QUEUE_SIZE];

void update_next_async_event_timeout(nabto_stamp_t* ne)
{
    int i;
    for (i = 0; i < QUEUE_SIZE; i++) {
        struct queued_request* req = &queue[i];
        if (req->request != NULL) {
            nabto_update_min_stamp(ne, &req->expire);
        }
    }
}


void init_request_queue()
{
    memset(queue, 0, QUEUE_SIZE * sizeof(struct queued_request));
}

void clear_queue_entry(struct queued_request* entry)
{
    memset(entry, 0, sizeof(struct queued_request));
}

void init_queue_entry(struct queued_request* entry, application_request* request, uint32_t ms)
{
    entry->request = request;
    nabtoSetFutureStamp(&(entry->expire), ms);
    entry->created = nabtoGetStamp();
    return;
}


struct queued_request* find_empty_queue_slot()
{
    int i;
    for (i = 0; i < QUEUE_SIZE; i++) {
        if (queue[i].request == NULL) {
            return &queue[i];
        }
    }
    return NULL;
}

struct queued_request* find_queue_entry(application_request* request)
{
    int i;
    for (i = 0; i < QUEUE_SIZE; i++) {
        if (queue[i].request == request) {
            return &queue[i];
        }
    }
    return NULL;
}

application_event_result application_event(application_request* request, unabto_query_request* r_b, unabto_query_response* w_b)
{
    switch (request->queryId) {
        case 1:
        {
            struct queued_request* entry = find_empty_queue_slot();
            if (entry == NULL) {
                return AER_REQ_OUT_OF_RESOURCES;
            }
            
            uint32_t expire;
            // read 4 bytes from the input buffer
            if (!unabto_query_read_uint32(r_b, &expire)) {
                return AER_REQ_TOO_SMALL;
            }

            init_queue_entry(entry, request, expire);
            return AER_REQ_ACCEPTED;
        }
    }
    return AER_REQ_INV_QUERY_ID;    
}

/**
 * Query whether a response to a queued request is ready.
 * @param appreq  to return the request being ready
 * @return        true if a response is ready
 *
 * This function is called from the framework and it's out
 * responsibility to tell the framework which application requests has
 * an answer ready.
 */
bool application_poll_query(application_request** applicationRequest)
{
    int i;
    for (i = 0; i < QUEUE_SIZE; i++) {
        if (queue[i].request != NULL) {
            if (nabtoIsStampPassed(&(queue[i].expire))) {
                *applicationRequest = queue[i].request;
                return true;
            }
        }
    }
    return false;
}

/**
 * Retrieve the response from a queued request.
 * @param appreq  the application request being responded to (from #application_poll_query)
 * @param w_b     buffer, to retrieve the response message
 * @return        the result, see #application_event_result
 *
 * Must be called only after #application_poll_query returns true and then with appreq retrieved there.
 *
 * The application must release/delete its internal ressouce holding the requested request.
 */
application_event_result application_poll(application_request* applicationRequest, unabto_query_response* writeBuffer)
{
    struct queued_request* entry = find_queue_entry(applicationRequest);

    if (entry == NULL) {
        return AER_REQ_SYSTEM_ERROR;
    }

    nabto_stamp_t now = nabtoGetStamp();
    nabto_stamp_diff_t diff = nabtoStampDiff(&now, &(entry->created));
    int32_t duration = nabtoStampDiff2ms(diff);
    if (!unabto_query_write_int32(writeBuffer, duration)) {
        return AER_REQ_RSP_TOO_LARGE;
    } else {
        return AER_REQ_RESPONSE_READY;
    }
}


/**
 * Drop the queued request - the framework has discarded it.
 */
void application_poll_drop(application_request* applicationRequest)
{
    struct queued_request* entry = find_queue_entry(applicationRequest);
    if (entry != NULL) {
        clear_queue_entry(entry);
    }
}

