# Porting uNabto - creating  a new uNabto platform adapter 

In this section a new uNabto platform adapter will be created for the
linux platform. In each step we will describe what is neccessary.

A uNabto platform adapter is the code which the uNabto Core uses for
network communication, time handling, dns resolving, logging,
randomness etc. We already provide modules for many platforms for
network, dns, timing, encryption, random etc. they are located in the
modules folder in the source tree. All the functionality in this
description is also available as uNabto modules and the implementation
using modules can be found under the official unix platform in the
platforms/unix folder in the uNabto source.

We will describe all the neccessary steps to create a new platform
here. First we will describe the overall structure of a uNabto
platform adapter and secondly we will describe de individual module
implementations which is neccessary for a complete uNabto platform
adapter.

## Overall structure.

The overall structure of a uNabto platform adapter consists of 2
header files and several source files. The two header files are
`unabto_platform_types.h` and `unabto_platform.h`. The first header file
describes all the types neccessary for uNabto to run while the latter
describes all the platform dependencies which isn't types this is for
example macro definitions. A uNabto platform adapter is added to a
uNabto project by adding the uNabto platform adapter to the compilers
include path, then uNabto will include the respective
`unabto_platform.h` and `unabto_platform_types.h`

## Basic code

First we create a main program which can run uNabto with the platform
adapter we are developing. And secondly a uNabto configuration file
which sets the outline for the implementation.

The main file below is just a simple runner which setup uNabto and
calls the tick function.


```C
// code from src/main.c
#include <unabto/unabto_common_main.h>
#include <unabto/unabto_app.h>
int main() {
    nabto_main_setup* nms = unabto_init_context();
    nms->id = "myid.example.net";
    unabto_init();
    while(true) {
        unabto_tick();
    }
}

application_event_result application_event(application_request* request, unabto_query_request* read_buffer, unabto_query_response* write_buffer) {
    return AER_REQ_INV_QUERY_ID;
}
```

uNabto configuration file.


```C
// code from src/unabto_config.h
#define UNABTO_PLATFORM_CUSTOM 1
#define NABTO_SET_TIME_FROM_ALIVE 0
#define NABTO_ENABLE_STREAM 0
```

## Implementing `unabto_platform_types.h`

Next we create the `unabto_platform_types.h` file it should define all
neccessary types for unabto. Thats at least timestamps, `int*_t`
`uint*_t`, booleans and sockets.


```C
// code from src/unabto_platform_types.h
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

typedef uint64_t nabto_stamp_t;
typedef int64_t nabto_stamp_diff_t;

typedef int nabto_socket_t;
```

## Implementing `unabto_platform.h`

When we have all the basic types we need to make a `unabto_platform.h`
implementation. This can be used to implement all the adhoc functions
which isn't neccessarily specified as a link time dependency in
`unabto_external_environment.h`

```C
// code from src/unabto_platform.h
#include "unabto_platform_types.h"

#include <platforms/unabto_common_types.h>

#define NABTO_INVALID_SOCKET -1

#define nabtoMsec2Stamp(stamp) (stamp)

#define NABTO_LOG_BASIC_PRINT(severity, message)
```

## Implementing a network adapter

The network adapter should implement the functions which is neccessary
for creating communication sockets. This is init, close, read and
write functionality for network data. A simple unix implementation is
like

```C
// code from src/network_adapter.c
#include <unabto/unabto_external_environment.h>
#include <unabto/unabto_logging.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>


bool nabto_init_socket(uint32_t localAddr, uint16_t* localPort, nabto_socket_t* sock) {
    nabto_socket_t sd;
    
    NABTO_LOG_TRACE(("Open socket: ip=" PRIip ", port=%u", MAKE_IP_PRINTABLE(localAddr), (int)*localPort));
    
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd == -1) {
        NABTO_LOG_ERROR(("Unable to create socket: (%i) '%s'.", errno, strerror(errno)));
        return false;
    }

    {
        struct sockaddr_in sa;
        int status;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = INADDR_ANY;
        sa.sin_port = htons(*localPort);
        
        status = bind(sd, (struct sockaddr*)&sa, sizeof(sa));
        
        if (status < 0) {
            NABTO_LOG_ERROR(("Unable to bind socket: (%i) '%s'.", errno, strerror(errno)));
            close(sd);
            return false;
        }
        int flags = fcntl(sd, F_GETFL, 0);
        if (flags == -1) flags = 0;
        fcntl(sd, F_SETFL, flags | O_NONBLOCK);
        *sock = sd;
    }
    {
        struct sockaddr_in sao;
        socklen_t len = sizeof(sao);
        if ( getsockname(*sock, (struct sockaddr*)&sao, &len) != -1) {
            *localPort = htons(sao.sin_port);
        } else {
            NABTO_LOG_ERROR(("Unable to get local port of socket: (%i) '%s'.", errno, strerror(errno)));
        }
    }
    
    NABTO_LOG_TRACE(("Socket opened: ip=" PRIip ", port=%u", MAKE_IP_PRINTABLE(localAddr), (int)*localPort));
    
    return true;
}

void nabto_close_socket(nabto_socket_t* sock) {
    if (sock && *sock != NABTO_INVALID_SOCKET) {
        close(*sock);
        *sock = NABTO_INVALID_SOCKET;
    }
}

ssize_t nabto_read(nabto_socket_t sock,
                   uint8_t*       buf,
                   size_t         len,
                   uint32_t*      addr,
                   uint16_t*      port)
{
    int res;
    struct sockaddr_in sa;
    int salen = sizeof(sa);
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    res = recvfrom(sock, (char*) buf, (int)len, 0, (struct sockaddr*)&sa, &salen);
    
    if (res >= 0) {
        *addr = ntohl(sa.sin_addr.s_addr);
        *port = ntohs(sa.sin_port);
    }
    return res;
}

ssize_t nabto_write(nabto_socket_t sock,
                 const uint8_t* buf,
                 size_t         len,
                 uint32_t       addr,
                 uint16_t       port)
{
    int res;
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(addr);
    sa.sin_port = htons(port);
    res = sendto(sock, buf, (int)len, 0, (struct sockaddr*)&sa, sizeof(sa));
    if (res < 0) {
        NABTO_LOG_ERROR(("ERROR: %i in nabto_write()", (int) errno));
    }
    return res;
}
```

