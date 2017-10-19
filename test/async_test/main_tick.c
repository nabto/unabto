#include "unabto/unabto_common_main.h"
#include "unabto/unabto_logging.h"
#include "unabto/unabto_environment.h"
#include "modules/cli/unabto_args.h"
#include "unabto_version.h"
#include "unabto/unabto_app_adapter.h"
#include "async_application_event.h"

void init_request_queue();
    
int main(int argc, char* argv[])
{
    nabto_main_setup* nms;

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

    init_request_queue();
    
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
