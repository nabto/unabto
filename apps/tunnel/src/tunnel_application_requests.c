#include <unabto/unabto_app.h>
#include "tunnel_application_requests.h"
#include "stun/stuntest.h"
#include <modules/mt/unabto_thread.h>

typedef enum {
    STUN_IDLE = 0,
    STUN_TESTING,
    STUN_DONE
} stunState;

typedef struct {
    stunState state;
    uint32_t response;
    unabto_thread_t thread;
    application_request* request;
} stunRequest;

#define STUN_REQUEST_QUEUE_SIZE NABTO_APPREQ_QUEUE_SIZE

static stunRequest stunRequests[STUN_REQUEST_QUEUE_SIZE];

static stunRequest* findRequestWithState(stunState state) {
    int i;
    for (i = 0; i < STUN_REQUEST_QUEUE_SIZE; i++) {
        if (stunRequests[i].state == state) {
            return &stunRequests[i];
        }
    }
    return NULL;
}

static stunRequest* findRequest(application_request* request) {
    int i;
    for (i = 0; i < STUN_REQUEST_QUEUE_SIZE; i++) {
        if (stunRequests[i].request == request) {
            return &stunRequests[i];
        }
    }
    return NULL;
}

intptr_t doStun(void* data) 
{
    stunRequest* req = (stunRequest*)data;
    req->response = stuntest("194.192.21.148");
    req->state = STUN_DONE;
    return 0;
}

application_event_result application_event(application_request* request, buffer_read_t* readBuffer, buffer_write_t* writeBuffer)
{
    if (request->queryId == 1) {
        stunRequest* req = findRequestWithState(STUN_IDLE);
        if (req != NULL) {
            req->state = STUN_TESTING;
            unabto_thread_create(&req->thread, doStun, req);
            unabto_thread_detach(req->thread);
            req->request = request;
            return AER_REQ_ACCEPTED;
        } else {
            return AER_REQ_OUT_OF_RESOURCES;
        }
    }
    return AER_REQ_NO_QUERY_ID;
}

bool application_poll_query(application_request** applicationRequest) {
    stunRequest* req = findRequestWithState(STUN_DONE);
    if (req != NULL) {
        *applicationRequest = req->request;
        return true;
    }
    return false;
}

application_event_result application_poll(application_request* applicationRequest, buffer_read_t* readBuffer, buffer_write_t* writeBuffer) {
    stunRequest* req = findRequest(applicationRequest);
    if (req == NULL) {
        return AER_REQ_SYSTEM_ERROR;
    }

    req->state = STUN_IDLE;
    if (!unabto_query_write_uint32(writeBuffer, req->response)) {
        return AER_REQ_RSP_TOO_LARGE;
    }
    return AER_REQ_RESPONSE_READY;
}

void application_poll_drop(application_request* applicationRequest) {
    stunRequest* req = findRequest(applicationRequest);
    if (req != NULL) {
        unabto_thread_cancel(req->thread);
        req->state = STUN_IDLE;
    }
}
