/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#include "unabto/unabto_env_base.h"
#include "modules/cli/unabto_args.h"
#include "modules/diagnostics/unabto_diag.h"
#include <modules/util/read_hex.h>
#if NABTO_ENABLE_PROVISIONING
#include "modules/provision/unabto_provision_gopt.h"
#endif

#include "unabto/unabto_logging.h"
#include "unabto/unabto_common_main.h"
#include "unabto_version.h"
#include "gopt.h" // http://www.purposeful.co.uk/software/gopt/


#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#else
#define strdup _strdup
#endif

/**
 * Print help message
 * @param errmsg  if not 0 this is an error message
 */
static void help(const char* errmsg, const char *progname)
{
    if (errmsg) {
        printf("ERROR: %s\n", errmsg);
    }
    printf("Usage: %s -d <host id> [options]\n", progname);
#if NABTO_ENABLE_PROVISIONING
    printf(" - or: %s -P [<provisioning options>] [general options]\n", progname);
#endif
    printf("    -d: the host id to use (e.g. myweatherstation.nabto.net)\n");
    printf("    -s: use encryption (if no -k parameter specified, a null preshared secret is used)\n");
    printf("    -k: preshared key to use for encryption\n");
    printf("    -A: register with specified basestation\n");
    printf("        - if omitted, the basestation address is resolved using the specified server id\n");
    printf("    -a: bind to specified local address\n");
    printf("    -p: bind to specified local port\n");
    printf("    -i: bind to specified local interface (Unix only)\n");
    printf("    -V: print version and exit\n");
    printf("    -C: print configuration (unabto_config.h) and exit\n");
    printf("    -S: print size (in bytes) of memory usage and exit\n");
    printf("    -U: override the default html device driver url\n");

#if NABTO_ENABLE_PROVISIONING
    unabto_provision_gopt_help(progname);
#endif


} /* help(const char* errmsg) */

enum {
    FORCE_DNS_FALLBACK = 127,
    ENABLE_DNS_FALLBACK,
    DNS_ADDRESS,
    DNS_FALLBACK_DOMAIN,
};

#if NABTO_ENABLE_PROVISIONING
#define UNABTO_PROVISION_GOPT_START_ENUM 255
#endif

