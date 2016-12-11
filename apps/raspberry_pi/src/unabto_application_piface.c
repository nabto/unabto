/**
 *  uNabto application logic implementation
 */

#include "unabto/unabto_app.h"
#include <libpiface-1.0/pfio.h>
#include <stdio.h>

// Function prototypes
uint8_t writeDigitalOutput(uint8_t id, uint8_t onOff);
uint8_t readDigitalOutput(uint8_t id);
uint8_t readDigitalInput(uint8_t id);

// The virtual light bulb variable
static uint8_t theLight = 0;

/***************** The uNabto application logic *****************
 * This is where the user implements his/her own functionality
 * to the device. When a Nabto message is received, this function
 * gets called with the message's request id and parameters.
 * Afterwards a user defined message can be sent back to the
 * requesting browser.
 ****************************************************************/
application_event_result application_event(application_request* request, unabto_query_request* read_buffer, unabto_query_response* write_buffer) {
    switch(request->queryId) {
        case 1: {
		
			/**********
			<query name="digital_input.json" description="Digital Input" id="1">
			<request>
			</request>
			<response>
			  <parameter name="pin0_state" type="uint8"/>
			  <parameter name="pin1_state" type="uint8"/>
			  <parameter name="pin2_state" type="uint8"/>
			  <parameter name="pin3_state" type="uint8"/>
			  <parameter name="pin4_state" type="uint8"/>
			  <parameter name="pin5_state" type="uint8"/>
			  <parameter name="pin6_state" type="uint8"/>
			  <parameter name="pin7_state" type="uint8"/>
			</response>
			</query>
			************/	
  
			uint8_t i;
			uint8_t state;
			for (i=0; i<8; i++)
			{
				state = readDigitalInput(i);
				if (!unabto_query_write_uint8(write_buffer, state)) {
					return AER_REQ_RSP_TOO_LARGE;
				}
			}

            return AER_REQ_RESPONSE_READY;
        }
		
        case 2: {
		
			/**********
			<query name="digital_output_write.json" description="Turn light on and off" id="2">
			<request>
				<parameter name="pin_id" type="uint8"/>
				<parameter name="value" type="uint8"/>
			</request>
			<response>
				<parameter name="pin_state" type="uint8"/>
			</response>
			</query>
			**********/
  
            uint8_t id;
            uint8_t state;

            // Read parameters in request
            if (!unabto_query_read_uint8(read_buffer, &id)) return AER_REQ_TOO_SMALL;
			if (!unabto_query_read_uint8(read_buffer, &state)) return AER_REQ_TOO_SMALL;

            // Read pin state
			uint8_t newState;
            newState = writeDigitalOutput(id, state);

            // Write back pin state
            if (!unabto_query_write_uint8(write_buffer, newState)) return AER_REQ_RSP_TOO_LARGE;

            return AER_REQ_RESPONSE_READY;
        }
		
        case 3: {
			/**********
			<query name="digital_output_read.json" description="Read light status" id="3">
			<request>
			</request>
			<response>
			  <parameter name="pin0_state" type="uint8"/>
			  <parameter name="pin1_state" type="uint8"/>
			  <parameter name="pin2_state" type="uint8"/>
			  <parameter name="pin3_state" type="uint8"/>
			  <parameter name="pin4_state" type="uint8"/>
			  <parameter name="pin5_state" type="uint8"/>
			  <parameter name="pin6_state" type="uint8"/>
			  <parameter name="pin7_state" type="uint8"/>
			</response>
			</query>
			************/   
  
			uint8_t i;
			uint8_t state;
			for (i=0; i<8; i++)
			{
					state = readDigitalOutput(i);
					if (!unabto_query_write_uint8(write_buffer, state)) {
							return AER_REQ_RSP_TOO_LARGE;
					}
			}

            return AER_REQ_RESPONSE_READY;
        }
		
    }
    return AER_REQ_INV_QUERY_ID;
}

// Set digital output-pin and return state,
uint8_t writeDigitalOutput(uint8_t id, uint8_t onOff) {
    uint8_t state;
    state = onOff;
	pfio_digital_write(id, state);

    uint8_t binaryRead;
    binaryRead = pfio_read_output();
    if( (binaryRead & (1 << (id))) == (1 << (id)) ) 
        state = 1;
    else 
        state = 0;

    return state;
}

// Set return state of digital output-pin,
uint8_t readDigitalOutput(uint8_t id) {
    uint8_t state;

    uint8_t binaryRead;
    binaryRead = pfio_read_output();
    if( (binaryRead & (1 << (id))) == (1 << (id)) ) 
		state = 1;
    else
		state = 0;

    return state;
}

// Return input-pin state,
uint8_t readDigitalInput(uint8_t id) {
    uint8_t state;
    state = pfio_digital_read(id);

    return state;
}

void setTimeFromGSP(uint32_t time)
{
}
