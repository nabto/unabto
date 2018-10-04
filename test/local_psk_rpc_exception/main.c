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

application_event_result application_event(application_request* appreq,
                                           unabto_query_request* request,
                                           unabto_query_response* response) {
    switch (appreq->queryId) {
    case 0: { // ok.json
        if (!unabto_query_write_uint8(response, 1)) {
            return AER_REQ_RSP_TOO_LARGE;
        }
        return AER_REQ_RESPONSE_READY;            
    }
    case 10: return AER_REQ_NO_ACCESS;
    case 20: return AER_REQ_OUT_OF_RESOURCES;
    default: return AER_REQ_NO_QUERY_ID;
    }
}

bool unabto_local_psk_connection_get_key(const struct unabto_psk_id* keyId, const char* clientId, const struct unabto_optional_fingerprint* pkFp, struct unabto_psk* key) {
    memset(&key->data, 0, 16);
    return true;
}
