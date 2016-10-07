/**
 *  Implementation of main for uNabto SDK demo
 */

#include "unabto/unabto_env_base.h"
#include "unabto/unabto_common_main.h"
#include "unabto/unabto_logging.h"
#include "unabto_version.h"
#include "modules/cli/unabto_args.h"
#include <unabto/unabto_app.h>

#ifdef WIN32
#elif defined(__MACH__)
#else
#include <sched.h>
#endif

void nabto_yield(int msec);

/**
 *  main using gopt to check command line arguments
 *  -h for help
 */
int main(int argc, char* argv[])
{
    // Set nabto to default values

    nabto_main_setup* nms = unabto_init_context();

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

    nabto_stamp_t attachExpireStamp;
    // 20 seconds
    nabtoSetFutureStamp(&attachExpireStamp, 1000*20);
        
    // The main loop gives nabto a tick from time to time.
    // Everything else is taken care of behind the scenes.
    while (true) {
        unabto_tick();
        nabto_yield(10);
        if (nabtoIsStampPassed(&attachExpireStamp)) {
            NABTO_LOG_ERROR(("Attach took too long."));
            exit(4);
        }
        if (unabto_is_connected_to_gsp()) {
            NABTO_LOG_INFO(("Successfully attached to the gsp, now exiting."));
            exit(0);
        }
    }

    NABTO_LOG_ERROR(("we should not end here"));
    exit(5);
    
    unabto_close();
    return 0;
}

application_event_result application_event(application_request* request, buffer_read_t* read_buffer, buffer_write_t* write_buffer) {
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
