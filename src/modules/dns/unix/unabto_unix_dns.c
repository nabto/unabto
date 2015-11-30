#include <unabto/unabto_external_environment.h>

#include <pthread.h>

typedef struct {
    const char* id;
    uint32_t resolved_addr;
    nabto_dns_status_t status;
    pthread_t thread;
    bool thread_created;
} resolver_state_t;

static resolver_state_t resolver_state;

static bool resolver_is_running = false;

void* resolver_thread(void* ctx) {
    resolver_state_t* state = (resolver_state_t*)ctx;

    struct hostent* he = gethostbyname(state->id);
    if (he == 0) {
        state->status = NABTO_DNS_ERROR;
    } else {
        state->status = NABTO_DNS_OK;
        state->resolved_addr = htonl(*((uint32_t*)he->h_addr_list[0]));
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
    uint32_t addr = inet_addr(id);
    if (addr != INADDR_NONE) {
        resolver_state.resolved_addr = htonl(addr);
        resolver_state.status = NABTO_DNS_OK;
    } else {
        // host isn't a dotted IP, so resolve it through DNS
        if (resolver_is_running) {
            return;
        }

        resolver_is_running = true;
        resolver_state.status = NABTO_DNS_NOT_FINISHED;
        resolver_state.id = id;
        if (create_detached_resolver() != 0) {
            resolver_is_running = false;
            resolver_state.status = NABTO_DNS_ERROR;
            NABTO_LOG_FATAL(("Cannot create detached resolver."));
        }
    }
}

nabto_dns_status_t nabto_dns_is_resolved(const char *id, uint32_t* v4addr) {
    if (resolver_is_running) {
        return NABTO_DNS_NOT_FINISHED;
    }
    
    if (resolver_state.status == NABTO_DNS_OK) {
        *v4addr = resolver_state.resolved_addr;
        return NABTO_DNS_OK;
    }
    return NABTO_DNS_ERROR;
}
