/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
/*
 * This macro uniquely defines this file as the main entry point.
 * There should only be one such definition in the entire project,
 * and this file must define the AppConfig variable as described below.
 */

#define ENABLE_DHCP_BUG_WORKAROUND 0

#include "unabto/unabto_env_base.h"
#include "modules/network/microchip/unabto_microchip_arp.h"
#include "unabto/unabto_common_main.h"

#include "TCPIP Stack/TCPIP.h"
#include "unabto/unabto_util.h"

// Declare the AppConfig data structure (required by Microchips TCP/IP stack)
APP_CONFIG AppConfig;

bool platform_initialize() {
    uint32_t lastIp;
    nabto_stamp_t timeout;

    // Configure TCP/IP stack stuff
    AppConfig.Flags.bIsDHCPEnabled = true;
    AppConfig.Flags.bInConfigMode = true;
    AppConfig.MyIPAddr.Val = MY_DEFAULT_IP_ADDR_BYTE1 | MY_DEFAULT_IP_ADDR_BYTE2 << 8ul | MY_DEFAULT_IP_ADDR_BYTE3 << 16ul | MY_DEFAULT_IP_ADDR_BYTE4 << 24ul;
    AppConfig.DefaultIPAddr.Val = AppConfig.MyIPAddr.Val;
    AppConfig.MyMask.Val = MY_DEFAULT_MASK_BYTE1 | MY_DEFAULT_MASK_BYTE2 << 8ul | MY_DEFAULT_MASK_BYTE3 << 16ul | MY_DEFAULT_MASK_BYTE4 << 24ul;
    AppConfig.DefaultMask.Val = AppConfig.MyMask.Val;
    AppConfig.MyGateway.Val = MY_DEFAULT_GATE_BYTE1 | MY_DEFAULT_GATE_BYTE2 << 8ul | MY_DEFAULT_GATE_BYTE3 << 16ul | MY_DEFAULT_GATE_BYTE4 << 24ul;
    AppConfig.PrimaryDNSServer.Val = MY_DEFAULT_PRIMARY_DNS_BYTE1 | MY_DEFAULT_PRIMARY_DNS_BYTE2 << 8ul | MY_DEFAULT_PRIMARY_DNS_BYTE3 << 16ul | MY_DEFAULT_PRIMARY_DNS_BYTE4 << 24ul;
    AppConfig.SecondaryDNSServer.Val = MY_DEFAULT_SECONDARY_DNS_BYTE1 | MY_DEFAULT_SECONDARY_DNS_BYTE2 << 8ul | MY_DEFAULT_SECONDARY_DNS_BYTE3 << 16ul | MY_DEFAULT_SECONDARY_DNS_BYTE4 << 24ul;

    TickInit();
    
    StackInit();

    
    unabto_microchip_arp_reset();
    

// Wait for dhcp.
    lastIp = AppConfig.MyIPAddr.Val;

#if ENABLE_DHCP_BUG_WORKAROUND == 1
    nabtoSetFutureStamp(&timeout, 5000);
#endif

    NABTO_LOG_DEBUG(("Acquiring IP from DHCP server..."));

    while (lastIp == AppConfig.MyIPAddr.Val) {
        StackTask();

#if ENABLE_DHCP_BUG_WORKAROUND == 1
        if (nabtoIsStampPassed(&timeout)) {
            Reset();
        }
#endif
    }

    READ_U32(lastIp, &AppConfig.MyIPAddr.Val);

    NABTO_LOG_DEBUG(("Got IP from DHCP server: "PRIip, MAKE_IP_PRINTABLE(lastIp)));

    return true;
}

void platform_shutdown(void) {
}

void platform_tick(void) {
}
