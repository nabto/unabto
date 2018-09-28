/*
* Copyright (C) Nabto - All Rights Reserved.
*/
/*
* deunabto_external_environment.c
*
*  Created on: 6-10-2011
*      Author: jbk@nabto.com
*/

#include "FreeRTOS.h"
#include "semphr.h"
#include "unabto_env_base_freertos_lwip.h"
#include "unabto/unabto_external_environment.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "unabto/unabto_util.h"

#define MAX_SOCKETS             2


typedef struct RecvBuffer
{
    struct pbuf *buf;
    u32_t recvAddr;
    u16_t recvPort;
} RecvBuffer;

typedef struct Socket // "socket"
{
    struct udp_pcb *pcb;
    RecvBuffer recvBuffer;
} Socket;

static struct Socket sockets[MAX_SOCKETS] = { {.pcb = NULL }, {.pcb = NULL }};


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
        *buf = (uint8_t)i;
}


static void udpRecv(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
    static int dropped=0;
    RecvBuffer *rb = &((Socket*)arg)->recvBuffer;
    if(NULL == rb->buf)
    {
        rb->buf = p;
        rb->recvAddr = addr->addr; // * ntohl(addr->addr);
        rb->recvPort = port;
    }
    else
    {
        pbuf_free(p); // drop
        dropped++;
    }
}


/******************************************************************************/
void nabto_socket_set_invalid(nabto_socket_t* socket)
{
    socket = NABTO_INVALID_SOCKET;
}

/**
* Initialise a udp socket.  This function is called for every socket
* uNabto creates, this will normally occur two times. One for local
* connections and one for remote connections.
*
* @param localPort    The local port to bind to.
*                     A port number of 0 gives a random port.
* @param socket       To return the created socket descriptor.
* @return             true iff successfull
*/
bool nabto_socket_init(uint16_t* localPort, nabto_socket_t* socket)
{
    int i;
    struct ip_addr ipAddr;
    
    if (NULL == socket)
        return false; 
    
    for (i=0; i < MAX_SOCKETS && sockets[i].pcb; i++);
    if (i == MAX_SOCKETS)
        return false;
        
    sockets[i].pcb = udp_new();
    sockets[i].pcb->so_options |= SOF_BROADCAST;
    
    udp_recv(sockets[i].pcb, udpRecv, &sockets[i]);
    ipAddr.addr = IP_ADDR_ANY; 
    if (ERR_OK != udp_bind(sockets[i].pcb, IP_ADDR_ANY , *localPort))
        return false;
    
    *socket = i;
    return true;
}

bool nabto_socket_is_equal(const nabto_socket_t* s1, const nabto_socket_t* s2)
{
    return *s1==*s2;
}

/**
* Close a socket. 
* Close can be called on already closed sockets. And should tolerate this behavior.
*
* @param socket the socket to be closed
*/
void nabto_socket_close(nabto_socket_t* socket)
{
    if (NULL == socket)
        return; 
    
    if (0 > *socket || *socket >= MAX_SOCKETS)
        return;
    
    if (NULL != sockets[*socket].pcb)
    {
        udp_remove(sockets[*socket].pcb);
        sockets[*socket].pcb = NULL;
    }
}

static uint32_t loops=0;

ssize_t nabto_read(nabto_socket_t socket,
                   uint8_t*       buf,
                   size_t         len,
                   struct nabto_ip_address*      addr,
                   uint16_t*      port)
{
    ssize_t res = (ssize_t)len;
    if (0 > socket || socket >= MAX_SOCKETS)
        return -1;
    
    loops++;
    RecvBuffer *rb = &sockets[socket].recvBuffer;
    if (NULL != rb->buf)
    {
        struct pbuf *p, *pHead = rb->buf;
        p = pHead;
        while (p && p->len <= len)
        {
            memcpy(buf, p->payload, p->len);
            len -= p->len;
            buf += p->len;
            p = p->next;
        }
        pbuf_free(pHead);
        addr->type = NABTO_IP_V4;
        addr->addr.ipv4 = ntohl(rb->recvAddr);
    //          READ_U32(*addr, &rb->recvAddr);

        *port = rb->recvPort;
        rb->buf = NULL;
    }
    return res-(ssize_t)len;
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
*/
ssize_t nabto_write(nabto_socket_t socket,
                    const uint8_t* buf,
                    size_t         len,
                    struct nabto_ip_address*       addr,
                    uint16_t       port)
{
    ssize_t res = -1;
    static struct pbuf *p=NULL;
    struct ip_addr ipAddr;
    
    if (0 > socket || socket >= MAX_SOCKETS || NULL == buf || 0 == len || addr->type != NABTO_IP_V4)
        return -1;
        
    p = pbuf_alloc(PBUF_TRANSPORT, (unsigned short)len, PBUF_RAM);
    if (NULL == p)
        return -1;
    
    pbuf_take(p, buf, (unsigned short)len);
    ipAddr.addr = htonl(addr->addr.ipv4);
    if (ERR_OK == udp_sendto(sockets[socket].pcb, p, &ipAddr, port))
        res = (ssize_t)len;
    pbuf_free(p);
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

static uint32_t basestationIPV4 = 0; 

static void dns_callback(const char *name, struct ip_addr *ipAddr, void *dummy)
{
    (void)dummy;
    if ((ipAddr) && (ipAddr->addr))
    {
        basestationIPV4 = ntohl(ipAddr->addr);
    }
}

void nabto_resolve_ipv4(uint32_t ipv4, struct nabto_ip_address* ip) {
    ip->type = NABTO_IP_V4;
    ip->addr.ipv4 = ipv4;
}

void nabto_dns_resolve(const char* id)
{
    struct ip_addr resolvedIp;
    switch(dns_gethostbyname(id, &resolvedIp, dns_callback, NULL))
    {
        case ERR_OK:
            basestationIPV4 = ntohl(resolvedIp.addr);
            break;
        case ERR_INPROGRESS:
            basestationIPV4 = 0;
            break;
        default:
            NABTO_LOG_ERROR(("DNS call failed"));
    }
}

//TBC
nabto_dns_status_t nabto_dns_is_resolved(const char *id, struct nabto_ip_address* v4addr)
{
    if (basestationIPV4)
    {
        v4addr->type = NABTO_IP_V4;
        v4addr->addr.ipv4 = basestationIPV4;
        return NABTO_DNS_OK;
    }
    else
        return NABTO_DNS_ERROR;
}



