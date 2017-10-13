#include "unabto_weather_station.h"
#include <unabto/unabto_app.h>
#include <unabto/unabto_util.h>
#include <unabto/unabto_logging.h>
#include <unabto/util/unabto_buffer.h>

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC

#endif

#include <stdio.h>

static int32_t lastTemperature_;

/**
 * Retrieve a temperature.
 * @param index  select the temperature
 * @return       the temperature
 */
int32_t calcTemperature(int index)
{
    static uint32_t t = 0;
    switch (index) {
        case 0:  return   36 + ++t;
        case 1:  return  199 + ++t;
        case 2:  return -234 + ++t;
        case 3:  return   -6 + ++t;
        case 4:  return  ++t;
        default: return  999;
    }
}

int32_t getTemperature(int index)
{
    lastTemperature_ = calcTemperature(index);
    return lastTemperature_;
}


/** Retrieve the windspeed. @return the windspeed */
uint32_t getWindSpeed()
{
    static uint32_t ws = 0;
    return ws++;
}

uint32_t getHumidity() 
{
    return 42;
}

/*return between 0 and 1000 */
uint32_t simulatedRand()  {
    return rand()%500;
}

uint32_t getTemperatureSimulated() {
    return 21250+simulatedRand();
}

uint32_t getWindSpeedSimulated() {
    return 7250+simulatedRand();
}

uint32_t getHumiditySimulated() {
    return 65250+simulatedRand();
}

/******************************************************************************/
/******************************************************************************/

application_event_result weather_station_application(application_request* request, unabto_query_request* read_buffer, unabto_query_response* write_buffer)
{
    switch (request->queryId) {
    case 1:
        /*
         * <query name="house_temperature" id="1"
         *        template="house_temperature_template_overrule.tpt">
         *   <request>
         *     <parameter name="sensor_id" type="uint32"/>
         *   </request>
         *   <response>
         *     <parameter name="temperature" type="int32"/>
         *   </response>
         * </query>
         */
        {
            int32_t temp;
            uint32_t ix;
            // read 4 bytes from the input buffer
            if (!unabto_query_read_uint32(read_buffer, &ix)) return AER_REQ_TOO_SMALL;
            // write 4 bytes to the output buffer
            temp = getTemperature(ix);
            if (!unabto_query_write_uint32(write_buffer, temp)) return AER_REQ_RSP_TOO_LARGE;
            return AER_REQ_RESPONSE_READY;
        }
        
    case 2:
        /*
         * <query name="wind_speed" id="2">
         *   <request>
         *   </request>
         *   <response>
         *     <parameter name="speed_m_s" type="uint32"/>
         *   </response>
         * </query>
         */
        if (!unabto_query_write_uint32(write_buffer, getWindSpeed())) return AER_REQ_RSP_TOO_LARGE;
        return AER_REQ_RESPONSE_READY;

    case 3:
        /**
         * <query name="weather_station" description="Weather station" id="3">
         *   <request>
         *   </request>
         *   <response>
         *     <parameter name="temperature"/>
         *     <parameter name="windspeed"/>
         *     <parameter name="humidity"/>
         *  </response>
         * </query>
         */
        if (unabto_query_write_uint32(write_buffer, getTemperatureSimulated()) &&
            unabto_query_write_uint32(write_buffer, getWindSpeedSimulated()) &&
            unabto_query_write_uint32(write_buffer, getHumiditySimulated())) {
            return AER_REQ_RESPONSE_READY;
        } else {
            return AER_REQ_RSP_TOO_LARGE;
        }

    case 4:
        return AER_REQ_RESPONSE_READY;
    }
    /**
     * if no opcode matched return it as an error
     */
    return AER_REQ_INV_QUERY_ID;
}



/******************************************************************************/
/******************************************************************************/
#if NABTO_APPLICATION_EVENT_MODEL_ASYNC == 0

application_event_result application_event(application_request* request, unabto_query_request* r_b, unabto_query_response* w_b)
{
    return weather_station_application(request, r_b, w_b);
}

#endif // NABTO_APPLICATION_EVENT_MODEL_ASYNC == 0
/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
/******************************************************************************/
#if NABTO_APPLICATION_EVENT_MODEL_ASYNC

/** used to hold the one and only request to be treated by the weather station application */
static application_request* saved_app_req = 0;


#define PGR_LOG_APPREQ(func, msg) NABTO_LOG_INFO(("APPREQ %" PRItext ": %" PRItext, #func, msg));
#define PGR_LOG_APPREQ_RES(func, res) NABTO_LOG_INFO(("APPREQ %" PRItext ": %d", #func, res));


