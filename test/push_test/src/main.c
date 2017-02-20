/**
 *  Implementation of main for uNabto SDK demo
 */
#include "unabto/unabto_env_base.h"
#include "unabto/unabto_common_main.h"
#include "unabto/unabto_logging.h"
#include "unabto_version.h"
#include "modules/cli/unabto_args.h"
#include <unabto/unabto_app.h>
#include "unabto/unabto_push.h"
#include <unabto/unabto_protocol_defines.h>
#include <unabto/unabto_util.h>

#ifdef WIN32
#elif defined(__MACH__)
#else
#include <sched.h>
#endif

#define PUSH_NOTIFICATION_TABLE_SIZE 2

uint8_t* dataBuffer[PUSH_NOTIFICATION_TABLE_SIZE];
uint32_t sequences[PUSH_NOTIFICATION_TABLE_SIZE];
uint16_t lengths[PUSH_NOTIFICATION_TABLE_SIZE];
uint8_t dataHead = 0;
bool isCallbackReceived = false;

uint8_t data[128];

enum {
    PUSH_DATA_PURPOSE_STATIC = 1,
    PUSH_DATA_PURPOSE_MSG = 2
};

enum {
    PUSH_DATA_TYPE_JSON = 1
};

void send_push_notification_now(uint16_t pnsid, const char * staticData, size_t lenSD, const char * msg, size_t lenMsg ){
    uint32_t seq;
    uint8_t* ptr;
    if(dataHead >= PUSH_NOTIFICATION_TABLE_SIZE){
        NABTO_LOG_INFO(("Too many notifications"));
        return;
    }
    /*
    const uint8_t* staticData = "\"playerId\" : \"hasdgadsg\"";
    size_t lenSd = 24;
    const uint8_t* msg = "The roof is on fire";
    size_t lenMsg = 19;
    uint16_t pnsid = 1;
     */
    lengths[dataHead] = lenSD+lenMsg+2*NP_PAYLOAD_PUSH_DATA_SIZE_WO_DATA; 
    dataBuffer[dataHead] = (uint8_t *)malloc(lengths[dataHead]);
    ptr = dataBuffer[dataHead];
    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_PUSH_DATA, 0,lenSD+2);
    WRITE_U8(ptr, PUSH_DATA_PURPOSE_STATIC); ptr++;
    WRITE_U8(ptr, PUSH_DATA_TYPE_JSON); ptr++;
    memcpy(ptr,staticData,lenSD); ptr += lenSD;
    
    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_PUSH_DATA, 0,lenMsg+2);
    WRITE_U8(ptr, PUSH_DATA_PURPOSE_MSG); ptr++;
    WRITE_U8(ptr, PUSH_DATA_TYPE_JSON); ptr++;
    memcpy(ptr,msg,lenMsg); ptr += lenMsg;
    
    unabto_send_push_notification(pnsid, &seq);
    sequences[dataHead] = seq;
    dataHead++; 
}

uint8_t* unabto_push_notification_get_data(uint8_t* bufStart, const uint8_t* bufEnd, uint32_t seq){
    int i;
    for(i=0;i<dataHead;i++){
        if(sequences[i] == seq){
            if(lengths[i]>bufEnd-bufStart){
                NABTO_LOG_ERROR(("Too much data"));
                return NULL;
            }
            memcpy(bufStart, dataBuffer[i], lengths[i]);
            return bufStart+lengths[i];
        }
    }
    NABTO_LOG_ERROR(("Did not find the seq for get data"));
    return NULL;
}

