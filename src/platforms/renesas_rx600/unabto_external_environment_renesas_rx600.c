/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/*
 * deunabto_external_environment.c
 *
 *  Created on: 6-10-2011
 *      Author: jbk@nabto.com
 */
#include <stdio.h>
#include "uip.h"
#include "clock-arch.h"
#include "unabto_common_main.h"
#include "unabto_env_base.h"



uint32_t basestationIPV4 = 0; // for export

#define UIP_BUFFERS 5
#define UIP_BUFFER_SIZE 1550 // relocate
#define MAX_SOCKETS 2

typedef struct XmitBuffer
{
    uint16_t len;
    uint16_t port;
    uint32_t addr;
    uint8_t buffer[UIP_BUFFER_SIZE];
} XmitBuffer;

typedef struct Socket
{
    bool used;
    size_t recvLen;
    struct uip_udp_conn *conn; // matching "connection", appstate is used by dhcp
    int xmitBufHead, xmitBufTail, xmitBufElms;
    XmitBuffer xmitBuffers[UIP_BUFFERS]; // keep data until the matching port is polled by unabto
} Socket;

static struct Socket _sockets[MAX_SOCKETS] = { {.used = false }, {.used = false }};    

XmitBuffer *socketPacketBufferGet(int socket)
{
    if (0 > socket || socket >= MAX_SOCKETS || 0 == _sockets[socket].xmitBufElms)
        return NULL;
    _sockets[socket].xmitBufElms--;
    _sockets[socket].xmitBufTail = (_sockets[socket].xmitBufTail + 1) % UIP_BUFFERS;
    return &_sockets[socket].xmitBuffers[_sockets[socket].xmitBufTail];
}

XmitBuffer *socketPacketBufferNew(int socket)
{
    if (0 > socket || socket >= MAX_SOCKETS || UIP_BUFFERS <= _sockets[socket].xmitBufElms)
        return NULL;
    _sockets[socket].xmitBufElms++;
    _sockets[socket].xmitBufHead = (_sockets[socket].xmitBufHead + 1) % UIP_BUFFERS;
    return &_sockets[socket].xmitBuffers[_sockets[socket].xmitBufHead];
}


/** 
 * Fill buffer with random content.
 * @param buf  the buffer
 * @param len  the length of the buffer
 */
void nabto_random(uint8_t* buf, size_t len)
{
    int i;
    if (NULL == buf)
        return;
    for (i=0; i < len; i++)
        *buf++ = rand();
}




/******************************************************************************/
/**
 * Initialise a udp socket.  This function is called for every socket
 * uNabto creates, this will normally occur two times. One for local
 * connections and one for remote connections.
 *
 * @param localAddr    The local address to bind to.
 * @param localPort    The local port to bind to.
 *                     A port number of 0 gives a random port.
 * @param socket       To return the created socket descriptor.
 * @return             true iff successfull
 */
bool nabto_init_socket(uint32_t localAddr, uint16_t* localPort, nabto_socket_t* socket)
{
    int i;
    uint8_t *ap = (uint8_t*)&localAddr;
    uip_ipaddr_t ipAddr;
    struct uip_udp_conn *newConnection;
    
    if (NULL == socket)
        return false; 
    
    for (i=0; i < MAX_SOCKETS && _sockets[i].used; i++);
    if (i == MAX_SOCKETS)
        return false;


    uip_ipaddr(&ipAddr, ap[3], ap[2], ap[1], ap[1]); // Any address
    newConnection = uip_udp_new(&ipAddr, 0);
    if (NULL == newConnection)
        return false;
    
    if (0 != *localPort)
        uip_udp_bind(newConnection, HTONS(*localPort));
    newConnection->clrport = 1; // hack
    uip_listen(HTONS(*localPort));  
    *socket = i;
    _sockets[i].conn = newConnection;
    _sockets[i].conn->appstate.callBack = unabto_app;
    _sockets[i].conn->appstate.unabto = i;
    _sockets[i].used = true;
    _sockets[i].xmitBufHead = _sockets[i].xmitBufTail = _sockets[i].xmitBufElms = 0;
    return true;
}

/**
 * Close a socket. 
 * Close can be called on already closed sockets. And should tolerate this behavior.
 *
 * @param socket the socket to be closed
 */
void nabto_close_socket(nabto_socket_t* socket)
{
    if (NULL == socket)
        return; 
    
    if (0 > *socket || *socket >= MAX_SOCKETS)
        return;
    
    _sockets[*socket].used = false;
}

static uint32_t loops=0;