struct {
    unabto_buffer        buf;
    unabto_query_request r_b;
    uint8_t       data[NABTO_REQUEST_MAX_SIZE];
} saved_params;


application_event_result application_event(application_request* request, unabto_query_request* r_b, unabto_query_response* w_b)
{
    // As an example - and for testing:
    // This implementation replies immediately to every second request whereas
    // the rest is saved to be answered on one of the next polls from unabto.
    // However, before saving a request, if another request is already saved (thus in progress),
    // the response will be a AER_REQ_OUT_OF_RESOURCES
    //
    static int test_xxx  = 0;
    int test_respond_now = ((++test_xxx) & 1);

    if (request->isLocal || test_respond_now) {
        PGR_LOG_APPREQ(application_event, "Respond immedeately");
        return weather_station_application(request, r_b, w_b);
    } else if (saved_app_req) {
        PGR_LOG_APPREQ(application_event, "No ressources");
        return AER_REQ_OUT_OF_RESOURCES;
    } else {
        PGR_LOG_APPREQ(application_event, "Accept request");
        saved_app_req = request;
        // IMPORTANT: w_b cannot be used when saving the request for later completion!
        //            the buffer designated by w_b may be overridden by the caller.
        {
            size_t sz = unabto_query_request_size(r_b);
            if (sz > sizeof(saved_params.data)) {
                return AER_REQ_TOO_LARGE;
            } else {
                // saves raw bytes from buffer (including already read data) into same type of data-holder,
                // because weather_station_application() neads a unabto_query_request* parameter
                // more realistic example would read/interpret parameters and copy to own structure.
                memcpy(saved_params.data, r_b->buffer->data, sz);
                buffer_init(&saved_params.buf, saved_params.data, (uint16_t)sz);
                unabto_query_request_init(&saved_params.r_b, &saved_params.buf);
                saved_params.r_b.position = r_b->position; // skip already read data
            }
        }
        return AER_REQ_ACCEPTED;
    }
}

bool application_poll_query(application_request** appreq)
{
    if (saved_app_req) {
        // for testing purpose, we simulate that the queued result is ready in first or second poll
        static int test_xxx  = 0;
        int test_deliver_now = (++test_xxx) & 1;

        if (test_deliver_now) {
            *appreq = saved_app_req;
            PGR_LOG_APPREQ(application_poll_query, "true");
            return true;
        }
    }
    return false;
}

application_event_result application_poll(application_request* request, unabto_query_response* w_b)
{
    application_event_result res = AER_REQ_SYSTEM_ERROR;
    if (saved_app_req == 0) {
        NABTO_LOG_FATAL(("No queued request"));
    } else if (request != saved_app_req) {
        NABTO_LOG_FATAL(("queued request and parameters doesn't match"));
    } else {
        unabto_query_request* r_b = &saved_params.r_b;
        res = weather_station_application(saved_app_req, r_b, w_b);
        PGR_LOG_APPREQ_RES(application_poll, (int)res);
        // hand the saved application request to the caller with the resulting response
        saved_app_req = 0;
    }
    return res;
}

void application_poll_drop(application_request* request)
{
    saved_app_req = 0;
}

#endif // NABTO_APPLICATION_EVENT_MODEL_ASYNC

enum {
    BODYLENGTH = 8
};

unabto_buffer* get_event_buffer2(size_t maximumLength)
{
    static uint8_t eventBufferData[NABTO_EVENT_CHANNEL_MAX_SIZE];
    static unabto_buffer eventBuffer;
    
    if(lastTemperature_ == 0)
    {
        return NULL; // application has no data to send to the base station
    }
    else
    {
        uint8_t body[BODYLENGTH];
            
        const char HEADER[] = "1:L:";
        const char TRAILER[] = "\nEOM\n";
        const uint16_t OVERHEAD = sizeof (HEADER) + sizeof (TRAILER) + 1; // make room for zero-termination during buffer build process
        
        buffer_write_t writeBuffer;

        sprintf((char*)body, "%d", lastTemperature_);
        
        // limit number of bytes to what uNabto indicates it can handle
        if((size_t)(BODYLENGTH + OVERHEAD) > maximumLength)
        {
            return NULL;
        }

        buffer_init(&eventBuffer, eventBufferData, sizeof (eventBufferData));
        buffer_write_init(&writeBuffer, &eventBuffer);
        buffer_write_text(&writeBuffer, HEADER);
        buffer_write_text(&writeBuffer, (char*)body);
        buffer_write_text(&writeBuffer, TRAILER);

        // remove zero-termination added by buffer write operations
        eventBuffer.size = writeBuffer.position;

        // return data to uNabto
        return &eventBuffer;
    }
}

/******************************************************************************/
/******************************************************************************/

