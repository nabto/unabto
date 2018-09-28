/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include <unabto/unabto_env_base.h>
#include "w5100_network.h"
#include <device_drivers/w5100/w5100.h>
#include <modules/network/dhcp/dhcp_client.h>
#include <modules/network/dns/dns_client.h>
#include <unabto/unabto_common_main.h>

void network_initialize(const uint8_t* mac)
{
  w5100_initialize();
  w5100_set_mac_address(mac);

  dhcp_client_initialize(mac);
}

void network_tick(void)
{
  static uint8_t dhcpIsBound = false;
  
  if(dhcpIsBound == false)
  {
    if(dhcpClientInformation.isBound == true)
    {
      NABTO_LOG_TRACE(("DHCP discovery succeeded: ip=" PRIip, MAKE_IP_PRINTABLE(dhcpClientInformation.localAddress)));

      w5100_set_local_address(dhcpClientInformation.localAddress);
      w5100_set_netmask(dhcpClientInformation.netmask);
      w5100_set_gateway_address(dhcpClientInformation.routerAddress);

      dns_client_initialize(dhcpClientInformation.dnsAddress);

      unabto_notify_ip_changed(dhcpClientInformation.localAddress);

      dhcpIsBound = true;
    }
  }
  else
  {
    dns_client_tick();
    if(dhcpClientInformation.isBound == false)
    {
      dhcpIsBound = false;
    }
  }

  dhcp_client_tick();
}

bool network_get_current_ip_address(uint32_t* ip)
{
  uint32_t currentIp = dhcpClientInformation.localAddress;

  if(*ip != currentIp)
  {
    *ip = currentIp;
    return true;
  }
  else
  {
    return false;
  }
}

void nabto_resolve_ipv4(uint32_t ipv4, struct nabto_ip_address* ip) {
    ip->type = NABTO_IP_V4;
    ip->addr.ipv4 = ipv4;
}

void nabto_dns_resolve(const char* id)
{
  dns_client_nabto_dns_resolve(id);
}

nabto_dns_status_t nabto_dns_is_resolved(const char* id, struct nabto_ip_address* v4addr)
{
  v4addr->type = NABTO_IP_V4;
  return dns_client_nabto_dns_is_resolved(id, &v4addr->addr.ipv4);
}
