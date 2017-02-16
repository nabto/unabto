#ifdef WIN32
#include <winsock2.h>
#else
#include <pthread.h>
#endif
#include <unabto/unabto_env_base.h>
#include <unabto/unabto_logging.h>
#if NABTO_ENABLE_CONNECTION_ESTABLISHMENT_ACL_CHECK || NABTO_ENABLE_EVENTCHANNEL
#include <unabto/unabto_app.h>
#endif

#define NABTO_TICK 5

#include <unabto/unabto_common_main.h>
#include <unabto/unabto_attach.h>
#include <unabto/unabto_memory.h>

#include <modules/cli/gopt/gopt.h>
#include <modules/diagnostics/unabto_diag.h>

#include <unabto/unabto_stream.h>
#include <unabto/unabto_app_adapter.h>

#include <unabto/unabto_common_main.h>
#include <unabto_version.h>
#include <unabto/unabto.h>

#include <errno.h>
#include <signal.h>

uint16_t* ports = NULL;
size_t ports_length = 0;
char** hosts = NULL;
size_t hosts_length = 0;
bool nice_exit = false;

void handle_signal(int signum) {
    if (signum == SIGTERM) {
        // Terminate application with exit,
        // This makes libprofiler extra happy
        NABTO_LOG_INFO(("Exit in signal handler: SIGTERM"));
        exit(1);
    }
    if (signum == SIGINT) {
        // Terminate application with exit,
        // This makes libprofiler extra happy
        if (nice_exit) {
            NABTO_LOG_INFO(("Exit in signal handler: SIGINT."));
            NABTO_LOG_INFO(("Cleanup nicely (since --nice_exit was specified)"));
            unabto_close();
            NABTO_LOG_INFO(("Cleanup done. Exiting"));
            exit(1);
        }
        NABTO_LOG_INFO(("Exit in signal handler: SIGINT. No --nice_exit argument -> no cleanup"));
        exit(1);
    }
}

static bool test_parse_args(const char * progname, int argc, char* argv[], nabto_main_setup* nms)
{
    const char *x1[] = { "help", 0 };
    const char *x2[] = { "version", 0 };
    const char *x3[] = { "config", 0 };
    const char *x4[] = { "size", 0 };
    const char *x5[] = { "deviceName", 0 };
    const char *x6[] = { "use_encryption", 0 };
    const char *x7[] = { "encryption_key", 0 };
    const char *x8[] = { "localport", 0 };
    const char *x11[] = { "nice_exit", 0};

    const struct { int k; int f; const char *s; const char*const* l; } opts[] = {
        { 'h', GOPT_NOARG,   "h?", x1 },
        { 'V', GOPT_NOARG,   "V",  x2 },
        { 'C', GOPT_NOARG,   "C",  x3 },
        { 'S', GOPT_NOARG,   "S",  x4 },
        { 'd', GOPT_ARG,     "d",  x5 },
        { 's', GOPT_NOARG,   "s",  x6 },
        { 'k', GOPT_ARG,     "k",  x7 },
        { 'p', GOPT_ARG,     "p",  x8 },
        { 'x', GOPT_NOARG,   "x",  x11 },
        { 0,0,0,0 }
    };

    void *options = gopt_sort(&argc, (const char**)argv, opts);

    if (gopt(options, 'h')) {
        printf("Usage: %s [options]\n", progname);
        printf("   -h  Print this help.\n");
        printf("   -V  Print release version.\n");
        printf("   -C  Print configuration (unabto_config.h).\n");
        printf("   -S  Print size (in bytes) of structures (memory usage).\n");
        exit(0);
    }
    
    if (gopt(options, 'V')) {
        printf("%s: " PRIversion "\n", progname, MAKE_VERSION_PRINTABLE());
        exit(0);
    }
    
    if (gopt(options, 'C')) {
        unabto_printf_unabto_config(stdout, progname);
        exit(0);
    }
    
    if (gopt(options, 'S')) {
        unabto_printf_memory_sizes(stdout, progname);
        exit(0);
    }

    if (!gopt_arg(options, 'd', &nms->id)) {
        NABTO_LOG_FATAL(("Specify a serverId with -d"));
    }
    
    if (gopt(options, 's')) {
        const char* preSharedKey;
        if ( gopt_arg( options, 'k', &preSharedKey)) {
            if (!unabto_read_psk_from_hex(preSharedKey, nms->presharedKey, 16)) {
                NABTO_LOG_FATAL(("Cannot read presharedkey");
            }
        }
#if NABTO_ENABLE_CONNECTIONS
        nms->cryptoSuite = CRYPT_W_AES_CBC_HMAC_SHA256;
#endif
    }

    if (gopt(options, 'x')) {
        nice_exit = true;
    }

    return true;
}


#if NABTO_ENABLE_CONNECTION_ESTABLISHMENT_ACL_CHECK
bool allow_client_access(nabto_connect* con)
{
    return true;
}
#endif

#if NABTO_ENABLE_EVENTCHANNEL
unabto_buffer* get_event_buffer2(size_t maxSize)
{
    return NULL;
}
#endif

int main(int argc, char** argv)
{
    const char *progname;

    progname = strrchr(argv[0], '/');
    if (!progname)
        progname = argv[0];
    else
        progname++;

    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);
    
    nabto_main_setup* nms = unabto_init_context();
    
    if (!test_parse_args(progname, argc, argv, nms)) {
        NABTO_LOG_FATAL(("failed to parse commandline args"));
    }

    unabto_init();

    NABTO_LOG_TRACE(("Started\n"));

    while(true) {
        unabto_tick();
    }

    NABTO_LOG_TRACE(("We should never be here"));
    return 0;
}

void setTimeFromGSP(uint32_t stamp)
{
    NABTO_NOT_USED(stamp);
}

application_event_result application_event(application_request* applicationRequest, uanbto_query_request* readBuffer, unabto_query_response* writeBuffer) {
    return AER_REQ_NO_QUERY_ID;
}

#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
bool application_poll_query(application_request** applicationRequest) {
    return false;
}

application_event_result application_poll(application_request* applicationRequest, unabto_query_request* readBuffer, unabto_query_response* writeBuffer) {
    return AER_REQ_SYSTEM_ERROR;
}

void application_poll_drop(application_request* applicationRequest) {}
#endif

#if NABTO_ENABLE_STREAM
void unabto_stream_accept(unabto_stream* stream) {
    unabto_stream_release(stream);
}
#endif

#if NABTO_ENABLE_STATUS_CALLBACKS
void unabto_attach_state_changed(nabto_state state) {
    // Not handled
}
#endif


#if NABTO_ENABLE_STREAM && NABTO_ENABLE_STREAM_EVENTS
void unabto_stream_event(unabto_stream* stream, unabto_stream_event_type event) {

}
#endif