// Return with data if the uip has new data for this specific socket
ssize_t nabto_read(nabto_socket_t socket,
                   uint8_t*       buf,
                   size_t         len,
                   uint32_t*      addr,
                   uint16_t*      port)
{
    size_t res;
    uint8_t *ap = (uint8_t*)addr;
    if (0 > socket || MAX_SOCKETS <= socket  || uip_poll() || socket != uip_udp_conn->appstate.unabto)
        return 0;

    res = len  < uip_datalen()?len:uip_datalen();
    if (0 < res)
    {
        memcpy(buf, uip_appdata, res);
        // host order little endian
        ap[3] = uip_ipaddr1(uip_udp_conn->srcipaddr);
        ap[2] = uip_ipaddr2(uip_udp_conn->srcipaddr);
        ap[1] = uip_ipaddr3(uip_udp_conn->srcipaddr);
        ap[0] = uip_ipaddr4(uip_udp_conn->srcipaddr);
        *port = HTONS(uip_udp_conn->srcport);
    }
    loops++;
    return res;
}

static void uipSend(const uint8_t* buf, size_t len, uint32_t addr, uint16_t port)
{
    uip_ipaddr_t ipAddr;
    uint8_t *ap = (uint8_t*)&addr;
    // host order little endian
    uip_ipaddr(ipAddr,  ap[3], ap[2], ap[1], ap[0]);  
    uip_udp_sendto(buf, len, ipAddr, HTONS(port));
}

/******************************************************************************/
/**
 * Write message to network (blocking) The memory allocation and
 * deallocation for the buffer is handled by the caller.
 *
 * @param socket  the UDP socket
 * @param buf   the bytes to be sent
 * @param len   number of bytes to be sent
 * @param addr  the receivers IP address (host byte order)
 * @param port  the receivers UDP port (host byte order)
 * @return      true when success
 *
 * With uip we can't be sure, the the current connection is the one unabto wants to use,
 * if not we copies the buffer, address etc.
 */
ssize_t nabto_write(nabto_socket_t socket,
                    const uint8_t* buf,
                    size_t         len,
                    uint32_t       addr,
                    uint16_t       port)
{
    static int dropped=0;
    ssize_t res = -1;
    XmitBuffer *xmitBuf;

    if (0 > socket || socket >= MAX_SOCKETS)
        return -1;


    if (0 == uip_sdatalen() && uip_udp_conn->appstate.unabto == socket)
    {
        uipSend(buf, len, addr, port);
        res = len; //...
    }
    else if (NULL != (xmitBuf = socketPacketBufferNew(socket)))
    {
        // Keep until uip calls back
        xmitBuf->addr = addr;
        xmitBuf->port = port;
        xmitBuf->len = len;
        memcpy(xmitBuf->buffer, buf, len);
    } 
    else
        dropped++;
    return res;
}

/******************************************************************************/

/**
 * Init the platform. This is called once when the platform is
 * initialised. This can be used to initialise platform specific behavior.
 * The function is called before first socket operations.
 * @return true if success.
 */
bool nabto_init_platform(){return true;};

/*****************************************************************************/
/**
 * Close the platform. This is called once when the platform is
 * closed.
 * When this function is called no further socket invocations will occur. This
 * can be used to make cleanup on the platform.
 */
void nabto_close_platform(){return;};

void resolv_found (char *name, u16_t *ipaddr)
{
}

void nabto_dns_resolve(const char* id)
{
    if (NULL != id)
        resolv_query(id);
}

//TBC
nabto_dns_status_t nabto_dns_is_resolved(const char *id, uint32_t* v4addr)
{
//    static ip_addr_t bsAddr;
    uip_ipaddr_t *ipAddr;

    if (NULL == (ipAddr = resolv_lookup(id)))
        return NABTO_DNS_ERROR;

    uint8_t *ap = (uint8_t*)v4addr;
    // host order little endian
    ap[3] = uip_ipaddr1(ipAddr);
    ap[2] = uip_ipaddr2(ipAddr);
    ap[1] = uip_ipaddr3(ipAddr);
    ap[0] = uip_ipaddr4(ipAddr);
    return NABTO_DNS_OK;
}

static nabto_main_context _nmc;

#define MY_ID_POSTFIX "renesas.u.nabto.net"

void initNabto()
{
    extern struct uip_eth_addr my_mac;
    static char myId[100];
    nabto_init_default_values(&_nmc.nabtoMainSetup);
    sprintf(myId, "%02x%02x%02x%02x%02x%02x."MY_ID_POSTFIX,
                   my_mac.addr[0], my_mac.addr[1], my_mac.addr[2], 
                   my_mac.addr[3],my_mac.addr[4],my_mac.addr[5]);  
    _nmc.nabtoMainSetup.id = myId;
    nabto_main_init(&_nmc);
}


void unabto_app()
{
    int i;

    if (!dhcp_configured())
        return; 

    for (i=0; i < MAX_SOCKETS; i++)
        if (_sockets[i].used && uip_udp_conn->appstate.unabto == i)
        {
            XmitBuffer *xmitBuf = socketPacketBufferGet(i);
            if (NULL == xmitBuf)
                break;
            uipSend(xmitBuf->buffer, xmitBuf->len,
                    xmitBuf->addr, xmitBuf->port);
            break;
        }
        
    nabto_main_tick(&_nmc); // unabto will call read and read packet data
}

void app_dispatcher()
{
    if (NULL != uip_udp_conn)
        uip_udp_conn->appstate.callBack();
}

