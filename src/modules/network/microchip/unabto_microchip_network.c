/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NETWORK

/*
 * This macro uniquely defines this file as the main entry point.
 * There should only be one such definition in the entire project,
 * and this file must define the AppConfig variable as described below.
 * 
 */
#define THIS_IS_STACK_APPLICATION

#include "unabto_microchip_network.h"
#include <TCPIP Stack/TCPIP.h>
#include "unabto_microchip_arp.h"
#include <unabto/unabto_external_environment.h>
#include <unabto/unabto_common_main.h>

APP_CONFIG AppConfig; // Declare the AppConfig data structure (required by Microchips TCP/IP stack)

void network_initialize(void)
{
  // Configure TCP/IP stack stuff
  AppConfig.Flags.bIsDHCPEnabled = true;
  AppConfig.MyIPAddr.Val = MY_DEFAULT_IP_ADDR_BYTE1 | MY_DEFAULT_IP_ADDR_BYTE2 << 8ul | MY_DEFAULT_IP_ADDR_BYTE3 << 16ul | MY_DEFAULT_IP_ADDR_BYTE4 << 24ul;
  AppConfig.DefaultIPAddr.Val = AppConfig.MyIPAddr.Val;
  AppConfig.MyMask.Val = MY_DEFAULT_MASK_BYTE1 | MY_DEFAULT_MASK_BYTE2 << 8ul | MY_DEFAULT_MASK_BYTE3 << 16ul | MY_DEFAULT_MASK_BYTE4 << 24ul;
  AppConfig.DefaultMask.Val = AppConfig.MyMask.Val;
  AppConfig.MyGateway.Val = MY_DEFAULT_GATE_BYTE1 | MY_DEFAULT_GATE_BYTE2 << 8ul | MY_DEFAULT_GATE_BYTE3 << 16ul | MY_DEFAULT_GATE_BYTE4 << 24ul;
  AppConfig.PrimaryDNSServer.Val = MY_DEFAULT_PRIMARY_DNS_BYTE1 | MY_DEFAULT_PRIMARY_DNS_BYTE2 << 8ul | MY_DEFAULT_PRIMARY_DNS_BYTE3 << 16ul | MY_DEFAULT_PRIMARY_DNS_BYTE4 << 24ul;
  AppConfig.SecondaryDNSServer.Val = MY_DEFAULT_SECONDARY_DNS_BYTE1 | MY_DEFAULT_SECONDARY_DNS_BYTE2 << 8ul | MY_DEFAULT_SECONDARY_DNS_BYTE3 << 16ul | MY_DEFAULT_SECONDARY_DNS_BYTE4 << 24ul;

#if UNABTO_PLATFORM_PIC18

  /**
   * To Remove any leftover state from earlier initializations of the
   * MAC we first disable rx, then reset rx and tx and lastly disable
   * the ethernet MAC.
   */
  ECON1bits.TXRST = 1;
  ECON1bits.RXRST = 1;
  Delay10us(1);
  ECON1bits.TXRST = 0;
  ECON1bits.RXRST = 0;
  Delay10us(1);
  ECON2bits.ETHEN = 0;
  Delay10us(1);

#endif

  StackInit();

  unabto_microchip_arp_reset();
}

void network_tick(void)
{
  static uint32_t lastIp = 0xffffffff;

  StackTask();

  unabto_microchip_arp_tick();
  nabto_dns_resolver();

  if(network_get_current_ip_address(&lastIp))
  {
    NABTO_LOG_TRACE(("Got IP address: " PRIip, MAKE_IP_PRINTABLE(lastIp)));
    unabto_microchip_arp_reset();
    unabto_notify_ip_changed(lastIp);
  }
}

bool network_get_current_ip_address(uint32_t* ip)
{
  uint32_t mirroredIp = swapl(AppConfig.MyIPAddr.Val);

  if(*ip != mirroredIp)
  {
    *ip = mirroredIp;
    return true;
  }
  else
  {
    return false;
  }
}
