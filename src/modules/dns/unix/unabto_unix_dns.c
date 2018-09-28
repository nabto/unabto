#include <unabto/unabto_external_environment.h>
#include <unabto/unabto_util.h>
#include <unabto/unabto_message.h>
#include <unabto/unabto_endpoint.h>

#include <pthread.h>
#include <netdb.h>

typedef struct {
    const char* id;
    struct nabto_ip_address resolved_addrs[NABTO_DNS_RESOLVED_IPS_MAX];
    nabto_dns_status_t status;
    pthread_t thread;
} resolver_state_t;

static resolver_state_t resolver_state;

static bool resolver_is_running = false;


void* resolver_thread(void* ctx) {
    resolver_state_t* state = (resolver_state_t*)ctx;

    struct addrinfo hints;
    struct addrinfo* result;
    int status;
    memset(&hints, 0, sizeof(hints));
    
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_UNSPEC;

    
    status = getaddrinfo(state->id, "4242", &hints, &result);
    if (status != 0) {
        state->status = NABTO_DNS_ERROR;
    } else {
        struct addrinfo* rp;
        uint8_t i;
        state->status = NABTO_DNS_OK;
        // read ipv4 addresses
        for (i = 0, rp = result; i < NABTO_DNS_RESOLVED_IPS_MAX && rp != NULL; rp = rp->ai_next) {
            struct nabto_ip_address* ip = &state->resolved_addrs[i];
            if (rp->ai_family == AF_INET) {
                struct sockaddr_in* sa4;
                ip->type = NABTO_IP_V4;
                sa4 = (struct sockaddr_in*)(rp->ai_addr);
                READ_U32(ip->addr.ipv4, &sa4->sin_addr.s_addr);
                i++;
            }
        }

        if (i == NABTO_DNS_RESOLVED_IPS_MAX && NABTO_DNS_RESOLVED_IPS_MAX >= 2) {
            // make room for atleast one ipv6 address
            i--;
        }
        // read ipv6 addresses
        for (rp = result; i < NABTO_DNS_RESOLVED_IPS_MAX && rp != NULL; rp = rp->ai_next) {
            struct nabto_ip_address* ip = &state->resolved_addrs[i];
            if (rp->ai_family == AF_INET6) {
                struct sockaddr_in6* sa6;
                ip->type = NABTO_IP_V6;
                sa6 = (struct sockaddr_in6*)(rp->ai_addr);
                memcpy(ip->addr.ipv6, sa6->sin6_addr.s6_addr, 16);
                i++;
            }
        }
        freeaddrinfo(result);
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
    // host isn't a dotted IP, so resolve it through DNS
    if (resolver_is_running) {
        return;
    }
    memset(resolver_state.resolved_addrs, 0, sizeof(struct nabto_ip_address)*NABTO_DNS_RESOLVED_IPS_MAX);
    resolver_is_running = true;
    resolver_state.status = NABTO_DNS_NOT_FINISHED;
    resolver_state.id = id;
    if (create_detached_resolver() != 0) {
        resolver_is_running = false;
        resolver_state.status = NABTO_DNS_ERROR;
        NABTO_LOG_FATAL(("Cannot create detached resolver."));
    }
}

nabto_dns_status_t nabto_dns_is_resolved(const char *id, struct nabto_ip_address* addrs) {
    if (resolver_is_running) {
        return NABTO_DNS_NOT_FINISHED;
    }
    
    if (resolver_state.status == NABTO_DNS_OK) {
        uint8_t i;
        for (i = 0; i < 2; i++) {
            addrs[i] = resolver_state.resolved_addrs[i];
        }
        
        return NABTO_DNS_OK;
    }
    return NABTO_DNS_ERROR;
}



void nabto_resolve_ipv4(uint32_t ipv4, struct nabto_ip_address* ip)
{
    struct addrinfo hints;
    struct addrinfo* result;
    struct nabto_ip_address printIp;
    const char* ipv4String;
    int status;
    
    memset(&hints, 0, sizeof(hints));
    
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_UNSPEC;
    
    printIp.type = NABTO_IP_V4;
    printIp.addr.ipv4 = ipv4;
    ipv4String = nabto_ip_to_string(&printIp);
    status = getaddrinfo(ipv4String, "4242", &hints, &result);
    if (status != 0 || result == NULL) {
        NABTO_LOG_ERROR(("getaddrinfo failed unexpectedly"));
        // not possible
        *ip = printIp;
    } else {
        if (result->ai_family == AF_INET) {
            struct sockaddr_in* sa4;
            ip->type = NABTO_IP_V4;
            sa4 = (struct sockaddr_in*)(result->ai_addr);
            READ_U32(ip->addr.ipv4, &sa4->sin_addr.s_addr);
            NABTO_LOG_TRACE(("getaddrinfo returned IPv4 address: %s", nabto_ip_to_string(ip)));
        } else if (result->ai_family == AF_INET6) {
            struct sockaddr_in6* sa6;
            ip->type = NABTO_IP_V6;
            sa6 = (struct sockaddr_in6*)(result->ai_addr);
            memcpy(ip->addr.ipv6, sa6->sin6_addr.s6_addr, 16);
            NABTO_LOG_TRACE(("getaddrinfo returned IPv6 address: %s", nabto_ip_to_string(ip)));
        } else {
            // unknown family
            // probably not possible.
            NABTO_LOG_ERROR(("getaddrinfo returned unknown family"));
            *ip = printIp;
        }
        freeaddrinfo(result);
    }
    
}