void unabto_push_notification_callback(uint32_t seq, unabto_push_hint* hint){
    int i;
    switch(*hint){
        case UNABTO_PUSH_HINT_OK:
            NABTO_LOG_INFO(("Callback with hint: OK"));
            break;
        case UNABTO_PUSH_HINT_QUEUE_FULL:
            NABTO_LOG_INFO(("Callback with hint: QUEUE FULL"));
            break;
        case UNABTO_PUSH_HINT_INVALID_DATA_PROVIDED:
            NABTO_LOG_INFO(("Callback with hint: INVALID DATA PROVIDED"));
            break;
        case UNABTO_PUSH_HINT_NO_CRYPTO_CONTEXT:
            NABTO_LOG_INFO(("Callback with hint: NO CRYPTO CONTEXT"));
            break;
        case UNABTO_PUSH_HINT_ENCRYPTION_FAILED:
            NABTO_LOG_INFO(("Callback with hint: ENCRYPTION FAILED"));
            break;
        case UNABTO_PUSH_HINT_FAILED:
            NABTO_LOG_INFO(("Callback with hint: FAILED"));
            break;
        case UNABTO_PUSH_HINT_QUOTA_EXCEEDED:
            NABTO_LOG_INFO(("Callback with hint: QUOTA EXCEEDED"));
            break;
        case UNABTO_PUSH_HINT_QUOTA_EXCEEDED_REATTACH:
            NABTO_LOG_INFO(("Callback with hint: QUOTA EXCEEDED REATTACH"));
            break;
        default:
            NABTO_LOG_INFO(("Callback with unknown hint"));
    }            
    for(i=0;i<dataHead;i++){
        if(sequences[i] == seq){
            free(dataBuffer[i]);
            if(i<PUSH_NOTIFICATION_TABLE_SIZE-1){
                memmove(&dataBuffer[i], &dataBuffer[i+1],dataHead-i);
            }
            dataHead--;
            isCallbackReceived = true;
            return;
        }
    }   
}

void nabto_yield(int msec);

/**
 *  main using gopt to check command line arguments
 *  -h for help
 */
int main(int argc, char* argv[])
{
//{"App_id": "58a88e8b-83f1-429d-8863-8d8180ae83ed","Player_id":"a1925aa7-5126-46a7-99c7-ecb7f888299b"}
    // Set nabto to default values
    nabto_main_setup* nms = unabto_init_context();
    const uint8_t* staticData = "{\"App_id\": \"58a88e8b-83f1-429d-8863-8d8180ae83ed\",\"Player_id\":\"a1925aa7-5126-46a7-99c7-ecb7f888299b\"}";
    size_t lenSd = 101;
    const uint8_t* msg = "{\"temp\": 943}";
    size_t lenMsg = 13;
    uint16_t pnsid = 1;
    // Overwrite default values with command line args
    if (!check_args(argc, argv, nms)) {
        return 1;
    }
    NABTO_LOG_INFO(("Identity: '%s'", nms->id));
    NABTO_LOG_INFO(("Program Release %i.%i", RELEASE_MAJOR, RELEASE_MINOR));
    NABTO_LOG_INFO(("Buffer size: %i", nms->bufsize));

    // Initialize nabto
    if (!unabto_init()) {
        NABTO_LOG_FATAL(("Failed at nabto_main_init"));
    }
    unabto_push_init();
    nabto_stamp_t attachExpireStamp;
    // 20 seconds
    nabtoSetFutureStamp(&attachExpireStamp, 1000*20);
        
    // The main loop gives nabto a tick from time to time.
    // Everything else is taken care of behind the scenes.
    bool first = true;
    while (true) {
        unabto_tick();
        nabto_yield(10);
        if (nabtoIsStampPassed(&attachExpireStamp)) {
            NABTO_LOG_ERROR(("Push took too long."));
            exit(4);
        }
        if (unabto_is_connected_to_gsp() && first) {
            NABTO_LOG_INFO(("Successfully attached to the gsp, sending push notification"));
            send_push_notification_now(pnsid,staticData,lenSd,msg,lenMsg);
            first = false;
        }
        if (isCallbackReceived){
            NABTO_LOG_INFO(("Successfully received push callback"));
            exit(0);
        }
    }

    NABTO_LOG_ERROR(("we should not end here"));
    exit(5);
    
    unabto_close();
    return 0;
}

application_event_result application_event(application_request* request, unabto_query_request* read_buffer, unabto_query_response* write_buffer) {
    return AER_REQ_INV_QUERY_ID;
}
void nabto_yield(int msec)
{
#ifdef WIN32
    Sleep(msec);
#elif defined(__MACH__)
    if (msec) usleep(1000*msec);
#else
    if (msec) usleep(1000*msec); else sched_yield();
#endif
}
