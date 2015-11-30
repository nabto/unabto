/**
 *  uNabto application logic implementation
 */

#include <unabto/unabto_app.h>
#include <unabto/unabto_external_environment.h>
#include <unabto/unabto_env_base.h>
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
application_event_result application_event(application_request* request, buffer_read_t* read_buffer, buffer_write_t* write_buffer) {
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
            if (!buffer_read_uint8(read_buffer, &light_id)) {
                NABTO_LOG_ERROR(("Can't read light_id, the buffer is too small"));
                return AER_REQ_TOO_SMALL;
            }
            if (!buffer_read_uint8(read_buffer, &light_on)) {
                NABTO_LOG_ERROR(("Can't read light_state, the buffer is too small"));
                return AER_REQ_TOO_SMALL;
            }

            // Set light according to request
            light_state = setLight(light_id, light_on);

            // Write back led state
            if (!buffer_write_uint8(write_buffer, light_state)) {
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
            if (!buffer_read_uint8(read_buffer, &light_id)) return AER_REQ_TOO_SMALL;

            // Read light state
            light_state = readLight(light_id);

            // Write back led state
            if (!buffer_write_uint8(write_buffer, light_state)) return AER_REQ_RSP_TOO_LARGE;

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

void setTimeFromGSP(uint32_t time) {
}
