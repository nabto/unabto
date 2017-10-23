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
