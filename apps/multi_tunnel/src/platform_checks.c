#include "platform_checks.h"
#include <unabto/unabto_env_base.h>
#include <unabto/unabto_logging.h>
#include <unabto/unabto_util.h>

bool test_if_lo_exists();

bool platform_checks() {
    bool result = true;
#if !(defined(WIN32) || defined(MOXA))
    result &= test_if_lo_exists();
#endif
    return result;
}

#if !(defined(WIN32) || defined(MOXA))

#include <sys/types.h>
#include <ifaddrs.h>
#include <net/if.h>

bool test_if_lo_exists() {
    bool foundLoopback = false;

    struct ifaddrs *addrs,*tmp;

    getifaddrs(&addrs);
    tmp = addrs;

    while (tmp)
    {
        const char* lo = "lo";
        if (tmp->ifa_addr) {
            if (strncmp(tmp->ifa_name, lo, strlen(lo)) == 0) {
                if (! (tmp->ifa_flags & (IFF_UP))) {
                    NABTO_LOG_FATAL(("Loopback interface exists but it's not up"));
                } else if (! (tmp->ifa_flags & (IFF_UP))) {
                    NABTO_LOG_FATAL(("Loopback interface exists but it's not running"));
                } else {
                    foundLoopback = true;
                }
            }
        }

        tmp = tmp->ifa_next;
    }

    freeifaddrs(addrs);

    if (!foundLoopback) {
        NABTO_LOG_FATAL(("No loopback interface found, searched for an interface named lo. This is required if the tunnel should be able to connect to services on localhost."));
    }
    return foundLoopback;
}
#endif


#if defined(WIN32)

int check_ulimit_files(int desired) {
    return -1;
}

#else
#include <sys/time.h>
#include <sys/resource.h>

int check_ulimit_files(int desired) {
    struct rlimit rlim;
    if (-1 == getrlimit(RLIMIT_NOFILE, &rlim)) {
        return -1;
    }

    if (rlim.rlim_cur < desired) {
        NABTO_LOG_INFO(("ulimit too low raising it"));
        rlim.rlim_cur = MIN(desired, rlim.rlim_max);
        if (-1 == setrlimit(RLIMIT_NOFILE, &rlim)) {
            return -1;
        }
    }
    return rlim.rlim_cur;
}

#endif