## Implementing a time adapter

The uNabto core has several timers which handle timed events, like
reconnecting or retransmitting a packet. In order to facilitate this
some time functions must be available. The timing needed does not need
to be absolute, it should just be monotonically increasing in some
arbitrary time period.

```C
// code from src/time_adapter.c
#include <unabto/unabto_external_environment.h>

nabto_stamp_t nabtoGetStamp() {
    struct timespec res;
    clock_gettime(CLOCK_MONOTONIC, &res);
    return res.tv_sec+(res.tv_nsec/1000000);
}

bool nabtoIsStampPassed(nabto_stamp_t* stamp) {
    nabto_stamp_t now = nabtoGetStamp();
    return *stamp <= now;
}

nabto_stamp_diff_t nabtoStampDiff(nabto_stamp_t * newest, nabto_stamp_t * oldest) {
    return newest - oldest;
}

int nabtoStampDiff2ms(nabto_stamp_diff_t diff) {
    return (int) diff;
}
```

## Implementing a dns adapter

The dns adapter should be able to do an asynchronous dns resolving.

```C
// code from src/dns_adapter.c
#include <unabto/unabto_external_environment.h>
#include <unabto/unabto_util.h>

#include <netdb.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

typedef struct {
    const char* id;
    uint32_t resolved_addrs[NABTO_DNS_RESOLVED_IPS_MAX];
    nabto_dns_status_t status;
    pthread_t thread;
} resolver_state_t;

static resolver_state_t resolver_state;

static bool resolver_is_running = false;

void* resolver_thread(void* ctx) {
    resolver_state_t* state = (resolver_state_t*)ctx;

    struct hostent* he = gethostbyname(state->id);
    if (he == 0) {
        state->status = NABTO_DNS_ERROR;
    } else if (he->h_addrtype == AF_INET && he->h_length == 4) {
        uint8_t i;
        state->status = NABTO_DNS_OK;
        for (i = 0; i < NABTO_DNS_RESOLVED_IPS_MAX; i++) {
            uint8_t* addr = (uint8_t*)he->h_addr_list[i];
            if (addr == NULL) {
                break;
            }
            READ_U32(state->resolved_addrs[i], addr);
        }
    }
    resolver_is_running = false;
    return NULL;
}

static int create_detached_resolver() {
    pthread_t thread;
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) != 0) {
        return -1;
    }
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
        pthread_attr_destroy(&attr);
        return -1;
    }
    if (pthread_create(&thread, &attr, resolver_thread, &resolver_state) != 0) {
        pthread_attr_destroy(&attr);
        return -1;
    }
    pthread_attr_destroy(&attr);
    return 0;
}

void nabto_dns_resolve(const char* id) {
    if (resolver_is_running) {
        return;
    }
    memset(resolver_state.resolved_addrs, 0, NABTO_DNS_RESOLVED_IPS_MAX*sizeof(uint32_t));
    resolver_is_running = true;
    resolver_state.status = NABTO_DNS_NOT_FINISHED;
    resolver_state.id = id;
    if (create_detached_resolver() != 0) {
        resolver_is_running = false;
        resolver_state.status = NABTO_DNS_ERROR;
        exit(1);
    }
}

nabto_dns_status_t nabto_dns_is_resolved(const char *id, uint32_t* v4addrs) {
    if (resolver_is_running) {
        return NABTO_DNS_NOT_FINISHED;
    }
    
    if (resolver_state.status == NABTO_DNS_OK) {
        uint8_t i;
        for (i = 0; i < NABTO_DNS_RESOLVED_IPS_MAX; i++) {
            v4addrs[i] = resolver_state.resolved_addrs[i];
        }
        return NABTO_DNS_OK;
    }
    return NABTO_DNS_ERROR;
}

```

## Implementing a random adapter

```C
// code from src/random_adapter.c
#include <unabto/unabto_external_environment.h>
#include <openssl/rand.h>

void nabto_random(uint8_t* buf, size_t len) {
    if (!RAND_bytes((unsigned char*)buf, len)) {
        NABTO_LOG_FATAL(("RAND_bytes failed."));
    }
}
```
