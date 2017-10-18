#include "async_application_event.h"

#include "unabto/unabto_common_main.h"
#include "unabto/unabto_logging.h"
#include "unabto/unabto_environment.h"
#include "modules/cli/unabto_args.h"
#include "unabto_version.h"
#include "unabto/unabto_app_adapter.h"
#include <modules/timers/auto_update/unabto_time_auto_update.h>
#include <modules/network/select/unabto_network_select_api.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#ifdef WIN32
#include <windows.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/resource.h>
#endif

void wait_event();

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
    
    init_request_queue();

    unabto_time_auto_update(false);
    while(true) {
        unabto_time_update_stamp();
        wait_event();
    }
    unabto_close();
}

void wait_event()
{
    int timeout;
    nabto_stamp_t ne;
    nabto_stamp_t now;
    int nfds;
    int max_read_fd = 0;
    int max_write_fd = 0;
    fd_set read_fds;
    fd_set write_fds;
    struct timeval timeout_val;
    
    unabto_next_event(&ne);
    update_next_async_event_timeout(&ne);
    now = nabtoGetStamp();
    timeout = nabtoStampDiff2ms(nabtoStampDiff(&ne, &now));
    if (timeout < 0) timeout = 0;

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);

    
    unabto_network_select_add_to_read_fd_set(&read_fds, &max_read_fd);
    
#if NABTO_ENABLE_TCP_FALLBACK
    unabto_tcp_fallback_select_add_to_read_fd_set(&read_fds, &max_read_fd);
    unabto_tcp_fallback_select_add_to_write_fd_set(&write_fds, &max_write_fd);
#endif

    timeout_val.tv_sec = (timeout/1000);
    timeout_val.tv_usec = ((timeout)%1000)*1000;

    nfds = select(MAX(max_read_fd+1, max_write_fd+1), &read_fds, &write_fds, NULL, &timeout_val);

    NABTO_LOG_TRACE(("foobar %i", nfds));
    if (nfds < 0) NABTO_LOG_FATAL(("Error in epoll_wait: %d", errno));
    unabto_network_select_read_sockets(&read_fds);
    
    unabto_time_event();
}

