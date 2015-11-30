#include <unabto/unabto_common_main.h>
#include <modules/cli/unabto_args.h>
#include <unabto/unabto_app.h>
#include "stream_echo.h"

int main(int argc, char** argv) {
    nabto_main_setup* nms = unabto_init_context();

    stream_echo_init();
    
    if (!check_args(argc, argv, nms)) {
        return 1;
    }

    if (!unabto_init()) {
        return 1;
    }
    
    while(true) {
        unabto_tick();
#ifdef WIN32
        Sleep(1);
#endif
#if (_POSIX_C_SOURCE >= 199309L)
        struct timespec sleepTime;
        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = 10*1000000;
        nanosleep(&sleepTime, NULL);
#endif
    }
    
    unabto_close();
}

application_event_result application_event(application_request* applicationRequest, buffer_read_t* readBuffer, buffer_write_t* writeBuffer) {
    return AER_REQ_INV_QUERY_ID;
}
