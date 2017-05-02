#if defined(_MSC_VER)
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#if defined(WIN32) || defined(WINCE)
#include <winsock2.h>
#include <windows.h>
#endif

#define NABTO_TICK 5

#include <modules/tunnel/unabto_tunnel.h>
#include <modules/tunnel/unabto_tunnel_select.h>
#include <modules/tunnel/unabto_tunnel_epoll.h>
#include <modules/fingerprint_acl/fp_acl_ae.h>
#include <modules/fingerprint_acl/fp_acl_memory.h>
#include <modules/fingerprint_acl/fp_acl_file.h>

#include <unabto/unabto_app.h>

#include <unabto/unabto_common_main.h>
#include <unabto/unabto_attach.h>
#include <unabto_version.h>

#include <modules/cli/gopt/gopt.h>
#include <modules/diagnostics/unabto_diag.h>
#include <modules/util/read_hex.h>

#include <unabto/unabto_stream.h>

#include <modules/network/epoll/unabto_epoll.h>

#include "platform_checks.h"

#ifdef WINCE
#define HANDLE_SINGALS 0
#else
#define HANDLE_SIGNALS 1
#endif

#ifndef WINCE
#include <errno.h>
#endif

#if HANDLE_SIGNALS
#include <signal.h>
#endif

#if defined(_MSC_VER)
#define strdup(s) _strdup(s)
#endif

static uint16_t* ports = NULL;
static size_t ports_length = 0;
static char** hosts = NULL;
static size_t hosts_length = 0;
static bool allow_all_ports = false;
static bool allow_all_hosts = false;
static bool no_access_control = false;
static bool nice_exit = false;
#if HANDLE_SIGNALS && defined(WIN32)
static HANDLE signal_event = NULL;
#endif
static struct fp_acl_db fp_acl_db;
struct fp_mem_persistence fp_persistence_file;

#if NABTO_ENABLE_EPOLL
static bool useSelectBased = false;
#endif

static char* default_hosts[] = { "127.0.0.1", "localhost", 0 };

enum {
    ALLOW_PORT_OPTION = 1,
    ALLOW_HOST_OPTION,
    ALLOW_ALL_PORTS_OPTION,
    ALLOW_ALL_HOSTS_OPTION,
    NO_ACCESS_CONTROL_OPTION,
    DISABLE_TCP_FALLBACK_OPTION,
    CONTROLLER_PORT_OPTION,
    SELECT_BASED_OPTION,
    TUNNELS_OPTION,
    STREAM_WINDOW_SIZE_OPTION,
    CONNECTIONS_SIZE_OPTION,
    DISABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS
};

#if HANDLE_SIGNALS
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
            NABTO_LOG_INFO(("Cleanup done. Exiting"));
            exit(1);
        }
        NABTO_LOG_INFO(("Exit in signal handler: SIGINT. No --nice_exit argument -> no cleanup"));
        exit(1);
    }
#ifdef WIN32
    SetEvent(signal_event);
#endif
}
#endif