bool check_args(int argc, char* argv[], nabto_main_setup *nms)
{
    const char *address;
    const char *basestationAddress;
    const char *preSharedKey;
    const char *localPortStr;
    const char *bufferSizeStr;
    const char *idParam;
    const char *interfaceParam;
    const char *htmlddurloverride;
    uint32_t addr;
    const char *progname;
#ifdef WIN32
    char modulename[MAX_PATH];
#endif

#if NABTO_ENABLE_DNS_FALLBACK
    const char *dnsAddress;
    const char *dnsFallbackDomain;
#endif

    uint8_t psk[16] = { 0 };
    
    const char x0s[] = "h?";     const char* x0l[] = { "help", "HELP", 0 };
    const char x1s[] = "a";      const char* x1l[] = { "localAddress", 0 };
    const char x2s[] = "d";      const char* x2l[] = { "deviceName", 0 };
    const char x3s[] = "l";      const char* x3l[] = { "log", 0 };
    const char x4s[] = "A";      const char* x4l[] = { "controllerAddress", 0 };
    const char x5s[] = "p";      const char* x5l[] = { "localport", 0 };
    const char x6s[] = "b";      const char* x6l[] = { "buffersize", 0 };
    const char x7s[] = "k";      const char* x7l[] = { "presharedkey", 0 };
    const char x8s[] = "s";      const char* x8l[] = { "securedevice", 0 };
    const char x9s[] = "n";      const char* x9l[] = { "datanullencrypted", 0 };
    const char x10s[] = "i";     const char* x10l[] = { "interface", 0 };
    const char x11s[] = "U";     const char* x11l[] = { "htmlddurloverride", 0 };
    const char x12s[] = "V";     const char* x12l[] = { "version", 0 };
    const char x13s[] = "C";     const char* x13l[] = { "config", 0 };
    const char x14s[] = "S";     const char* x14l[] = { "size", 0 };
    const char x15s[] = "";      const char* x15l[] = { "forcednsfallback", 0 };
    const char x16s[] = "";      const char* x16l[] = { "dnsaddress", 0 };
    const char x17s[] = "";      const char* x17l[] = { "dnsfallbackdomain", 0 };
    const char x18s[] = "P";     const char* x18l[] = { "provision", 0 };
    const char x19s[] = "";      const char* x19l[] = { "enablednsfallback", 0 };

    const struct { int k; int f; const char *s; const char*const* l; } opts[] = {
        { 'h', 0,           x0s, x0l },
        { 'a', GOPT_ARG,    x1s, x1l },
        { 'd', GOPT_ARG,    x2s, x2l },
        { 'l', GOPT_REPEAT, x3s, x3l },
        { 'A', GOPT_ARG,    x4s, x4l },
        { 'p', GOPT_ARG,    x5s, x5l },
        { 'b', GOPT_ARG,    x6s, x6l },
        { 'k', GOPT_ARG,    x7s, x7l },
        { 's', 0,           x8s, x8l },
        { 'n', 0,           x9s, x9l },
        { 'i', GOPT_ARG,    x10s, x10l },
        { 'U', GOPT_ARG,    x11s, x11l },
        { 'V', GOPT_NOARG,  x12s, x12l },
        { 'C', GOPT_NOARG,  x13s, x13l },
        { 'S', GOPT_NOARG,  x14s, x14l },
        { FORCE_DNS_FALLBACK, GOPT_NOARG, x15s, x15l },
        { DNS_ADDRESS, GOPT_ARG, x16s, x16l },
        { DNS_FALLBACK_DOMAIN, GOPT_ARG, x17s, x17l },
        { 'P', GOPT_NOARG,    x18s, x18l },
        { ENABLE_DNS_FALLBACK, GOPT_NOARG, x19s, x19l },

#if NABTO_ENABLE_PROVISIONING
    UNABTO_PROVISION_GOPT_ARGS()
#endif
        { 0,0,0,0 }
    };

    void *options = gopt_sort( & argc, (const char**)argv, opts);
    int idOk = 0;
    
#ifdef WIN32
    modulename[0] = 0;
    GetModuleFileNameA(NULL, modulename, sizeof(modulename));
    progname = strrchr(modulename, '\\');
    if (!progname)
        progname = modulename;
    else
        progname++;
#else
    progname = strrchr(argv[0], '/');
    if (!progname)
        progname = argv[0];
    else
        progname++;
#endif

    if( gopt( options, 'h')) {
        help("Help", progname);
        return false;
    }
    
    if (gopt(options, 'V')) {
        fprintf(stdout, PRIversion "\n", MAKE_VERSION_PRINTABLE());
        return false;
    }

    if( gopt( options, 'C')) {
        unabto_printf_unabto_config(stdout, progname);
        return false;
    }
    
    if (gopt(options, 'S')) {
        unabto_printf_memory_sizes(stdout, progname);
        return false;
    }

    if( gopt_arg( options, 'p', &localPortStr) ){
        nms->localPort = atoi(localPortStr);
    }

    if( gopt_arg( options, 'b', &bufferSizeStr) ){
        nms->bufsize = (uint16_t)atoi(bufferSizeStr);
    }

    if( gopt_arg( options, 'A', & basestationAddress ) ){
        addr = inet_addr(basestationAddress);
        if (addr == INADDR_NONE) {
            help("Illegal basestation address", progname);
            gopt_free(options);
            return false;
        }
        nms->controllerArg.addr.type = NABTO_IP_V4;
        nms->controllerArg.addr.addr.ipv4 = htonl(addr);
    }

    if ( gopt_arg( options, 'k', &preSharedKey)) {
        if (!unabto_read_psk_from_hex(preSharedKey, psk, 16)) {
            return false;
        }
    }

    if (gopt(options, 's')) {
#if NABTO_ENABLE_CONNECTIONS
        if (!unabto_set_aes_crypto(nms, psk, 16)) {
            return false;
        }
#endif
    }
    if (gopt(options, 'n')) {
        nms->secureData = false;
    }

    if (gopt_arg(options, 'i', &interfaceParam)) {
        nms->interfaceName = strdup(interfaceParam);
    }

    if (gopt_arg(options, 'U', &htmlddurloverride)) {
        nms->url = strdup(htmlddurloverride);
    }

#if NABTO_ENABLE_PROVISIONING
    if (gopt(options, 'P')) {
        idOk = unabto_provision_gopt_apply(nms, progname, options);
    }
#endif
    if (!idOk) {
        if( gopt_arg( options, 'd', &idParam ) ){
            nms->id = strdup(idParam);
        } else {
#if NABTO_ENABLE_PROVISIONING
            help("You must either specify valid provisioning options or specify a uNabto device id", progname);
#else
            help("You must specify an id for your uNabto device", progname);
#endif
            gopt_free(options);
            return false;
        }
    }
    
#if NABTO_ENABLE_DNS_FALLBACK
    if (gopt(options, FORCE_DNS_FALLBACK)) {
        nms->forceDnsFallback = true;
    }

    if(gopt(options, ENABLE_DNS_FALLBACK)) {
        nms->enableDnsFallback = true;
    }

    if (gopt_arg(options, DNS_ADDRESS, &dnsAddress)) {
        addr = inet_addr(dnsAddress);
        if (addr == INADDR_NONE) {
            help("Illegal dns address", progname);
            gopt_free(options);
            return false;
        }
        nms->dnsAddress = htonl(addr);
    }

    if (gopt_arg(options, DNS_FALLBACK_DOMAIN, &dnsFallbackDomain)) {
        nms->dnsFallbackDomain = strdup(dnsFallbackDomain);
    }
#endif

    gopt_free(options);
    
    return true;
}
