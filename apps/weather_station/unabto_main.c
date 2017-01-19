/**
 * @file
 *
 * The uNabto ServerPeer Example Implementation.
 *
 * Using C++ and boost structures for convenience
 * - but not in nabto_event() implementing the uNabto protocol
 *
 */
#ifdef WIN32
#include <winsock2.h>
#endif
#include "unabto/unabto_common_main.h"
#include "unabto/unabto_logging.h"
#include "unabto/unabto_environment.h"
#include "modules/cli/unabto_args.h"
#include "unabto_version.h"
#include "unabto/unabto_app_adapter.h"

#include "unabto_weather_station.h"

#if NABTO_ENABLE_STREAM
#include "unabto_stream_test.h" /* for stream testing */
#endif

/* main() example. */

/**
 * Entry point.
 * @param argc  number of parameters
 * @param argv  the parameters
 * - 1 ip-address
 * - 2 serverid | - 
 * - 3 log
 * @return      the result
 */
int main(int argc, char* argv[])
{
    nabto_main_setup* nms;
    nabto_endpoint localEp;

    // flush stdout
    setvbuf(stdout, NULL, _IONBF, 0);

    nms = unabto_init_context();

    /**
     * Overwrite default values with command line args
     */
    if (!check_args(argc, argv, nms)) {
        return 1;
    }

    if (!unabto_init()) {
        NABTO_LOG_FATAL(("Failed at nabto_main_init"));
    }

    localEp.addr=nms->ipAddress;
    localEp.port=nms->localPort;
    
    if (nms->ipAddress != INADDR_ANY) NABTO_LOG_INFO(("Own IP address: " PRIep, MAKE_EP_PRINTABLE(localEp)));
    else NABTO_LOG_INFO(("Own IP address, local IP is set to INADDR_ANY: " PRIep, MAKE_EP_PRINTABLE(localEp)));

    while (true) {
        /*
         * Here the main application should do it's work
         * and then whenever time is left for Nabto (but at least regularly)
         * the nabto_main_tick() should be called.
         *
         * nabto_main_tick()
         * - polls the socket for incoming messages
         * - administers timers to do its own timer based work
         */
        unabto_tick();
#if NABTO_ENABLE_STREAM
        nabto_stream_test_tick();
#endif
#if (_POSIX_C_SOURCE >= 199309L)
        struct timespec sleepTime;
        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = 10*1000000;
        nanosleep(&sleepTime, NULL);
#endif
    }

    unabto_close();
    return 0;
}