static bool tunnel_parse_args(int argc, char* argv[], nabto_main_setup* nms) {

    const char *tunnel_port_str;

    const char x1s[] = "h";      const char* x1l[] = { "help", 0 };
    const char x2s[] = "V";      const char* x2l[] = { "version", 0 };
    const char x3s[] = "C";      const char* x3l[] = { "config", 0 };
    const char x4s[] = "S";      const char* x4l[] = { "size", 0 };
    const char x9s[] = "d";      const char* x9l[] = { "device_name", 0 };
    const char x10s[] = "H";     const char* x10l[] = { "tunnel_default_host", 0 }; // only used together with the tunnel.
    const char x11s[] = "P";     const char* x11l[] = { "tunnel_default_port", 0 };
    const char x12s[] = "s";     const char* x12l[] = { "use_encryption", 0 };
    const char x13s[] = "k";     const char* x13l[] = { "encryption_key", 0 };
    const char x14s[] = "p";     const char* x14l[] = { "localport", 0 };
    const char x15s[] = "a";     const char* x15l[] = { "check_acl", 0 };
    const char x16s[] = "";      const char* x16l[] = { "allow_port", 0};
    const char x17s[] = "";      const char* x17l[] = { "allow_host", 0};
    const char x18s[] = "x";     const char* x18l[] = { "nice_exit", 0};
    const char x19s[] = "";      const char* x19l[] = { "allow_all_ports", 0};
    const char x21s[] = "l";     const char* x21l[] = { "nabtolog", 0 };
    const char x22s[] = "A";     const char* x22l[] = { "controller", 0 };
    const char x23s[] = "";      const char* x23l[] = { "disable_tcp_fb", 0 };
    const char x24s[] = "";      const char* x24l[] = { "controller_port", 0 };
#if NABTO_ENABLE_EPOLL
    const char x25s[] = "";      const char* x25l[] = { "select_based", 0 };
#endif

    const char x26s[] = "";      const char* x26l[] = { "tunnels", 0 };
    const char x27s[] = "";      const char* x27l[] = { "stream_window_size",0 };
    const char x28s[] = "";      const char* x28l[] = { "connections", 0 };
    const char x29s[] = "";      const char* x29l[] = { "disable_extended_rendezvous_multiple_sockets", 0 };
    const char x30s[] = "";      const char* x30l[] = { "allow_all_hosts", 0};
    const char x31s[] = "";      const char* x31l[] = { "no-access-control", 0};

    const struct { int k; int f; const char *s; const char*const* l; } opts[] = {
        { 'h', GOPT_NOARG,   x1s, x1l },
        { 'V', GOPT_NOARG,   x2s, x2l },
        { 'C', GOPT_NOARG,   x3s, x3l },
        { 'S', GOPT_NOARG,   x4s, x4l },
        { 'd', GOPT_ARG,     x9s, x9l },
        { 'H', GOPT_ARG,     x10s, x10l },
        { 'P', GOPT_ARG,     x11s, x11l },
        { 's', GOPT_NOARG,   x12s, x12l },
        { 'k', GOPT_ARG,     x13s, x13l },
        { 'p', GOPT_ARG,     x14s, x14l },
        { ALLOW_PORT_OPTION, GOPT_REPEAT|GOPT_ARG, x16s, x16l },
        { ALLOW_HOST_OPTION, GOPT_REPEAT|GOPT_ARG, x17s, x17l },
        { 'x', GOPT_NOARG, x18s, x18l },
        { ALLOW_ALL_PORTS_OPTION, GOPT_NOARG, x19s, x19l },
        { 'l', GOPT_REPEAT|GOPT_ARG,  x21s, x21l },
        { 'A', GOPT_ARG, x22s, x22l },
        { DISABLE_TCP_FALLBACK_OPTION, GOPT_NOARG, x23s, x23l },
        { CONTROLLER_PORT_OPTION, GOPT_ARG, x24s, x24l },
#if NABTO_ENABLE_EPOLL
        { SELECT_BASED_OPTION, GOPT_NOARG, x25s, x25l },
#endif
#if NABTO_ENABLE_DYNAMIC_MEMORY
        { TUNNELS_OPTION, GOPT_ARG, x26s, x26l },
        { STREAM_WINDOW_SIZE_OPTION, GOPT_ARG, x27s, x27l },
        { CONNECTIONS_SIZE_OPTION, GOPT_ARG, x28s, x28l },
#endif
#if NABTO_ENABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS
        { DISABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS, GOPT_NOARG, x29s, x29l },
#endif
        { ALLOW_ALL_HOSTS_OPTION, GOPT_NOARG, x30s, x30l },
        { NO_ACCESS_CONTROL_OPTION, GOPT_NOARG, x31s, x31l },
        { 0,0,0,0 }
    };

    void *options = gopt_sort( & argc, (const char**)argv, opts);

    const char * h;
    int p;
    const char* localPortStr;
    const char* optionString;
    const char* basestationAddress;
    const char* controllerPort;
    const char* tunnelsOption;
    const char* streamWindowSizeOption;
    const char* connectionsOption;
    uint32_t addr;


    if (gopt(options, 'h')) {
        printf("Usage: unabto_tunnel [options] -d devicename\n");
        printf("  -h, --help                  Print this help.\n");
        printf("  -V, --version               Print version.\n");
        printf("  -C, --config                Print configuration (unabto_config.h).\n");
        printf("  -S, --size                  Print size (in bytes) of memory usage.\n");
        printf("  -l, --nabtolog              Speficy log level such as *.trace.\n");
        printf("  -d, --device_name           Specify name of this device.\n");
        printf("  -H, --tunnel_default_host   Set default host name for tunnel (%s).\n", UNABTO_TUNNEL_TCP_DEFAULT_HOST);
        printf("  -P, --tunnel_default_port   Set default port for tunnel (%u).\n", UNABTO_TUNNEL_TCP_DEFAULT_PORT);
        printf("  -k, --encryption_key        Specify encryption key.\n");
        printf("  -p, --localport             Specify port for local connections.\n");
        printf("  -A, --controller            Specify controller address\n");
        printf("      --controller_port       sets the controller port number.\n");
        printf("      --allow_port            Ports that are allowed. \n");
        printf("      --allow_all_ports       Allow connections to all port numbers.\n");
        printf("      --allow_host            Hostnames that are allowed in addition to localhost \n");
        printf("      --allow_all_hosts       Allow connections to all TCP hosts.\n");
        printf("  -x, --nice_exit             Close the tunnels nicely when pressing Ctrl+C.\n");
#if NABTO_ENABLE_TCP_FALLBACK
        printf("      --disable_tcp_fb        Disable tcp fallback.\n");
#endif
#if NABTO_ENABLE_EPOLL
        printf("      --select_based          Use select instead of epoll.\n");
#endif
#if NABTO_ENABLE_DYNAMIC_MEMORY
        printf("      --tunnels               Specify how many concurrent tcp streams should be possible.\n");
        printf("      --stream_window_size    Specify the stream window size, the larger the value the more memory the aplication will use, but higher throughput will be possible.\n");
        printf("      --connections           Specify the maximum number of allowed concurrent connections.\n");
#endif
#if NABTO_ENABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS
        printf("      --disable_extended_rendezvous_multiple_sockets     Disable multiple sockets in extended rendezvous.\n");
#endif
        exit(0);
    }

    if (gopt(options, 'V')) {
        printf("Version: " PRIversion "\n", MAKE_VERSION_PRINTABLE());
        exit(0);
    }

    if (gopt(options, 'C')) {
        unabto_printf_unabto_config(stdout, "unabto_tunnel");
        exit(0);
    }

    if (gopt(options, 'S')) {
        unabto_printf_memory_sizes(stdout, "unabto_tunnel");
        exit(0);
    }
    
    { 
        size_t optionsLength = gopt(options, 'l');
        size_t i;
        if (optionsLength > 0) {
            for (i = 0; i < optionsLength; i++) {
                optionString = gopt_arg_i(options, 'l', i);
                if (!unabto_log_system_enable_stdout_pattern(optionString)) {
                    NABTO_LOG_FATAL(("Logstring %s is not a valid logsetting", optionString));
                }
            }
        } else {
            unabto_log_system_enable_stdout_pattern("*.info");
        }
        
    } 
    

    if (!gopt_arg( options, 'd', &nms->id)) {
        NABTO_LOG_FATAL(("Specify a serverId with -d. Try -h for help."));
    }

    if(gopt_arg(options, 'H', &h)) {
        unabto_tunnel_tcp_set_default_host(h);
    }

    if (gopt_arg(options, 'P', &tunnel_port_str)) {
        if(1 != sscanf(tunnel_port_str, "%d", &p)) {
            NABTO_LOG_FATAL(("Reading of port parameter failed."));
        } else {
            unabto_tunnel_tcp_set_default_port(p);
        }
    }
     
    if( gopt_arg( options, 'p', &localPortStr) ){
        int localPort = atoi(localPortStr);
        nms->localPort = localPort;
    }

    const char* preSharedKey;
    nms->cryptoSuite = CRYPT_W_AES_CBC_HMAC_SHA256;
    nms->secureAttach = true;
    nms->secureData = true;
    if ( gopt_arg( options, 'k', &preSharedKey)) {
        if (!unabto_read_psk_from_hex(preSharedKey, nms->presharedKey ,16)) {
            NABTO_LOG_FATAL(("Cannot read psk"));
        }
    } else {
        if (!gopt( options, 's' )) {
            NABTO_LOG_FATAL(("Specify a preshared key with -k. Try -h for help."));
        } else {
            // using zero key, undocumented but handy for testing
        }
    }

    if( gopt_arg( options, 'A', & basestationAddress ) ){
        addr = inet_addr(basestationAddress);
        if (addr == INADDR_NONE) {
            NABTO_LOG_FATAL(("Invalid basestation address"));
        }
        nms->controllerArg.addr = htonl(addr);
    }

    if (gopt(options, 'x')) {
        nice_exit = true;
    }

    ports_length = gopt(options, ALLOW_PORT_OPTION);
    if (ports_length > 0) {
        size_t i;
        ports = (uint16_t*) malloc(ports_length*sizeof(uint16_t));
        memset(ports,0,ports_length*sizeof(uint16_t));
        for (i = 0; i < ports_length; i++) {
            const char* opt = gopt_arg_i(options, ALLOW_PORT_OPTION, i);
            ports[i] = atoi(opt);
        }
    } else {
        NABTO_LOG_FATAL(("You did not allow client to connect to any ports, specify with '--allow_port'. Try -h for help."));
    }
    
    hosts_length = gopt(options, ALLOW_HOST_OPTION);
    if (hosts_length > 0) {
        size_t i;
        hosts = (char**) malloc(hosts_length*sizeof(char*));
        for (i = 0; i < hosts_length; i++) {
            const char* opt = gopt_arg_i(options, ALLOW_HOST_OPTION, i);
            hosts[i] = strdup(opt);
        }
    }

    if (gopt(options, ALLOW_ALL_PORTS_OPTION)) {
        allow_all_ports = true;
    }

    if (gopt(options, ALLOW_ALL_HOSTS_OPTION)) {
        allow_all_hosts = true;
    }

    if (gopt(options, NO_ACCESS_CONTROL_OPTION)) {
        no_access_control = true;
    }

#if NABTO_ENABLE_TCP_FALLBACK
    if (gopt(options, DISABLE_TCP_FALLBACK_OPTION)) {
        nms->enableTcpFallback = false;
    }
#endif

    if (gopt_arg(options, CONTROLLER_PORT_OPTION, &controllerPort)) {
        nms->controllerArg.port = atoi(controllerPort);
    }

#if NABTO_ENABLE_EPOLL
    if (gopt(options, SELECT_BASED_OPTION)) {
        useSelectBased = true;
    }
#endif

#if NABTO_ENABLE_DYNAMIC_MEMORY
    if (gopt_arg(options, TUNNELS_OPTION, &tunnelsOption)) {
        nms->streamMaxStreams = atoi(tunnelsOption);
    }

    if (gopt_arg(options, STREAM_WINDOW_SIZE_OPTION, &streamWindowSizeOption)) {
        uint16_t windowSize = atoi(streamWindowSizeOption);
        nms->streamReceiveWindowSize = windowSize;
        nms->streamSendWindowSize = windowSize;
    }

    if (gopt_arg(options, CONNECTIONS_SIZE_OPTION, &connectionsOption)) {
        nms->connectionsSize = atoi(connectionsOption);
    }
#endif

#if NABTO_ENABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS
    if (gopt(options, DISABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS)) {
        nms->enableExtendedRendezvousMultipleSockets = false;
    } else {
#if NABTO_ENABLE_DYNAMIC_MEMORY
        int desiredFds = NABTO_EXTENDED_RENDEZVOUS_MAX_SOCKETS + nms->connectionsSize + nms->streamMaxStreams + 50/*arbitrary number*/;
#else
        int desiredFds = NABTO_EXTENDED_RENDEZVOUS_MAX_SOCKETS + NABTO_CONNECTIONS_SIZE + NABTO_STREAM_MAX_STREAMS + 50/*arbitrary number*/;
#endif
        int actualFds = check_ulimit_files(desiredFds);
        if (actualFds == -1) {

        } else if (actualFds < desiredFds) {
            NABTO_LOG_ERROR(("Disabling extended rendezvous with multiple sockets since the platform cannot give the required number of file descriptors"));
            nms->enableExtendedRendezvousMultipleSockets = false;
        }
    }
#endif

    return true;
}

