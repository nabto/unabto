/**
 *  uNabto application logic implementation
 */
#include "Nano1X2Series.h"
#include "unabto_application.h"

// The virtual light bulb variable
//static uint8_t theLight = 0;

application_event_result application_event(application_request* request, unabto_query_request* read_buffer, unabto_query_response* write_buffer) {
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

uint8_t setLight(uint8_t id, uint8_t onOff) {
    PD10 = !onOff;
    NABTO_LOG_INFO((onOff?("Nabto: Light turned ON!"):("Nabto: Light turned OFF!")));
#if NABTO_ENABLE_LOGGING == 0
    print_Line(3, onOff?("Light ON "):("Light OFF"));
#endif
    return onOff;
}

uint8_t readLight(uint8_t id) {
    return !PD10;
}
