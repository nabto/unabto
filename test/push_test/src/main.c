/**
 *  Implementation of main for uNabto SDK demo
 */
#include "modules/push_service/push_service.h"
#include "unabto/unabto_env_base.h"
#include "unabto/unabto_common_main.h"
#include "unabto/unabto_logging.h"
#include "unabto_version.h"
#include "modules/cli/gopt/gopt.h"
#include <modules/util/read_hex.h>
#include <unabto/unabto_app.h>
#include "unabto/unabto_push.h"
#include <unabto/unabto_protocol_defines.h>
#include <unabto/unabto_util.h>

#ifdef WIN32
#elif defined(__MACH__)
#else
#include <sched.h>
#endif

struct configuration {
    const char *device_id;
    const char *pre_shared_key;
    const char *notification;
    const char *pnsid_str;
};

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
static void help(const char* errmsg, const char *progname);
bool parse_argv(int argc, char* argv[], struct configuration* config);

/**
 *  main using gopt to check command line arguments
 *  -h for help
 */
int main(int argc, char* argv[])
{
    struct configuration conf;
    memset(&conf, 0, sizeof(struct configuration));
    // Overwrite default values with command line args
    if (!parse_argv(argc, argv, &conf)) {
        help(0, argv[0]);
        return 1;
    }

    // Set nabto to default values
    nabto_main_setup* nms = unabto_init_context(); 
    nms->id = strdup(conf.device_id);

    nms->secureAttach = true;
    nms->secureData = true;
    nms->cryptoSuite = CRYPT_W_AES_CBC_HMAC_SHA256;

    if (!unabto_read_psk_from_hex(conf.pre_shared_key, nms->presharedKey, 16)) {
        help("Invalid cryptographic key specified", argv[0]);
        return false;
    }

    uint16_t pnsID = 0;

    if (conf.pnsid_str) {
        pnsID = atoi(conf.pnsid_str);
    } else {
        help("No PNS ID provided", argv[0]);
        return 1;
    }

    int testContext = 1;
    push_message pm;
    if(!init_push_message(&pm, pnsID, conf.notification)){
        NABTO_LOG_ERROR(("init_push_message failed"));
        return 1;
    }

    // Initialize nabto
    if (!unabto_init()) {
        NABTO_LOG_FATAL(("Failed at nabto_main_init"));
    }

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

bool parse_argv(int argc, char* argv[], struct configuration* config) {
    const char x0s[] = "h?";     const char* x0l[] = { "help", 0 };
    const char x1s[] = "d";      const char* x1l[] = { "deviceid", 0 };
    const char x2s[] = "k";      const char* x2l[] = { "presharedkey", 0 };
    const char x3s[] = "n";      const char* x3l[] = { "notification", 0 };
    const char x4s[] = "i";      const char* x4l[] = { "PNS ID", 0 };

    const struct { int k; int f; const char *s; const char*const* l; } opts[] = {
        { 'h', 0,           x0s, x0l },
        { 'd', GOPT_ARG,    x1s, x1l },
        { 'k', GOPT_ARG,    x2s, x2l },
        { 'n', GOPT_ARG,    x3s, x3l },
        { 'i', GOPT_ARG,    x4s, x4l },
        { 0, 0, 0, 0 }
    };
    void *options = gopt_sort( & argc, (const char**)argv, opts);

    if( gopt( options, 'h')) {
        help(0, argv[0]);
        exit(0);
    }

    if (!gopt_arg(options, 'd', &config->device_id)) {
        return false;
    }

    if (!gopt_arg(options, 'k', &config->pre_shared_key)) {
        return false;
    }

    if (!gopt_arg(options, 'n', &config->notification)) {
        return false;
    }

    if (!gopt_arg(options, 'i', &config->pnsid_str)) {
        return false;
    }


    return true;
}

static void help(const char* errmsg, const char *progname)
{
    if (errmsg) {
        printf("ERROR: %s\n", errmsg);
    }
    printf("\nPush notification test application.\n");

    printf("Usage: %s -d <device id> -k <crypto key> -n <json notification to send> -i <pns id>\n\n", progname);
}