void acl_init() {
    NABTO_LOG_WARN(("Please review default access permissions and just remove this warning if acceptable"));
    struct fp_acl_settings default_settings;

    // master switch: system allows both local and remote access to
    // users with the right privileges and is open for pairing with new users 
    default_settings.systemPermissions =
        FP_ACL_SYSTEM_PERMISSION_PAIRING |
        FP_ACL_SYSTEM_PERMISSION_LOCAL_ACCESS |
        FP_ACL_SYSTEM_PERMISSION_REMOTE_ACCESS;

    // first user is granted admin permission and local+remote access
    default_settings.firstUserPermissions =
        FP_ACL_PERMISSION_ADMIN |
        FP_ACL_PERMISSION_LOCAL_ACCESS |
        FP_ACL_PERMISSION_REMOTE_ACCESS;

    // subsequent users will just have guest privileges but still have local+remote access
    default_settings.defaultUserPermissions =
        FP_ACL_PERMISSION_LOCAL_ACCESS |
        FP_ACL_PERMISSION_REMOTE_ACCESS;

    if (fp_acl_file_init("persistence.bin", "tmp.bin", &fp_persistence_file) != FP_ACL_DB_OK) {
        NABTO_LOG_ERROR(("cannot load acl file"));
        exit(1);
    }
    fp_mem_init(&fp_acl_db, &default_settings, &fp_persistence_file);
    fp_acl_ae_init(&fp_acl_db);
}


