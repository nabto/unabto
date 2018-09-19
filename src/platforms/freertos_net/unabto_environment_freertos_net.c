/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/**
 * @file
 * The Basic environment for the Nabto FreeRTOS NET Device Server implementation.
 *
 */

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS+UDP includes. */
#include "FreeRTOS_UDP_IP.h"
#include "FreeRTOS_Sockets.h"

/* Nabto includes. */
#include <unabto/unabto.h>

/******************************************************************************/

bool nabto_init_platform( void )
{
    return true;
}
/******************************************************************************/

void nabto_close_platform( void )
{
}
/******************************************************************************/

#define MAX_SOCKETS 5 // TBD - get from unabto.h;

static uint16_t nSockets=0;
static nabto_socket_t activeSockets[MAX_SOCKETS];

nabto_socket_t *getActiveSockets(uint16_t *nS)
{
    nabto_socket_t * aS = NULL;
    if (0 < nSockets)
    {
        *nS = nSockets;
        aS = activeSockets;
    }
    return aS;
}

bool nabto_init_socket( uint16_t* localPort, nabto_socket_t* socketDescriptor )
{
int to = 0;
struct freertos_sockaddr xAddress, *pxAddressToUse;
bool bReturn = true;

    if (MAX_SOCKETS <= nSockets)
    {
        bReturn = false;
    }
    else
    {
        *socketDescriptor = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_DGRAM, FREERTOS_IPPROTO_UDP );

        if( NULL == *socketDescriptor )
        {
            bReturn = false;
        }
        else
        {
            memset( &xAddress, 0, sizeof( xAddress ) );
            xAddress.sin_addr = INADDR_ANY;
            xAddress.sin_port = FreeRTOS_htons( *localPort );


            pxAddressToUse = &xAddress;


            if( 0 > FreeRTOS_bind(*socketDescriptor, pxAddressToUse, sizeof( xAddress ) ) )
            {
                FreeRTOS_closesocket( *socketDescriptor );
                bReturn = false;
            }
            else
            {
                /* Set receive time out to 0.  Timeouts are performed using a
                select() call. */
                FreeRTOS_setsockopt( *socketDescriptor, 0, FREERTOS_SO_RCVTIMEO, &to, sizeof( to ) );
                activeSockets[nSockets++] = *socketDescriptor;
            }

            *localPort = FreeRTOS_htons( xAddress.sin_port );
        }
    }
    return bReturn;
}
/******************************************************************************/

ssize_t nabto_read( nabto_socket_t socket,
                    uint8_t*       buf,
                    size_t         len,
                    struct nabto_ip_address*      addr,
                    uint16_t*      port )
{
    int res;
    struct freertos_sockaddr xAddress;
    socklen_t xAddresslen = sizeof(xAddress);

    memset( &xAddress, 0, sizeof( xAddress ) );
    res = FreeRTOS_recvfrom( socket, buf, ( int ) len , 0, &xAddress, &xAddresslen );

    if( res > 0 )
    {
        addr->type = NABTO_IP_V4;
        addr->addr.ipv4 = FreeRTOS_htonl( xAddress.sin_addr );
        *port = FreeRTOS_htons( xAddress.sin_port );
    }
    else
    {
        res = 0;
    }

    return res;
} 
/******************************************************************************/

ssize_t nabto_write( nabto_socket_t socket,
                     const uint8_t* buf,
                     size_t         len,
                     struct nabto_ip_address*       addr,
                     uint16_t       port )
{
    int res;
    struct freertos_sockaddr xAddress;
    if (addr->type != NABTO_IP_V4) {
        return 0;
    }

    memset( &xAddress, 0, sizeof( xAddress ) );
    xAddress.sin_addr = FreeRTOS_htonl( addr->addr.ipv4 );
    xAddress.sin_port = FreeRTOS_htons( port );
    res = FreeRTOS_sendto( socket, buf, ( int )len, 0, ( struct freertos_sockaddr * ) &xAddress, sizeof( xAddress ) );
    return res;
} 
/******************************************************************************/

void nabto_close_socket( nabto_socket_t* socketDescriptor )
{
    if (socketDescriptor) 
    {
        FreeRTOS_closesocket(*socketDescriptor);
        *socketDescriptor = NULL;
    }
}
/******************************************************************************/

void nabto_dns_resolve(const char* id) 
{
    ( void ) id;
}
/******************************************************************************/

nabto_dns_status_t nabto_dns_is_resolved( const char *id, uint32_t* v4addr )
{
    uint32_t addr;
    nabto_dns_status_t status;

    addr = FreeRTOS_gethostbyname( id );

    if( 0 == addr )
    {
        status = NABTO_DNS_ERROR;
    }
    else
    {
        *v4addr = FreeRTOS_htonl(addr);
        status = NABTO_DNS_OK;
    }

    return status;
}
/******************************************************************************/

void setTimeFromGSP( uint32_t stamp )
{
    NABTO_NOT_USED(stamp);
}
