/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _DNS_CLIENT_H_
#define _DNS_CLIENT_H_

#include <unabto/unabto_env_base.h>
#include <unabto/unabto_external_environment.h>

#ifdef __cplusplus
extern "C" {
#endif

void dns_client_initialize(uint32_t dnsServerIp);

void dns_client_tick(void);

void dns_client_nabto_dns_resolve(const char* host);

/**
 * resolve an ipv4 dns address
 * if resolving fails in first attempt we call the function later to 
 * see if the address is resolved. The id is always constant for a device
 * meaning the address could be hardcoded but then devices will fail if 
 * the basestation gets a new ip address.
 * @param id      name controller hostname
 * @param v4addr  pointer to ipaddress
 * @return false if address is not resolved yet
 */
nabto_dns_status_t dns_client_nabto_dns_is_resolved(const char* host, uint32_t* ip);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
