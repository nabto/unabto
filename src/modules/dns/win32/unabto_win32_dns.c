#include <unabto/unabto_external_environment.h>
#include <unabto/unabto_util.h>

#include <winsock2.h>
#include <windows.h>

typedef struct {
    const char* id;
    uint32_t resolved_addr[NABTO_DNS_RESOLVED_IPS_MAX];
    nabto_dns_status_t status;
} resolver_state_t;

static resolver_state_t resolver_state;

static bool resolver_is_running = false;

DWORD WINAPI resolver_thread(LPVOID ctx) {
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
            READ_U32(state->resolved_addr[i], addr);
        }
    }
    resolver_is_running = false;
    return 0;
}

static int create_detached_resolver() {
    HANDLE thread;
    thread = CreateThread(NULL, 0, resolver_thread, &resolver_state, 0, NULL);
    if (!thread) {
        return -1;
    }
    return 0;
}

void nabto_dns_resolve(const char* id) {
    uint32_t addr = inet_addr(id);
    if (addr != INADDR_NONE) {
        resolver_state.resolved_addr[0] = htonl(addr);
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
            exit(1);
        }
    }
}

nabto_dns_status_t nabto_dns_is_resolved(const char *id, uint32_t* v4addr) {
    if (resolver_is_running) {
        return NABTO_DNS_NOT_FINISHED;
    }
    
    if (resolver_state.status == NABTO_DNS_OK) {
        uint8_t i;
        for (i = 0; i < NABTO_DNS_RESOLVED_IPS_MAX; i++) {
            v4addr[i] = resolver_state.resolved_addr[i];
        }
        return NABTO_DNS_OK;
    }
    return NABTO_DNS_ERROR;
}
