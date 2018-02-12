/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _DHCP_CLIENT_H_
#define _DHCP_CLIENT_H_

#include <unabto/unabto_env_base.h>
#include <unabto/unabto_external_environment.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  bool isBound;
  uint32_t serverAddress;
  uint32_t leaseTime;
  uint32_t localAddress;
  uint32_t netmask;
  uint32_t routerAddress;
  uint32_t dnsAddress;
} dhcp_client_information;

extern dhcp_client_information dhcpClientInformation;

void dhcp_client_initialize(const uint8_t* mac);
void dhcp_client_tick(void);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
