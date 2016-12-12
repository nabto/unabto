#include <unabto/unabto_env_base.h>
#include <unabto/unabto_app.h>
#include <unabto/unabto_common_main.h>

#include <modules/cli/unabto_args.h>

#include <time.h>

void crossSleep(int ms) {
    struct timespec sleepTime;
    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = ms*1000000;
    nanosleep(&sleepTime, NULL);
}

int main(int argc, char** argv) {
    nabto_main_setup* setup = unabto_init_context();
    if (!check_args(argc, argv, setup)) {
        NABTO_LOG_FATAL(("Could not parse the arguments from the commandline."));
    }

    if (!unabto_init()) {
        NABTO_LOG_FATAL(("unabto_init failed."));
    }
    while(true) {
        unabto_tick();
        crossSleep(10);
    }

    unabto_close();
}

application_event_result application_event(application_request* appreq, unabto_query_request* r_b, unabto_query_response* w_b) {
    switch (appreq->queryId) {
        // readExample.json
        case 1: {
            if (!unabto_query_write_uint8(w_b, 1)) {
                return AER_REQ_RSP_TOO_LARGE;
            }
            return AER_REQ_RESPONSE_READY;
        }
        
        // writeExample.json
        case 2: {
            uint8_t foo;
            if (!unabto_query_read_uint8(r_b, &foo)) {
                return AER_REQ_TOO_SMALL;
            }
            return AER_REQ_RESPONSE_READY;
        }
            
        // readMultipleExample.json
        case 3: {
            if (!unabto_query_write_uint8(w_b, 1) ||
                !unabto_query_write_uint8(w_b, 2)) {
                return AER_REQ_RSP_TOO_LARGE;
            }
            
            return AER_REQ_RESPONSE_READY;
        }
        
        // requestResponseExample.json
        case 4: {
            uint8_t foo, bar;
            
            if (!unabto_query_read_uint8(r_b, &foo) ||
                !unabto_query_read_uint8(r_b, &bar)) {
                return AER_REQ_TOO_SMALL;
            }
            
            if (!unabto_query_write_uint8(w_b, foo+1) ||
                !unabto_query_write_uint8(w_b, bar+1)) {
                return AER_REQ_RSP_TOO_LARGE;
            }
            return AER_REQ_RESPONSE_READY;
        }
        
        // listExample.json
        case 5: {
            uint16_t listLength;
            uint8_t* listItems;
            unabto_list_ctx listCtx;
            uint16_t i;

            // Read list length then each item in list
            unabto_query_read_list_length(r_b, &listLength);
            listItems = (uint8_t*)malloc(listLength*sizeof(uint8_t));

            for (i = 0; i < listLength; i++) {
                if (!unabto_query_read_uint8(r_b, &listItems[i])) {
                    return AER_REQ_TOO_SMALL;
                }
            }

            // Increment each item by one for demo purpose
            for (i = 0; i < listLength; i++) {
                listItems[i]++;
            }
            
            // Start list, write each item to list and end list
            unabto_query_write_list_start(w_b, &listCtx);
            for (i = 0; i < listLength; i++) {
                if (!unabto_query_write_uint8(w_b, listItems[i])) {
                    return AER_REQ_RSP_TOO_LARGE;
                }
            }
            unabto_query_write_list_end(w_b, &listCtx, i);

            free(listItems);
            return AER_REQ_RESPONSE_READY;
        }
        
        // rawExample.json
        case 6: {
            uint16_t length;
            uint8_t *buffer;
            uint8_t *data;
            
            // Receive pointer to raw data and data length
            if (!unabto_query_read_uint8_list(r_b, &buffer, &length)) {
                return AER_REQ_TOO_SMALL;
            }
            data = (uint8_t*)malloc(length * sizeof(uint8_t));
            memcpy(data, buffer, length);
            
            // Change data for demo purpose
            data[0] = '1';
            
            // Write back raw data
            if (!unabto_query_write_uint8_list(w_b, data, length)) {
                return AER_REQ_RSP_TOO_LARGE;
            }
            
            free(data);
            return AER_REQ_RESPONSE_READY;
        }

        // advancedListExample.json
        case 7: {
            unabto_list_ctx listCtx, listCtxInner;
            uint32_t listItems[] = {1, 2, 3, 4};
            uint8_t string1[] = "foo";
            uint8_t string2[] = "bar";
            uint16_t i;
            
            // Start outer list
            unabto_query_write_list_start(w_b, &listCtx);

            // Write single integer parameter
            if (!unabto_query_write_uint8(w_b, 123)) {
                return AER_REQ_RSP_TOO_LARGE;
            }

            // Start 1st inner list and write some values
            unabto_query_write_list_start(w_b, &listCtxInner);
            for (i = 0; i < 4; i++) {
                if (!unabto_query_write_uint32(w_b, listItems[i])) {
                    return AER_REQ_RSP_TOO_LARGE;
                }
            }
            unabto_query_write_list_end(w_b, &listCtxInner, i);

            // Start 2nd inner list and write raw values
            unabto_query_write_list_start(w_b, &listCtxInner);
            if (!unabto_query_write_uint8_list(w_b, string1, 4) ||
                !unabto_query_write_uint8_list(w_b, string2, 4)) {
                return AER_REQ_RSP_TOO_LARGE;
            }
            unabto_query_write_list_end(w_b, &listCtxInner, 2);

            // End outer list
            unabto_query_write_list_end(w_b, &listCtx, 1);

            return AER_REQ_RESPONSE_READY;
        }
        
        // rawJsonExample.json
        case 8: {
            uint8_t string1[] = "{points : [ {\"x\":1, \"y\":1}, {\"x\":2, \"y\":2}, {\"x\":3, \"y\":3}, {\"x\":4, \"y\":4} ]}";
            
            if (!unabto_query_write_uint8_list(w_b, string1, strlen((const char*)string1))) {
                return AER_REQ_RSP_TOO_LARGE;
            }
            return AER_REQ_RESPONSE_READY;
        }
    }
    return AER_REQ_NO_QUERY_ID;
}
