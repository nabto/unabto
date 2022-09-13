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
#include <modules/tunnel/unabto_tunnel_common.h>
#include <modules/tunnel/unabto_tunnel_select.h>
#include <modules/tunnel/unabto_tunnel_epoll.h>
#include <modules/fingerprint_acl/fp_acl_ae.h>
#include <modules/fingerprint_acl/fp_acl_memory.h>
#include <modules/fingerprint_acl/fp_acl_file.h>

#include <unabto/unabto_app.h>

#include <unabto/unabto_common_main.h>
#include <unabto/unabto_attach.h>
#include <unabto_version.h>
#include <unabto/unabto_stream.h>

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

#if NABTO_ENABLE_EPOLL
static bool useSelectBased = false;
#endif

static char* default_hosts[] = { "127.0.0.1", "localhost", 0 };

uint8_t obscurity_key_id[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t obscurity_key[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// AMP video client
#define AMP_MAX_NAME_LENGTH 50
#define AMP_DEVICE_NAME_DEFAULT "Tunnel"
#define AMP_PRODUCT_NAME_DEFAULT "uNabto Video"
static char device_name[AMP_MAX_NAME_LENGTH];
static char product_name[AMP_MAX_NAME_LENGTH];
static const char* device_icon = "chip.png";
static const char* device_interface_id_ = "8eee78e7-8f22-4019-8cee-4dcbc1c8186c";
static uint16_t device_interface_version_major_ = 1;
static uint16_t device_interface_version_minor_ = 0;

// PPKA (RSA fingerprint) access control
static struct fp_acl_db fp_acl_db;
struct fp_mem_persistence fp_persistence_file;
#define REQUIRES_GUEST FP_ACL_PERMISSION_NONE
#define REQUIRES_OWNER FP_ACL_PERMISSION_ADMIN

enum {
    ALLOW_PORT_OPTION = 1,
    ALLOW_HOST_OPTION,
    ALLOW_ALL_PORTS_OPTION,
    ALLOW_ALL_HOSTS_OPTION,
    NO_ACCESS_CONTROL_OPTION,
    DISABLE_TCP_FALLBACK_OPTION,
    DISABLE_CRYPTO_OPTION,
    CONTROLLER_PORT_OPTION,
    SELECT_BASED_OPTION,
    TUNNELS_OPTION,
    STREAM_WINDOW_RECV_SIZE_OPTION,
    STREAM_WINDOW_SEND_SIZE_OPTION,
    CONNECTIONS_SIZE_OPTION,
    DISABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS,
    DISABLE_EXTENDED_RENDEZVOUS_MULTIPLE_PORTS,
    UART_DEVICE_OPTION,
    STREAM_SEGMENT_POOL_SIZE_OPTION
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
    const char *display_name_str;

    const char x1s[] = "h";      const char* x1l[] = { "help", 0 };
    const char x2s[] = "V";      const char* x2l[] = { "version", 0 };
    const char x3s[] = "C";      const char* x3l[] = { "config", 0 };
    const char x4s[] = "S";      const char* x4l[] = { "size", 0 };
    const char x9s[] = "d";      const char* x9l[] = { "device-name", 0 };
    const char x10s[] = "H";     const char* x10l[] = { "tunnel-default-host", 0 }; // only used together with the tunnel.
    const char x11s[] = "P";     const char* x11l[] = { "tunnel-default-port", 0 };
    const char x12s[] = "s";     const char* x12l[] = { "dummy-key", 0 };
    const char x13s[] = "k";     const char* x13l[] = { "encryption-key", 0 };
    const char x14s[] = "p";     const char* x14l[] = { "localport", 0 };
    const char x16s[] = "";      const char* x16l[] = { "allow-port", 0};
    const char x17s[] = "";      const char* x17l[] = { "allow-host", 0};
    const char x18s[] = "x";     const char* x18l[] = { "nice-exit", 0};
    const char x19s[] = "";      const char* x19l[] = { "allow-all-ports", 0};
    const char x21s[] = "l";     const char* x21l[] = { "nabtolog", 0 };
    const char x22s[] = "A";     const char* x22l[] = { "controller", 0 };
    const char x23s[] = "";      const char* x23l[] = { "disable-tcp-fb", 0 };
    const char x24s[] = "";      const char* x24l[] = { "controller-port", 0 };
#if NABTO_ENABLE_EPOLL
    const char x25s[] = "";      const char* x25l[] = { "select-based", 0 };
#endif

    const char x26s[] = "";      const char* x26l[] = { "tunnels", 0 };
    const char x27s[] = "";      const char* x27l[] = { "stream-recv-window-size", 0 };
    const char x28s[] = "";      const char* x28l[] = { "connections", 0 };
    const char x29s[] = "";      const char* x29l[] = { "disable-extended-rendezvous-multiple-sockets", 0 };
    const char x29_1s[] = "";    const char* x29_1l[] = { "disable-extended-rendezvous-multiple-ports", 0 };
    const char x30s[] = "";      const char* x30l[] = { "allow-all-hosts", 0};
    const char x31s[] = "";      const char* x31l[] = { "no-access-control", 0};
    const char x32s[] = "";      const char* x32l[] = { "no-crypto", 0};
    const char x33s[] = "";      const char* x33l[] = { "uart-device", 0 };
    const char x34s[] = "";      const char* x34l[] = { "stream-segment-pool-size", 0 };
    const char x35s[] = "";      const char* x35l[] = { "stream-send-window-size", 0 };
    const char x36s[] = "N";     const char* x36l[] = { "display-name", 0 };
    const char x37s[] = "R";     const char* x37l[] = { "product-name", 0 };
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
        { DISABLE_CRYPTO_OPTION, GOPT_NOARG, x32s, x32l },
        { CONTROLLER_PORT_OPTION, GOPT_ARG, x24s, x24l },
#if NABTO_ENABLE_EPOLL
        { SELECT_BASED_OPTION, GOPT_NOARG, x25s, x25l },
#endif
#if NABTO_ENABLE_DYNAMIC_MEMORY
        { TUNNELS_OPTION, GOPT_ARG, x26s, x26l },
        { STREAM_WINDOW_RECV_SIZE_OPTION, GOPT_ARG, x27s, x27l },
        { STREAM_WINDOW_SEND_SIZE_OPTION, GOPT_ARG, x35s, x35l },
        { CONNECTIONS_SIZE_OPTION, GOPT_ARG, x28s, x28l },
        { STREAM_SEGMENT_POOL_SIZE_OPTION, GOPT_ARG, x34s, x34l },
#endif
#if NABTO_ENABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS
        { DISABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS, GOPT_NOARG, x29s, x29l },
#endif
#if NABTO_ENABLE_EXTENDED_RENDEZVOUS_MULTIPLE_PORTS
        { DISABLE_EXTENDED_RENDEZVOUS_MULTIPLE_PORTS, GOPT_NOARG, x29_1s, x29_1l },
#endif

        { ALLOW_ALL_HOSTS_OPTION, GOPT_NOARG, x30s, x30l },
        { NO_ACCESS_CONTROL_OPTION, GOPT_NOARG, x31s, x31l },
        { UART_DEVICE_OPTION, GOPT_ARG, x33s, x33l },
        { 'N', GOPT_ARG, x36s, x36l },
        { 'R', GOPT_ARG, x37s, x37l },
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
    const char* streamWindowRecvSizeOption;
    const char* streamWindowSendSizeOption;
    const char* streamSegmentPoolSizeOption;
    const char* connectionsOption;
    const char* uartDevice = 0;
    const char* preSharedKey;
    bool fatalPortError = true;
    uint32_t addr;


    if (gopt(options, 'h')) {
        printf("Usage: unabto_tunnel [options] -d devicename\n");
        printf("  -h, --help                  Print this help.\n");
        printf("  -V, --version               Print version.\n");
        printf("  -C, --config                Print configuration (unabto_config.h).\n");
        printf("  -S, --size                  Print size (in bytes) of memory usage.\n");
        printf("  -l, --nabtolog              Speficy log level such as *.trace.\n");
        printf("  -d, --device-name           Specify Nabto device id of this device.\n");
        printf("  -N, --display-name          Specify display name of this device (retrieved by clients with RPC).\n");
        printf("  -R, --product-name          Specify display product name of this device (retrieved by clients with RPC).\n");
        printf("  -H, --tunnel-default-host   Set default host name for tunnel (%s).\n", UNABTO_TUNNEL_TCP_DEFAULT_HOST);
        printf("  -P, --tunnel-default-port   Set default port for tunnel (%u).\n", UNABTO_TUNNEL_TCP_DEFAULT_PORT);
        printf("  -k, --encryption-key        Specify encryption key.\n");
        printf("  -s, --dummy-key             Use dummy key (for testing only).\n");
        printf("      --no-crypto             Disable crypto (requires special basestation and client).\n");
        printf("  -p, --localport             Specify port for local connections.\n");
        printf("  -A, --controller            Specify controller address\n");
        printf("      --controller-port       sets the controller port number.\n");
        printf("      --allow-port            Ports that tunnel is allowed to connect to. \n");
        printf("      --allow-all-ports       Allow connections to all port numbers.\n");
        printf("      --allow-host            Hostnames that tunnel is allowed to connect to in addition to localhost \n");
        printf("      --allow-all-hosts       Allow connections to all TCP hosts.\n");
        printf("      --no-access-control     Do not enforce client access control on incoming connections. \n");
        if(unabto_tunnel_has_uart()){
            printf("      --uart-device           Sets the uart device\n");
        }
        printf("  -x, --nice-exit             Close the tunnels nicely when pressing Ctrl+C.\n");
#if NABTO_ENABLE_TCP_FALLBACK
        printf("      --disable-tcp-fb        Disable tcp fallback.\n");
#endif
#if NABTO_ENABLE_EPOLL
        printf("      --select-based          Use select instead of epoll.\n");
#endif
#if NABTO_ENABLE_DYNAMIC_MEMORY
        printf("      --tunnels               Specify how many concurrent tcp streams should be possible.\n");
        printf("      --stream-recv-window-size    Specify the stream recv window size, the larger the value the more memory the aplication will use, but higher throughput will be possible.\n");
        printf("      --stream-send-window-size    Specify the stream send window size, the larger the value the more memory the aplication will use, but higher throughput will be possible. The memory required can be limited by the send segment pool size option.\n");
        printf("      --connections           Specify the maximum number of allowed concurrent connections.\n");
        printf("      --stream-segment-pool-size  Specify the size of the pool for streaming send segments.\n");
#endif
#if NABTO_ENABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS
        printf("      --disable-extended-rendezvous-multiple-sockets     Disable multiple sockets in extended rendezvous.\n");
#endif
#if NABTO_ENABLE_EXTENDED_RENDEZVOUS_MULTIPLE_PORTS
        printf("      --disable-extended-rendezvous-multiple-ports       Disable multiple ports in extended rendezvous.\n");
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

    if (gopt_arg( options, 'N', &display_name_str)) {
        snprintf(device_name, sizeof(device_name), display_name_str);
    } else {
        snprintf(device_name, sizeof(device_name), AMP_DEVICE_NAME_DEFAULT);
    }

    if (gopt_arg( options, 'R', &display_name_str)) {
        snprintf(product_name, sizeof(product_name), display_name_str);
    } else {
        snprintf(product_name, sizeof(product_name), AMP_PRODUCT_NAME_DEFAULT);
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

    if (gopt(options, DISABLE_CRYPTO_OPTION)) {
        unabto_set_no_crypto(nms);
    } else {
        uint8_t psk[PRE_SHARED_KEY_SIZE];
        if ( gopt_arg( options, 'k', &preSharedKey)) {
            if (!unabto_read_psk_from_hex(preSharedKey, psk ,PRE_SHARED_KEY_SIZE)) {
                NABTO_LOG_FATAL(("Cannot read psk"));
            }
            if(!unabto_set_aes_crypto(nms, psk, PRE_SHARED_KEY_SIZE)){
                NABTO_LOG_FATAL(("init_nms_crypto failed"));
            }
        } else {
            if (!gopt( options, 's' )) {
                NABTO_LOG_FATAL(("Specify a preshared key with -k. Try -h for help."));
            } else {
                // using zero key, undocumented but handy for testing
                memset(psk,0,PRE_SHARED_KEY_SIZE);
                if(!unabto_set_aes_crypto(nms, psk, PRE_SHARED_KEY_SIZE)){
                    NABTO_LOG_FATAL(("init_nms_crypto failed"));
                }
            }
        }
    }

    if( gopt_arg( options, 'A', & basestationAddress ) ){
        addr = inet_addr(basestationAddress);
        if (addr == INADDR_NONE) {
            NABTO_LOG_FATAL(("Invalid basestation address"));
        }
        nms->controllerArg.addr.type = NABTO_IP_V4;
        nms->controllerArg.addr.addr.ipv4 = htonl(addr);
    }

    if (gopt(options, 'x')) {
        nice_exit = true;
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
        fatalPortError = false;
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
        fatalPortError = false;
    }

    if (gopt(options, ALLOW_ALL_HOSTS_OPTION)) {
        allow_all_hosts = true;
    }

    if (gopt(options, NO_ACCESS_CONTROL_OPTION)) {
        no_access_control = true;
    }

    if(unabto_tunnel_has_uart()){
        if(gopt_arg(options, UART_DEVICE_OPTION, &uartDevice)) {
            uart_tunnel_set_default_device(uartDevice);
            fatalPortError = false;
        } else {
            NABTO_LOG_TRACE(("UART tunnel enabled, but no UART device was given use --uart_device to specify a device."));
            uartDevice = 0;
        }
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

    if (gopt_arg(options, STREAM_WINDOW_SEND_SIZE_OPTION, &streamWindowSendSizeOption)) {
        uint16_t windowSize = atoi(streamWindowSendSizeOption);
        nms->streamSendWindowSize = windowSize;
    }

    if (gopt_arg(options, STREAM_WINDOW_RECV_SIZE_OPTION, &streamWindowRecvSizeOption)) {
        uint16_t windowSize = atoi(streamWindowRecvSizeOption);
        nms->streamReceiveWindowSize = windowSize;
    }

    if (gopt_arg(options, CONNECTIONS_SIZE_OPTION, &connectionsOption)) {
        nms->connectionsSize = atoi(connectionsOption);
    }

    if (gopt_arg(options, STREAM_SEGMENT_POOL_SIZE_OPTION, &streamSegmentPoolSizeOption)) {
        uint16_t poolSize = atoi(streamSegmentPoolSizeOption);
        nms->streamSegmentPoolSize = poolSize;
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
#if NABTO_ENABLE_EXTENDED_RENDEZVOUS_MULTIPLE_PORTS
    if (gopt(options, DISABLE_EXTENDED_RENDEZVOUS_MULTIPLE_PORTS)) {
        nms->enableExtendedRendezvousMultiplePorts = false;
    }
#endif
    if(fatalPortError == true) {
        NABTO_LOG_FATAL(("You did not allow client to connect to anything. For TCP tunnels please specify ports with '--allow-port' or '--allow-all-ports. For UART tunnel please specify a UART device with '--uart_device'. Try -h for help."));
    }
    return true;
}

void debug_dump_acl() {
    void* it = fp_acl_db.first();
    if (!it) {
        NABTO_LOG_DEBUG(("ACL is empty (no paired users)"));
    } else {
        NABTO_LOG_DEBUG(("ACL entries:"));
        while (it != NULL) {
            struct fp_acl_user user;
            fp_acl_db_status res = fp_acl_db.load(it, &user);
            if (res != FP_ACL_DB_OK) {
                NABTO_LOG_DEBUG(("ACL error %d\n", res));
                return;
            }
            if (user.fp.hasValue) {
                NABTO_LOG_DEBUG((" - %s [%02x:%02x:%02x:%02x:...]: %04x",
                                user.name,
                                user.fp.value.data[0], user.fp.value.data[1], user.fp.value.data[2], user.fp.value.data[3],
                                user.permissions));
            }
            it = fp_acl_db.next(it);
        }
    }
}

void acl_init() {
    struct fp_acl_settings default_settings;
    NABTO_LOG_WARN(("Please review default access permissions and just remove this warning if acceptable"));

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
        NABTO_LOG_ERROR(("cannot prepare acl file"));
        exit(1);
    }
    if (fp_mem_init(&fp_acl_db, &default_settings, &fp_persistence_file) != FP_ACL_DB_OK) {
        NABTO_LOG_FATAL(("Could not read persistence database file"));
    }
    fp_acl_ae_init(&fp_acl_db);
}


void educate_user() {
    printf("================================================================================\n");
    printf("IMPORTANT INFORMATION ABOUT SECURITY BEFORE USING IN PRODUCTION\n");
    printf("\n");
    printf("It is of utmost importance that you understand how security works in Nabto,\n");
    printf("please carefully read TEN036 \"Security in Nabto Solutions\" available from\n");
    printf("https://www.nabto.com/downloads.html.\n");
    printf("\n");
    printf("You must choose a client authorization model for use in your project to restrict\n");
    printf("access to this device, the different possible models are outlined in section 8\n");
    printf("of TEN036. Please contact support@nabto.com if you have any questions in this regard.");
    printf("\n\n");
    printf("PERFORMANCE\n");
    printf("\n");
    printf("Please see TEN030 \"Nabto Tunnels\" for information on how to optimize performance.\n");
    printf("If you observe performance problems, consult the FAQ section in TEN030 and follow\n");
    printf("the suggestions there before contacting support.\n");
    printf("================================================================================\n\n");
}

int main(int argc, char** argv)
{
    nabto_main_setup* nms = unabto_init_context();
    educate_user();
    platform_checks();
    acl_init();
    snprintf(device_name, sizeof(device_name), AMP_DEVICE_NAME_DEFAULT);

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
        bool allow = fp_acl_is_connection_allowed(connection);
        NABTO_LOG_INFO(("Allowing %s connect request: %s", (connection->isLocal ? "local" : "remote"), (allow ? "yes" : "no")));
        debug_dump_acl();
        return allow;
    }
}

bool unabto_tunnel_allow_client_access(nabto_connect* connection) {
    if (no_access_control) {
        return true;
    } else {
        bool allow = fp_acl_is_tunnel_allowed(connection, FP_ACL_PERMISSION_NONE);
        NABTO_LOG_INFO(("Allowing %s tunnel open request: %s", (connection->isLocal ? "local" : "remote"), (allow ? "yes" : "no")));
        debug_dump_acl();
        return allow;
    }
}

#if NABTO_ENABLE_TUNNEL_STATUS_CALLBACKS
void unabto_tunnel_status_callback(tunnel_status_event event, tunnel* tunnel) {
    tunnel_status_tcp_details info;
    unabto_tunnel_status_get_tcp_info(tunnel, &info);
    NABTO_LOG_INFO(("Tunnel event [%d] on tunnel [%d] to host [%s] on port [%d], bytes sent: [%d]", event, tunnel->tunnelId,
                    info.host, info.port, info.sentBytes));
}
#endif

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

int copy_buffer(unabto_query_request* read_buffer, uint8_t* dest, uint16_t bufSize, uint16_t* len) {
    uint8_t* buffer;
    if (!(unabto_query_read_uint8_list(read_buffer, &buffer, len))) {
        return AER_REQ_TOO_SMALL;
    }
    if (*len > bufSize) {
        return AER_REQ_TOO_LARGE;
    }
    memcpy(dest, buffer, *len);
    return AER_REQ_RESPONSE_READY;
}

int copy_string(unabto_query_request* read_buffer, char* dest, uint16_t destSize) {
    uint16_t len;
    int res = copy_buffer(read_buffer, (uint8_t*)dest, destSize-1, &len);
    if (res != AER_REQ_RESPONSE_READY) {
        return res;
    }
    dest[len] = 0;
    return AER_REQ_RESPONSE_READY;
}

int write_string(unabto_query_response* write_buffer, const char* string) {
    return unabto_query_write_uint8_list(write_buffer, (uint8_t *)string, strlen(string));
}

application_event_result application_event(application_request* request,
                                           unabto_query_request* query_request,
                                           unabto_query_response* query_response)
{
    NABTO_LOG_INFO(("Nabto application_event: %u", request->queryId));
    debug_dump_acl();

    if (request->queryId == 0) {
        // AMP get_interface_info.json
        if (!write_string(query_response, device_interface_id_)) return AER_REQ_RSP_TOO_LARGE;
        if (!unabto_query_write_uint16(query_response, device_interface_version_major_)) return AER_REQ_RSP_TOO_LARGE;
        if (!unabto_query_write_uint16(query_response, device_interface_version_minor_)) return AER_REQ_RSP_TOO_LARGE;
        return AER_REQ_RESPONSE_READY;

    } else if (request->queryId == 10000) {
        // AMP get_public_device_info.json
        if (!write_string(query_response, device_name)) return AER_REQ_RSP_TOO_LARGE;
        if (!write_string(query_response, product_name)) return AER_REQ_RSP_TOO_LARGE;
        if (!write_string(query_response, device_icon)) return AER_REQ_RSP_TOO_LARGE;
        if (!unabto_query_write_uint8(query_response, fp_acl_is_pair_allowed(request))) return AER_REQ_RSP_TOO_LARGE;
        if (!unabto_query_write_uint8(query_response, fp_acl_is_user_paired(request))) return AER_REQ_RSP_TOO_LARGE;
        if (!unabto_query_write_uint8(query_response, fp_acl_is_user_owner(request))) return AER_REQ_RSP_TOO_LARGE;
        return AER_REQ_RESPONSE_READY;

    } else if (request->queryId == 10010) {
        int res;
        // AMP set_device_info.json
        if (!fp_acl_is_request_allowed(request, REQUIRES_OWNER)) return AER_REQ_NO_ACCESS;
        res = copy_string(query_request, device_name, sizeof(device_name));
        if (res != AER_REQ_RESPONSE_READY) return res;
        if (!write_string(query_response, device_name)) return AER_REQ_RSP_TOO_LARGE;
        return AER_REQ_RESPONSE_READY;

    } else if (request->queryId >= 11000 && request->queryId < 12000) {
        NABTO_LOG_INFO(("ACL before handling ACL event:"));
        debug_dump_acl();

        // PPKA access control
        int res = fp_acl_ae_dispatch(11000, request, query_request, query_response);
        NABTO_LOG_INFO(("ACL after handling ACL event:"));
        debug_dump_acl();

        return res;

    } else {
        NABTO_LOG_WARN(("Unhandled query id: %u", request->queryId));
        return AER_REQ_INV_QUERY_ID;
    }
}

bool application_poll_query(application_request** applicationRequest) {
    return false;
}

application_event_result application_poll(application_request* applicationRequest, unabto_query_response* writeBuffer) {
    return AER_REQ_SYSTEM_ERROR;
}

void application_poll_drop(application_request* applicationRequest) {
}

#if NABTO_ENABLE_LOCAL_PSK_CONNECTION
bool is_obscurity_key(const struct unabto_psk_id* keyId) {
    // dummy key for trusted network bootstrapping
    return memcmp(keyId, obscurity_key_id, PSK_ID_LENGTH) == 0;
}

bool unabto_local_psk_connection_get_key(const struct unabto_psk_id* keyId, const char* clientId, const struct unabto_optional_fingerprint* pkFp, struct unabto_psk* key) {
    void* it;
    struct fp_acl_user user;

    if (is_obscurity_key(keyId)) {
        memcpy(&key->data, obscurity_key, PSK_LENGTH);
        return true;
    }

    if (!pkFp->hasValue) {
        return false;
    }

    it = fp_acl_db.find(&pkFp->value);

    if (it && fp_acl_db.load(it, &user) == FP_ACL_DB_OK && user.pskId.hasValue && user.psk.hasValue) {
        if (memcmp(keyId, user.pskId.value.data, FP_ACL_PSK_ID_LENGTH) == 0) {
            memcpy(&key->data, user.psk.value.data, FP_ACL_PSK_KEY_LENGTH);
            return true;
        }
    }

    NABTO_LOG_WARN(("User with fingerprint [%2x:%2x:%2x:...] is not configured with key [%2x:%2x:%2x:...]",
                    pkFp->value.data[0], pkFp->value.data[1], pkFp->value.data[2],
                    keyId->data[0], keyId->data[1], keyId->data[2]));
    return false;
}
#endif
