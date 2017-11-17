/**
 *  Implementation of main for uNabto SDK demo
 */
#include "modules/push_service/push_service.h"
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

bool isCallbackReceived = false;
void callback(void* ptr,const unabto_push_hint* hint){
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
    NABTO_LOG_INFO(("Got the context value: %i",*(int*)ptr));
    isCallbackReceived = true;
}

void nabto_yield(int msec);

/**
 *  main using gopt to check command line arguments
 *  -h for help
 */
int main(int argc, char* argv[])
{
    // Set nabto to default values
    nabto_main_setup* nms = unabto_init_context(); 

    // setting push notification data 
    uint16_t pnsid = UNABTO_PUSH_PNS_ID_FIREBASE;
    int testContext = 1;
    const char* sd = "{\"to\": \"58a88e8b-83f1-429d-8863-8d8180ae83ed\"}";
    const char* titleKey = "title_1";
    const char* bodyKey = "body_1";
    const char* bodyArg1 = "943";
    const char* bodyArg2 = "349";

    push_message pm;
    if(!init_push_message(&pm, pnsid,sd)){
        NABTO_LOG_ERROR(("init_push_message failed"));
        return 1;
    }
    if(!add_title_loc_key(&pm, titleKey)){
        NABTO_LOG_ERROR(("add_title_loc_key failed"));
        return 1;
    }
    if(!add_body_loc_key(&pm, bodyKey)){
        NABTO_LOG_ERROR(("add_body_loc_key failed"));
        return 1;
    }
    if(!add_body_loc_string_arg(&pm, bodyArg1)){
        NABTO_LOG_ERROR(("add_body_loc_string_arg failed"));
        return 1;
    }
    if(!add_body_loc_string_arg(&pm, bodyArg2)){
        NABTO_LOG_ERROR(("add_body_loc_string_arg failed"));
        return 1;
    }

    // Overwrite default values with command line args
    if (!check_args(argc, argv, nms)) {
        return 1;
    }
    NABTO_LOG_INFO(("Identity: '%s'", nms->id));
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
            if(first){
                NABTO_LOG_ERROR(("Timeout before getting attached"));
                exit(3);
            } else {
                NABTO_LOG_ERROR(("Timeout before Push ack was received."));
                exit(4);
            }
        }
        if (unabto_is_connected_to_gsp() && first) {
            NABTO_LOG_INFO(("Successfully attached to the gsp, sending push notification"));
//            send_push_notification(pnsid,staticData,msg,&callback, (void*)&testContext);
            send_push_message(&pm, &callback, (void*)&testContext);
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
