#include <unabto_platform_types.h>
#include <unabto/unabto_common_main.h>
#include <unabto/unabto_app.h>
#include <modules/network/epoll/unabto_epoll.h>

#include <modules/tunnel/unabto_tunnel.h>
#include <modules/tunnel/unabto_tunnel_epoll.h>
#include <modules/util/read_hex.h>

#include "param.h"
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

char* appname = "axistunnel";

static void
handle_sigterm(int signo)
{
    exit(1);
}

static void
init_signals(void)
{
    struct sigaction sa;
    
    sa.sa_flags = 0;
    
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handle_sigterm;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
}

int main(int argc, char* argv[]) {
    openlog(appname, LOG_PID, LOG_LOCAL4);

    init_signals();
    
    daemon(0,0);

    if (param_init(appname) < 0) {
        syslog(LOG_CRIT, "Could not initialize parameter handling\n");
        exit(1);
    }

    nabto_main_setup* nms = unabto_init_context();
    
    char* deviceid;
    char* preSharedKey; 
    if (param_get ("deviceid", &deviceid) != 0) {
        syslog(LOG_CRIT, "Could not get parameter deviceid\n");
        exit(1);
    }

    nms->id = strdup(deviceid);
    param_free(deviceid);

    if (param_get ("presharedkey", &preSharedKey) != 0) {
        syslog(LOG_CRIT, "Could not get parameter sharedkey\n");
        exit(1);
    }

    if (!unabto_read_psk_from_hex(preSharedKey, nms->presharedKey, 16)) {
        syslog(LOG_CRIT, "Could not parse parameter sharedkey\n");
        exit(1);
    }
    
    nms->cryptoSuite = CRYPT_W_AES_CBC_HMAC_SHA256;
    nms->secureAttach = true;
    nms->secureData = true;
    
    param_free(preSharedKey);

    tunnel_loop_epoll();
    return 0;
}


bool tunnel_allow_connection(const char* host, int port) {
    if (strcmp(host, "127.0.0.1") == 0 || strcmp(host, "127.0.0.1") == 0) {
        return true;
    }
    return false;
}


application_event_result application_event(application_request* request, unabto_query_request* readBuffer, unabto_query_response* writeBuffer)
{
    return AER_REQ_INV_QUERY_ID;
}

bool application_poll_query(application_request** applicationRequest) {
    return false;
}

application_event_result application_poll(application_request* applicationRequest, unabto_query_request* readBuffer, unabto_query_response* writeBuffer) {
    return AER_REQ_INV_QUERY_ID;
}

void application_poll_drop(application_request* applicationRequest) {
}
