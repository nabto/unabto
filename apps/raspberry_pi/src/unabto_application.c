/**
 *  uNabto application logic implementation
 */

#include "unabto/unabto_app.h"
#include <stdio.h>

// Function prototypes
uint8_t setLight(uint8_t id, uint8_t onOff);
uint8_t readLight(uint8_t id);

// The virtual light bulb variable
static uint8_t theLight = 0;

/***************** The uNabto application logic *****************
 * This is where the user implements his/her own functionality
 * to the device. When a Nabto message is received, this function
 * gets called with the message's request id and parameters.
 * Afterwards a user defined message can be sent back to the
 * requesting browser.
 ****************************************************************/
application_event_result demo_application(application_request* request, unabto_query_request* read_buffer, unabto_query_response* write_buffer) {
    switch(request->queryId) {
        case 1: {
            //  <query name="light_write.json" description="Turn light on and off" id="1">
            //    <request>
            //      <parameter name="light_id" type="uint8"/>
            //      <parameter name="light_on" type="uint8"/>
            //    </request>
            //    <response>
            //      <parameter name="light_state" type="uint8"/>
            //    </response>
            //  </query>

            uint8_t light_id;
            uint8_t light_on;
            uint8_t light_state;

            // Read parameters in request
            if (!unabto_query_read_uint8(read_buffer, &light_id)) {
                NABTO_LOG_ERROR(("Can't read light_id, the buffer is too small"));
                return AER_REQ_TOO_SMALL;
            }
            if (!unabto_query_read_uint8(read_buffer, &light_on)) {
                NABTO_LOG_ERROR(("Can't read light_state, the buffer is too small"));
                return AER_REQ_TOO_SMALL;
            }

            // Set light according to request
            light_state = setLight(light_id, light_on);

            // Write back led state
            if (!unabto_query_write_uint8(write_buffer, light_state)) {
                return AER_REQ_RSP_TOO_LARGE;
            }

            return AER_REQ_RESPONSE_READY;
        }
        case 2: {
            //  <query name="light_read.json" description="Read light status" id="2">
            //    <request>
            //      <parameter name="light_id" type="uint8"/>
            //    </request>
            //    <response>
            //      <parameter name="light_state" type="uint8"/>
            //    </response>
            //  </query>

            uint8_t light_id;
            uint8_t light_state;

            // Read parameters in request
            if (!unabto_query_read_uint8(read_buffer, &light_id)) return AER_REQ_TOO_SMALL;

            // Read light state
            light_state = readLight(light_id);

            // Write back led state
            if (!unabto_query_write_uint8(write_buffer, light_state)) return AER_REQ_RSP_TOO_LARGE;

            return AER_REQ_RESPONSE_READY;
        }
    }
    return AER_REQ_INV_QUERY_ID;
}

// Set virtual light and return state,
// only using ID #1 in this simple example
uint8_t setLight(uint8_t id, uint8_t onOff) {
    theLight = onOff;
    NABTO_LOG_INFO((theLight?("Nabto: Light turned ON!"):("Nabto: Light turned OFF!")));

#ifdef __arm__
    // Toggle ACT LED on Raspberry Pi
    if (theLight) {
        system("echo 1 | sudo tee /sys/class/leds/led0/brightness");
    }
    else {
        system("echo 0 | sudo tee /sys/class/leds/led0/brightness");
    }
#endif

    return theLight;
}

// Return virtual light state,
// only using ID #1 in this simple example
uint8_t readLight(uint8_t id) {
    return theLight;
}

/** Asynchronous event model - queue the request for later response */
#if NABTO_APPLICATION_EVENT_MODEL_ASYNC

// Holds one saved request
static application_request* saved_app_req = 0;

application_event_result application_event(application_request* request, uanbto_query_request* r_b, unabto_query_response* w_b)
{
    if (request->queryId == 1 || request->queryId == 2 || request->queryId == 3) {
        NABTO_LOG_INFO(("Application event: Respond immediately"));
        return demo_application(request, r_b, w_b);
    }
    if (saved_app_req) {
        NABTO_LOG_INFO(("Application event: No resources"));
        return AER_REQ_OUT_OF_RESOURCES;
    } else {
        NABTO_LOG_INFO(("Application event: Accept request"));
        saved_app_req = request;
        return AER_REQ_ACCEPTED;
    }
}

// Query whether a response to a queued request is ready
bool application_poll_query(application_request** appreq)
{
    if (saved_app_req) {
        /**
        * Fake a delay for demonstration purpose.
        * This could be replaced with a variable that is first
        * set when some proccessing is done and a response
        * to the client should be send.
        */
        static int fake_delay = 0;
        nabto_yield(20);
      
        if (++fake_delay >= 450) { //3000
            fake_delay = 0;
            *appreq = saved_app_req;
            NABTO_LOG_INFO(("Application poll query: Response ready"));
            return true;
        }
    }
    return false;
}

// Retrieve the response from a queued request
application_event_result application_poll(application_request* request, unabto_query_request* r_b, unabto_query_response* w_b)
{
    application_event_result res;

    if (saved_app_req == 0) {
        NABTO_LOG_FATAL(("No queued request"));
        return AER_REQ_SYSTEM_ERROR;
    }

    res = demo_application(request, r_b, w_b);
    if (res == AER_REQ_RESPONSE_READY) {
        NABTO_LOG_INFO(("Application poll: Response delivered"));
    }
    saved_app_req = 0;
    return res;
}

// Drop the queued request (framework discarded it internally)
void application_poll_drop(application_request* request)
{
    NABTO_LOG_INFO(("Application poll drop: Response discarded"));
    saved_app_req = 0;
}

/** Synchronous event model - just call the demo application directly */

#else

application_event_result application_event(application_request* request, unabto_query_request* r_b, unabto_query_response* w_b)
{
    return demo_application(request, r_b, w_b);
}

#endif /* NABTO_APPLICATION_EVENT_MODEL_ASYNC */

void setTimeFromGSP(uint32_t time)
{
}
