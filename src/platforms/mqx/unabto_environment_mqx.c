/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/**
 * @file
 * The environment for the Nabto Micro Device Server (PC Device), Implementation.
 *
 */

#define NABTO_DECLARED_MODULE NABTO_LOG_NETWORK

#include <errno.h>
#include <stdint.h>
#include <string.h>

#ifdef hest
#include "../../unabto_external_environment.h"
#include "../../unabto_logging.h"
#include "../../unabto_context.h"
#include "../../unabto_environment.h"
#else
#include "unabto_environment.h"
#include "unabto_logging.h"
#include "unabto_context.h"
#endif

#include <ipcfg.h>
typedef uint16_t socklen_t;


/******************************************************************************/
/******************************************************************************/

#define ETH_DEVICE 0

bool initInf( void );
bool initDNS( void );


void nabto_random(uint8_t* buf, size_t len)
{
    /* FIXME: this is not random - but crypto isn't included yet, so don't care */
    size_t ix;
    for (ix = 0; ix < len; ++ix) {
        *buf++ = (uint8_t)ix;
    }
} /* void nabto_random(uint8_t* buf, size_t len) */

static bool initInf( void )
{
    #define ENET_IPADDR IPADDR(169,254,3,3)
    #define ENET_IPMASK IPADDR(255,255,0,0)
    #define ENET_GATEWAY IPADDR(0,0,0,0)
    uint32_t           error;
    _enet_address     address;
    IPCFG_IP_ADDRESS_DATA ip_data;

    static bool infInitialized = false;
    if (infInitialized)
        return true;
    
#if RTCS_MINIMUM_FOOTPRINT
   /* runtime RTCS configuration for devices with small RAM, for others the default BSP setting is used */
   _RTCSPCB_init = 4;
   _RTCSPCB_grow = 2;
   _RTCSPCB_max = 20;
   _RTCS_msgpool_init = 4;
   _RTCS_msgpool_grow = 2;
   _RTCS_msgpool_max  = 20;
   _RTCS_socket_part_init = 4;
   _RTCS_socket_part_grow = 2;
   _RTCS_socket_part_max  = 20;
#endif
    
    error = RTCS_create();
    if (error != RTCS_OK) 
        return false;

     /* 
      * For this demo the mac address is calculated from IP ADDRES 
      * Replace this line with proper mac address initialization
      */
    ENET_get_mac_address (ETH_DEVICE, ENET_IPADDR, address);
    
    
    error = ipcfg_init_device (ETH_DEVICE, address);
    if (error != RTCS_OK) 
       return false;
    
    while(!ipcfg_get_link_active(ETH_DEVICE));
    while (ipcfg_task_poll() == FALSE);
    
    ip_data.ip = ENET_IPADDR;
    ip_data.mask = ENET_IPMASK;
    ip_data.gateway = ENET_GATEWAY;     
    error = ipcfg_bind_dhcp_wait(ETH_DEVICE, FALSE, &ip_data);
    return (error == IPCFG_ERROR_OK);
}


/******************************************************************************/
/******************************************************************************/


/******************************************************************************/
#define INADDR_NONE 0xffffffffu
    //
#define AF_INET         1
bool nabto_init_socket(uint16_t* localPort, nabto_socket_t* socket) {
    nabto_socket_t sd = RTCS_SOCKET_ERROR;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd == RTCS_SOCKET_ERROR) {
        return false;
    } else 
    {
        uint32_t optVal = TRUE;
        struct sockaddr_in sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = INADDR_ANY;
        sa.sin_port = *localPort;
        setsockopt(sd, SOL_UDP, OPT_RECEIVE_NOWAIT, &optVal, sizeof(uint32_t));
        if (bind(sd, &sa, sizeof(sa)) != RTCS_OK) {
 //           printf("bind() fails, the IP-address may be wrong or another process conflicts\n");
            shutdown(sd, 0); // second parameter ignored
            return false;
        }
        *socket = sd;
        {
            struct sockaddr_in sao;
            socklen_t len = sizeof(sao);
            if ( getsockname(*socket, &sao, &len) != -1) {
                *localPort = sao.sin_port;
            } else {
 //               NABTO_LOG_INFO(("error getting local port"));
            }
        }
    }
    return true;
}


/******************************************************************************/

bool nabto_init_platform()
{
    return initInf();
}

/******************************************************************************/
void nabto_close_platform() {
}


/******************************************************************************/

void nabto_close_socket(nabto_socket_t* socket)
{
    if (socket && *socket != NABTO_INVALID_SOCKET) {
        shutdown(*socket, 0);
        *socket = NABTO_INVALID_SOCKET;
    }

} /* void nabto_close_socket(nabto_socket_t* socketLocal) */

/******************************************************************************/

ssize_t nabto_read(nabto_socket_t socket,
                   uint8_t*       buf,
                   size_t         len,
                   struct nabto_ip_address*      addr,
                   uint16_t*      port)
{
    int res;
    struct sockaddr_in sa;
    socklen_t salen = sizeof(sa);
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    res = recvfrom(socket, buf, (uint32_t)len, 0, &sa, &salen);
    if (res > 0)
        addr->type = NABTO_IP_V4;
        addr->addr.ipv4 = ntohl(sa.sin_addr.s_addr);
    if (res >= 0) {
        addr->type = NABTO_IP_V4;
        addr->addr.ipv4 = sa.sin_addr.s_addr;
        *port = sa.sin_port;
    } else if (res == -1) {
        return 0;
    } else {
        int err = errno;
        if (1) {
 //           NABTO_LOG_INFO(("ERROR: %i in nabto_read()", err));
        }
    }

    return res;
} /* size_t nabto_read(socket, buf, len, addr, port) */

/******************************************************************************/
/******************************************************************************/

ssize_t nabto_write(nabto_socket_t socket,
                 const uint8_t* buf,
                 size_t         len,
                 uint32_t       addr,
                 uint16_t       port)
{
    int res;
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = addr;
    sa.sin_port = port;
    res = sendto(socket, (void*)buf, (uint32_t)len, 0, &sa, sizeof(sa));
    if (res < 0) {
        int err = errno;
        NABTO_LOG_INFO(("ERROR: %i in nabto_write()", err));
    }
    return res;
} /* bool nabto_write(socket, buf, len, addr, port) */

/******************************************************************************/
/******************************************************************************/
void nabto_yield(int timeOut) {
    ipcfg_task_poll();
    _time_delay((unsigned long)timeOut);
}

nabto_stamp_t nabtoGetStamp( void )
{
    TIME_STRUCT time;
    _time_get_elapsed(&time);
    return time.MILLISECONDS;
}

#define MAX_STAMP_DIFF 0x7fffffff;

bool nabtoIsStampPassed(nabto_stamp_t *stamp) {
    return *stamp - nabtoGetStamp() > MAX_STAMP_DIFF;
}

nabto_stamp_diff_t nabtoStampDiff(nabto_stamp_t * newest, nabto_stamp_t * oldest) {
    return (*newest - *oldest);
}

int nabtoStampDiff2ms(nabto_stamp_diff_t diff) {
    return (int) diff;
}

void nabto_dns_resolve(const char* id){
}

nabto_dns_status_t nabto_dns_is_resolved(const char *id, uint32_t* v4addr)
{
    #define MAX_HOSTNAMESIZE     64
    char hostname[MAX_HOSTNAMESIZE];
 
    memset (hostname, 0, sizeof (hostname));
    if (!RTCS_resolve_ip_address( (char*)id, v4addr, hostname, MAX_HOSTNAMESIZE ))
        return NABTO_DNS_ERROR;

    return NABTO_DNS_OK;
}
