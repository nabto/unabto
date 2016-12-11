/**
 * uNabto application logic implementation
 */

#include "unabto/unabto_app.h"
#include "unabto/unabto_util.h"
#include "unabto/unabto_app_adapter.h"

// Function prototypes
uint8_t setLed(uint8_t led_id, uint8_t led_on);
uint8_t readLed(uint8_t led_id);

/***************** The uNabto application logic *****************
 * This is where the user implements his/her own functionality
 * to the device. When a Nabto message is received, this function
 * gets called with the message's request id and parameters.
 * Afterwards a user defined message can be sent back to the
 * requesting browser.
 ****************************************************************/
application_event_result application_event(application_request* request, unabto_quer_request* read_buffer, unabto_query_response* write_buffer) {
    switch(request->queryId) {
        case 1: {
            //  <query name="light_write.json" description="Turn light on and off" id="1">
            //    <request>
            //      <parameter name="light_id" type="uint8"/>
            //      <parameter name="light_on" type="uint8"/>
            //	  </request>
            //    <response>
            //      <parameter name="light_state" type="uint8"/>
            //    </response>
            //  </query>

            uint8_t light_id;
            uint8_t light_on;
            uint8_t light_state;

            // Read parameters in request
            if (!unabto_query_read_uint8(read_buffer, &light_id)) return AER_REQ_TOO_SMALL;
            if (!unabto_query_read_uint8(read_buffer, &light_on)) return AER_REQ_TOO_SMALL;

            // Set onboard led according to request
            light_state = setLed(light_id, light_on);

            // Write back led state
            if (!unabto_query_write_uint8(write_buffer, light_state)) return AER_REQ_RSP_TOO_LARGE;

            return AER_REQ_RESPONSE_READY;
        }
        case 2: {
            //  <query name="light_read.json" description="Read light status" id="2">
            //    <request>
            //      <parameter name="light_id" type="uint8"/>
            //	  </request>
            //    <response>
            //      <parameter name="light_state" type="uint8"/>
            //    </response>
            //  </query>

            uint8_t light_id;
            uint8_t light_state;

            // Read parameters in request
            if (!unabto_query_read_uint8(read_buffer, &light_id)) return AER_REQ_TOO_SMALL;

            light_state = readLed(light_id);

            // Write back led state
            if (!unabto_query_write_uint8(write_buffer, light_state)) return AER_REQ_RSP_TOO_LARGE;

            return AER_REQ_RESPONSE_READY;
        }
    }
}

// Set first onboard LED and return state,
// only using ID #1 in this simple example
uint8_t setLed(uint8_t led_id, uint8_t led_on) {
    if (led_id == 1) {
        LED0_IO = led_on;
        return LED0_IO;
    }
    return -1;
}

// Return first onboard LED state,
// only using ID #1 in this simple example
uint8_t readLed(uint8_t led_id) {
    if (led_id == 1) {
        return LED0_IO;
    }
    return -1;
}
