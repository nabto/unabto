#include <modules/tunnel/tunnel.h>
#include <modules/tunnel/tunnel_epoll.h>
#include <modules/network/epoll/unabto_epoll.h>
#include "tunnel_application_requests.h"
#include <unabto/unabto_app.h>

#include <unabto/unabto_common_main.h>
#include <unabto/unabto_attach.h>
#include <unabto_version.h>

#include <modules/cli/gopt/gopt.h>
#include <modules/diagnostics/unabto_diag.h>

#include <unabto/unabto_stream.h>

#include <errno.h>

#include <pthread.h>

static void resolve_dns(const char *id);
static void wait_event(nabto_main_setup *, int epfd);
void* main_routine(void* args);

static int counter = 1;

static uint32_t bsip;

void help() {
    printf("Usage: test_tunnels --basename=<...> --rangestart=<...> --rangeend=<...>\n");
    printf("  -h, --help          print help\n");
    printf("  --basename          common name for all devices e.g. add-service.u.nabto.net\n");
    printf("  --rangestart        start of device range\n");
    printf("  --rangeend          end of device range\n");
    printf("  -l, --nabtolog      logsetting to use\n");
}

enum {
    OPTION_HELP = 128,
    OPTION_LOG,
    OPTION_RANGE_END,
    OPTION_RANGE_START,
    OPTION_BASENAME,
    OPTION_RANDOM_LOCAL_ADDRESS
};

static const char* base_name = "localhost.nabto.net";
static int range_start = 0;
static int range_end = 1000;
static bool random_local_address = false;
static bool parse_args(int argc, char* argv[])
{
    const char x1s[] = "h";      const char* x1l[] = { "help", 0 };
    const char x21s[] = "l";     const char* x21l[] = { "nabtolog", 0 };
    const char x30s[] = "";      const char* x30l[] = { "rangestart", 0 };
    const char x31s[] = "";      const char* x31l[] = { "rangeend", 0 };
    const char x32s[] = "";      const char* x32l[] = { "basename", 0 };
    const char x33s[] = "";      const char* x33l[] = { "random-local-address", 0 };
    
    const char *option;

    const struct { int k; int f; const char *s; const char*const* l; } opts[] = {
        { OPTION_HELP, GOPT_NOARG,           x1s,  x1l  },
        { OPTION_LOG,  GOPT_REPEAT|GOPT_ARG, x21s, x21l },
        { OPTION_RANGE_START, GOPT_ARG, x30s, x30l},
        { OPTION_RANGE_END, GOPT_ARG, x31s, x31l},
        { OPTION_BASENAME, GOPT_ARG, x32s, x32l},
        { OPTION_RANDOM_LOCAL_ADDRESS, GOPT_NOARG, x33s, x33l },
        {0,0,0,0}
    };
    
    void *options = gopt_sort( & argc, (const char**)argv, opts);
    
    if (gopt(options, OPTION_HELP)) {
        help();
        exit(0);
    }
    
    if (gopt_arg( options, OPTION_BASENAME, &base_name)) {
        
    }
    
    if (gopt_arg(options, OPTION_RANGE_START, &option)) {
        range_start = atoi(option);
    }
    
    if (gopt_arg(options, OPTION_RANGE_END, &option)) {
        range_end = atoi(option);
    }
    
    { 
        const char* optionString;
        size_t optionsLength = gopt(options, OPTION_LOG);
        int i;
        if (optionsLength > 0) {
            for (i = 0; i < optionsLength; i++) {
                optionString = gopt_arg_i(options, OPTION_LOG, i);
                if (!unabto_log_system_enable_stdout_pattern(optionString)) {
                    NABTO_LOG_FATAL(("Logstring %s is not a valid logsetting", optionString));
                }
            }
        } else {
            unabto_log_system_enable_stdout_pattern("*.info");
        }
        
    }

    if (gopt(options, OPTION_RANDOM_LOCAL_ADDRESS)) {
        random_local_address = true;
    }
    
    return true;
}


int main(int argc, char** argv) {
    char dnsid[250];
    int i, count;
    pthread_t* nt;
    int val;
    struct timespec tv;

    if (!parse_args(argc, argv)) {
        help();
        exit(1);
    }
    
    sprintf(dnsid, "foo.%s", base_name);

    count = range_end - range_start;
    if (count <= 0) {
        help();
        printf("end_range < begin_range\n");
        exit(1);
    }
    nt = (pthread_t*) calloc(count, sizeof(pthread_t));
    if (nt == NULL) {
        NABTO_LOG_FATAL(("failed to allocate memory for array of threads"));
    }

    resolve_dns(dnsid);

    for (i = 0; i < count; i++) {
        char* name = (char*) malloc(20 + strlen(base_name));
        if (name == NULL) {
            NABTO_LOG_FATAL(("failed to allocate memory for thread %d (%d.%s)", i, range_start + i, base_name));
        }

        sprintf(name, "%d.%s", range_start + i, base_name);
        val = pthread_create(nt + i, NULL, main_routine, name);
        if (val != 0) {          
            NABTO_LOG_FATAL(("failed to create thread %d (%d.%s), val: %d, err: %d", i, range_start + i, base_name, val, errno));
        }
    }

    while(1) {
        getchar();
    }

}

void* main_routine(void* args) {

    nabto_main_setup* nms;
    const char* id = (const char*)args;

    unabto_epoll_init();
    
    nms = unabto_init_context();
    nms->id = id;
    nms->localPort = 0;
    nms->secureAttach = true;
    nms->secureData = true;
    nms->localPort = 0;
    nms->cryptoSuite = CRYPT_W_AES_CBC_HMAC_SHA256;
    nms->controllerArg.addr = bsip;
    if (random_local_address) {
        nms->ipAddress = 0x7f000001 + (rand() % 1000);
    }

    tunnel_loop_epoll();

    free(args);
    return 0;
    
}

application_event_result application_event(application_request* request, buffer_read_t* readBuffer, buffer_write_t* writeBuffer)
{
    return AER_REQ_INV_QUERY_ID;
}

bool application_poll_query(application_request** applicationRequest) {
    return false;
}

application_event_result application_poll(application_request* applicationRequest, buffer_read_t* readBuffer, buffer_write_t* writeBuffer) {
    return AER_REQ_SYSTEM_ERROR;
}

void application_poll_drop(application_request* applicationRequest) {
}

void resolve_dns(const char *id) {
    uint32_t addr = inet_addr(id);
    if (addr == INADDR_NONE) {
        // host isn't a dotted IP, so resolve it through DNS
        struct hostent* he = gethostbyname(id);
        if (he == 0) {

        } else {
            addr = *((uint32_t*)he->h_addr_list[0]);
        }
    }
    bsip = htonl(addr);
}

bool tunnel_allow_connection(const char* host, int port) {
    return true;
}
