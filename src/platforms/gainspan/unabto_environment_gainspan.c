/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include "unabto_env_base.h"
#include "unabto_context.h"
#include <errno.h>
#include "gsn_includes.h"


void nabto_bsd_set_nonblocking(nabto_socket_t* socketDescriptor) {
    tfBlockingState(*socketDescriptor, TM_BLOCKING_OFF);
}

void nabto_resolve_ipv4(uint32_t ipv4, struct nabto_ip_address* ip) {
    ip->type = NABTO_IP_V4;
    ip->addr.ipv4 = ipv4;
}

void nabto_dns_resolve(const char* id) {

}

nabto_dns_status_t nabto_dns_is_resolved(const char* id, struct nabto_ip_address* v4addr) {
    struct addrinfo* result;
    struct addrinfo hint;
    nabto_dns_status_t status = NABTO_DNS_ERROR;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = PF_INET;
    getaddrinfo(id,NULL, NULL, &result);
    if (result == NULL) {
        return NABTO_DNS_ERROR;
    }
    
    if (result->ai_addr->sa_family == AF_INET) {
        struct sockaddr_in* addr = (struct sockaddr_in*)result->ai_addr;
        v4addr->type = NABTO_IP_V4;
        v4addr->addr.ipv4 = htonl(addr->sin_addr.s_addr);
        status = NABTO_DNS_OK;
    }
    freeaddrinfo(result);
    return status;
}