int main(int argc, char** argv)
{
    nabto_main_setup* nms = unabto_init_context();

    platform_checks();
    acl_init();
    
#if NABTO_ENABLE_EPOLL
    unabto_epoll_init();
#endif

    if (!tunnel_parse_args(argc, argv, nms)) {
        NABTO_LOG_FATAL(("failed to parse commandline args"));
    }

#if HANDLE_SIGNALS
#ifdef WIN32
    signal_event = CreateEvent(NULL, FALSE, FALSE, NULL);
#endif
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);
#endif

#if NABTO_ENABLE_EPOLL
    if (useSelectBased) {
        tunnel_loop_select();
    } else {
        tunnel_loop_epoll();
    }
#else
    tunnel_loop_select();
#endif
    return 0;
}

bool allow_client_access(nabto_connect* connection) {
    if (no_access_control) {
        return true;
    } else {
        bool local = connection->isLocal;
        bool allow = fp_acl_is_connection_allowed(connection) || local;
        NABTO_LOG_INFO(("Allowing %s connect request: %s", (local ? "local" : "remote"), (allow ? "yes" : "no")));
        return allow;
    }
}

bool is_default_host(const char* host) {
    char** p = default_hosts;
    while (*p) {
        if (strcmp(*p++, host) == 0) {
            return true;
        }
    }
    return false;
}

