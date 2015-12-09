#include "platform_checks.h"
#include <unabto/unabto_env_base.h>
#include <unabto/unabto_logging.h>

bool test_if_lo_exists();

bool platform_checks() {
    bool result = true;
#ifndef WIN32
    result &= test_if_lo_exists();
#endif
    return result;
}

#ifndef WIN32

#include <sys/types.h>
#include <ifaddrs.h>

bool test_if_lo_exists() {
    bool foundLoopback = false;
    
    struct ifaddrs *addrs,*tmp;
    
    getifaddrs(&addrs);
    tmp = addrs;
    
    while (tmp)
    {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_PACKET) {
            if (strcmp(tmp->ifa_name, "lo") == 0) {
                foundLoopback = true;
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
