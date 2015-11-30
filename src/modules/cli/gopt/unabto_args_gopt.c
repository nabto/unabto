/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#include "unabto/unabto_env_base.h"
#include "modules/cli/unabto_args.h"
#include "modules/diagnostics/unabto_diag.h"

#ifdef NABTO_ENABLE_PROVISIONING
#  if !(defined(NABTO_PROVISION_HOST) && defined(NABTO_PROVISION_API_KEY) && defined(NABTO_DEVICE_KEY_FILE))
#    error("Missing static configuration of provisioning host, api key and keyfile")
#  endif
#include "modules/provision/unabto_provision.h"
#endif

#include "unabto/unabto_logging.h"
#include "unabto_version.h"
#include "gopt.h" // http://www.purposeful.co.uk/software/gopt/

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>


/// Provide the safe version API. @param src source @param format formatstring @param param1 the parameter
/// TODO: use proper linux variant!!!!
#define sscanf_s(src, format, param1) sscanf(src, format, param1)

#else
#define strdup _strdup
#endif

/**
 * @file
 * The uNabto Program Argument Verification (PC Device) Implementation.
 */


/**
 * Print help message
 * @param errmsg  if not 0 this is an error message
 */
static void help(const char* errmsg, const char *progname)
{
    if (errmsg) {
        NABTO_LOG_INFO(("ERROR: %s", errmsg));
    }
    NABTO_LOG_INFO(("Usage: %s -d <host id> [options]", progname));
#if NABTO_ENABLE_PROVISIONING
    NABTO_LOG_INFO((" - or: %s -P <mac address to use for provisioning> [options]", progname));
#endif
    NABTO_LOG_INFO(("    -d: the host id to use (e.g. myweatherstation.nabto.net)"));
    NABTO_LOG_INFO(("    -s: use encryption (if no -k parameter specified, a null preshared secret is used)"));
    NABTO_LOG_INFO(("    -k: preshared key to use for encryption"));
    NABTO_LOG_INFO(("    -A: register with specified basestation"));
    NABTO_LOG_INFO(("        - if omitted, the basestation address is resolved using the specified server id"));
    NABTO_LOG_INFO(("    -a: bind to specified local address"));
    NABTO_LOG_INFO(("    -p: bind to specified local port"));
    NABTO_LOG_INFO(("    -i: bind to specified local interface (Unix only)"));
    NABTO_LOG_INFO(("    -V: print version and exit"));
    NABTO_LOG_INFO(("    -C: print configuration (unabto_config.h) and exit"));
    NABTO_LOG_INFO(("    -S: print size (in bytes) of memory usage and exit"));

#if NABTO_ENABLE_DNS_FALLBACK
    NABTO_LOG_INFO(("Dns fallback options"));
    NABTO_LOG_INFO(("    --enablednsfallback: enable unabto via dns"));
    NABTO_LOG_INFO(("    --forcednsfallback: force unabto via dns, do not wait for normal udp attach to fail"));
    NABTO_LOG_INFO(("    --dnsaddress: override system dns server"));
    NABTO_LOG_INFO(("    --dnsfallbackdomain: override default dnsfallback domain"));
#endif
} /* help(const char* errmsg) */

enum {
    FORCE_DNS_FALLBACK = 127,
    ENABLE_DNS_FALLBACK,
    DNS_ADDRESS,
    DNS_FALLBACK_DOMAIN
};

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

#if NABTO_ENABLE_PROVISIONING
    const char *macAddr;
#endif
    
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
    const char x18s[] = "P";     const char* x18l[] = { "provision-mac", 0 };
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
        { 'P', GOPT_ARG,    x18s, x18l },
        { ENABLE_DNS_FALLBACK, GOPT_NOARG, x19s, x19l },
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
        fprintf(stdout, "%d.%d\n", RELEASE_MAJOR, RELEASE_MINOR);
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

    if( gopt_arg( options, 'a', & address ) ){
        addr = inet_addr(address);
        if (addr == INADDR_NONE) {
            help("Illegal local address", progname);
            gopt_free(options);
            return false;
        }
		nms->ipAddress = htonl(addr);
    } 

    if( gopt_arg( options, 'p', &localPortStr) ){
		nms->localPort = atoi(localPortStr);
    }

    if( gopt_arg( options, 'b', &bufferSizeStr) ){
        nms->bufsize = (size_t)atoi(bufferSizeStr);
    }

    if( gopt_arg( options, 'A', & basestationAddress ) ){
        addr = inet_addr(basestationAddress);
        if (addr == INADDR_NONE) {
            help("Illegal basestation address", progname);
            gopt_free(options);
            return false;
        }
        nms->controllerArg.addr = htonl(addr);
    }

    if ( gopt_arg( options, 'k', &preSharedKey)) {
        size_t i;
        size_t pskLen = strlen(preSharedKey);
        // read the pre shared key as a hexadecimal string.
        for (i = 0; i < pskLen/2 && i < 16; i++) {
            sscanf_s(preSharedKey+(2*i), "%02hhx", &nms->presharedKey[i]);
        }
    }

    if (gopt(options, 's')) {
        nms->secureAttach= true;
        nms->secureData = true;
#if NABTO_ENABLE_CONNECTIONS
        nms->cryptoSuite = CRYPT_W_AES_CBC_HMAC_SHA256;
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
    if ( gopt_arg( options, 'P', &macAddr)) {
        char url[1024];
        if (snprintf(url, sizeof(url), "https://%s/mr/provision/app_simple.json?mac=%s&api_key=%s", NABTO_PROVISION_HOST, macAddr, NABTO_PROVISION_API_KEY) > sizeof(url)) {
            help("Illegal provision info", progname);
            return false;
        }
        if (!unabto_provision_persistent(nms, url, NABTO_DEVICE_KEY_FILE)) {
            NABTO_LOG_FATAL(("uNabto provisioning failed"));
        }
        idOk = 1;
    }
#endif
    if (!idOk) {
        if( gopt_arg( options, 'd', &idParam ) ){
            nms->id = strdup(idParam);
        } else {
#if NABTO_ENABLE_PROVISIONING
            help("You must either specify a mac address for your device for provisioning or specify a uNabto device id", progname);
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