bool tunnel_allow_connection(const char* host, int port) {
    size_t i;
    
    bool allow;
    bool portFound = false;
    bool hostFound = false;
    
    if (allow_all_ports) {
        portFound = true;
    } else {
        for (i = 0; i < ports_length; i++) {
            if (ports[i] == port) {
                portFound = true;
            }
        }
    }

    if (allow_all_hosts || is_default_host(host)) {
        hostFound = true;
    } else {
        for(i = 0; i < hosts_length; i++) {
            if (strcmp(host, hosts[i]) == 0) {
                hostFound = true;
            }
        }
    }
    
    allow = hostFound && portFound;
    
    if (!allow) {
        NABTO_LOG_INFO(("Current host/port access config has disallowed access to %s:%i", host, port));
    }

    return allow;
}


int write_string(unabto_query_response* write_buffer, const char* string) {
    return unabto_query_write_uint8_list(write_buffer, (uint8_t *)string, strlen(string));
}

application_event_result application_event(application_request* request,
                                           unabto_query_request* query_request,
                                           unabto_query_response* query_response)
{
    NABTO_LOG_INFO(("Nabto application_event: %u", request->queryId));

    // handle requests as defined in interface definition shared with
    // client - for the default demo, see
    // https://github.com/nabto/ionic-starter-nabto/blob/master/www/nabto/unabto_queries.xml

    application_event_result res;

    switch (request->queryId) {
    case 10000:
        // get_public_device_info.json
        if (!write_string(query_response, "Tunnel")) return AER_REQ_RSP_TOO_LARGE;
        if (!write_string(query_response, "Tunnel")) return AER_REQ_RSP_TOO_LARGE;
        if (!write_string(query_response, "")) return AER_REQ_RSP_TOO_LARGE;
        if (!unabto_query_write_uint8(query_response, fp_acl_is_pair_allowed(request))) return AER_REQ_RSP_TOO_LARGE;
        if (!unabto_query_write_uint8(query_response, fp_acl_is_user_paired(request))) return AER_REQ_RSP_TOO_LARGE; 
        if (!unabto_query_write_uint8(query_response, fp_acl_is_user_owner(request))) return AER_REQ_RSP_TOO_LARGE;
        return AER_REQ_RESPONSE_READY;

    case 11000:
        // get_users.json
        return fp_acl_ae_users_get(request, query_request, query_response); // implied admin priv check
        
    case 11010: 
        // pair_with_device.json
        if (!fp_acl_is_pair_allowed(request)) return AER_REQ_NO_ACCESS;
        res = fp_acl_ae_pair_with_device(request, query_request, query_response); 
        return res;

    case 11020:
        // get_current_user.json
        return fp_acl_ae_user_me(request, query_request, query_response); 

    case 11030:
        // get_system_security_settings.json
        return fp_acl_ae_system_get_acl_settings(request, query_request, query_response); // implied admin priv check

    case 11040:
        // set_system_security_settings.json
        return fp_acl_ae_system_set_acl_settings(request, query_request, query_response); // implied admin priv check

    case 11050:
        // set_user_permissions.json
        return fp_acl_ae_user_set_permissions(request, query_request, query_response); // implied admin priv check

    case 11060:
        // set_user_name.json
        return fp_acl_ae_user_set_name(request, query_request, query_response); // implied admin priv check

    case 11070:
        // remove_user.json
        return fp_acl_ae_user_remove(request, query_request, query_response); // implied admin priv check

    default:
        NABTO_LOG_WARN(("Unhandled query id: %u", request->queryId));
        return AER_REQ_INV_QUERY_ID;
    }
}

bool application_poll_query(application_request** applicationRequest) {
    return false;
}

application_event_result application_poll(application_request* applicationRequest, unabto_query_request* readBuffer, unabto_query_response* writeBuffer) {
    return AER_REQ_SYSTEM_ERROR;
}

void application_poll_drop(application_request* applicationRequest) {
}
