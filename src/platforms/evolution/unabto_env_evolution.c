/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/**
 * @file
 * The implementation of the environment using the Lantronix Evolution SDK.
 *
 */

#include <stddef.h>
#include <stdarg.h>
#include <unabto_env_base_evolution.h>
#include <evolution.h>
#include <evolution_socket.h>
#include <evolution_net.h>
#include <evolution_dns.h>
#include <evolution_errno.h>
#include "../../unabto_external_environment.h"
#include "../../unabto_std.h"
#include "../../unabto_context.h"

/****************************************************************************/
/***********************  CRYPTO  *******************************************/
/****************************************************************************/

void nabto_random(uint8_t* buf, size_t len)
{
    /* FIXME: this is not random - but crypto isn't included yet, so don't care */
    size_t ix;
    for (ix = 0; ix < len; ++ix) {
        *buf++ = (uint8_t)ix;
    }
} /* void nabto_random(uint8_t* buf, size_t len) */

/******************************************************************************/
/***********************  DEVICE  *********************************************/
/******************************************************************************/

bool nabto_init_platform() {
    return true;
}

void nabto_close_platform() {
}

/******************************************************************************/
/***********************  DNS  ************************************************/
/******************************************************************************/

void nabto_dns_resolve(const char* id) {
}

nabto_dns_status_t nabto_dns_is_resolved(const char *id, uint32_t* v4addr) {
    struct hostent res;
    char buffer[2048];
    uint32_t addr;
    struct hostent *he = gethostbyname(id, &res, buffer, sizeof(buffer));
    if (he == 0) {
        return NABTO_DNS_ERROR;
    }
    addr = *((uint32_t*)he->h_addr_list[0]);
    *v4addr = htonl(addr);
    return NABTO_DNS_OK;
}

/****************************************************************************/
/***********************  SOCKETS  ******************************************/
/****************************************************************************/

/**
 * Set a socket in non blocking mode.
 * @param sd  the socket
 */
void nabto_bsd_set_nonblocking(nabto_socket_t* sd)
{
    int flags = fcntlsocket(*sd, F_GETFL, 0);
    if (flags == -1) flags = 0;
    fcntlsocket(*sd, F_SETFL, flags | O_NDELAY);
}

bool nabto_init_socket(uint16_t* localPort, nabto_socket_t* socket) {
    nabto_socket_t sd = NABTO_INVALID_SOCKET;
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd == -1) {
        NABTO_LOG_ERROR(("Socket creation failed."));
        return false;
    }
    {
        struct sockaddr_in sa;
        int status;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = INADDR_ANY;
        sa.sin_port = htons(*localPort);
        
        status = bind(sd, (struct sockaddr*)&sa, sizeof(sa));
        
        if (status <0) {
            NABTO_LOG_ERROR(("Socket binding failed, %i", status));
            closesocket(sd);
            return false;
        }
        
        nabto_bsd_set_nonblocking(&sd);
        *socket = sd;
    }
    {
        struct sockaddr_in sao;
        int len = sizeof(sao);
        if ( getsockname(*socket, (struct sockaddr*)&sao, &len) != -1) {
            *localPort = htons(sao.sin_port);
        } else {
            NABTO_LOG_ERROR(("Error getting local port"));
        }
    }
    return true;
}


/******************************************************************************/

void nabto_close_socket(nabto_socket_t* socket)
{
    if (socket && *socket != NABTO_INVALID_SOCKET) {
        closesocket(*socket);
        *socket = NABTO_INVALID_SOCKET;
    }
}

/******************************************************************************/

ssize_t nabto_read(nabto_socket_t socket,
                   uint8_t*       buf,
                   size_t         len,
                   struct nabto_ip_address*      addr,
                   uint16_t*      port)
{
    int res;
    struct sockaddr_in sa;
    int salen = sizeof(sa);
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    res = recvfrom(socket, buf, (int)len, 0, (struct sockaddr*)&sa, &salen);
    if (res >= 0) {
#if NABTO_ENABLE_CONNECTION_ESTABLISHMENT_ACL_CHECK
        nabto_endpoint_t ep;
#endif

        addr->type = NABTO_IP_V4;
        addr->addr.ipv4 = ntohl(sa.sin_addr.s_addr);
        *port = ntohs(sa.sin_port);

#if NABTO_ENABLE_CONNECTION_ESTABLISHMENT_ACL_CHECK
        ep.addr.type = NABTO_IP_V4;
        ep.addr.addr.ipv4 = addr->addr.ipv4;
        ep.port = *port;
#endif
//        NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_USER1, ("data from addr: " PRIep, MAKE_EP_PRINTABLE(ep)), buf, res);
    } else if (res == -1) {
        return 0;
    }

    return res;
}

/******************************************************************************/

ssize_t nabto_write(nabto_socket_t socket,
                 const uint8_t* buf,
                 size_t         len,
                 uint32_t       addr,
                 uint16_t       port)
{
    int res;
    struct sockaddr_in sa;
#if NABTO_ENABLE_CONNECTION_ESTABLISHMENT_ACL_CHECK
    nabto_endpoint_t ep;
#endif
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(addr);
    sa.sin_port = htons(port);
#if NABTO_ENABLE_CONNECTION_ESTABLISHMENT_ACL_CHECK
    ep.addr = addr;
    ep.port = port;
#endif
    NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_USER1, ("nabto_write " PRIep, MAKE_EP_PRINTABLE(ep)),buf, len);
    res = sendto(socket, buf, (int)len, 0, (struct sockaddr*)&sa, sizeof(sa));
    if (res < 0) {
        int err = errno;
        NABTO_NOT_USED(err);
        NABTO_LOG_ERROR(("ERROR: %i in nabto_write()", err));
    }
    return res;
}

/******************************************************************************/
/***********************  TIMESTAMPS  *****************************************/
/******************************************************************************/

bool nabtoIsStampPassed(nabto_stamp_t *stamp) {
   return (bool)((nabtoGetStamp() - (*stamp)) > 0);
}

nabto_stamp_t nabtoGetStamp(void) {
    uint32_t sec;
    uint32_t ms;
    nabto_stamp_t token;
    sec = UpTimeSeconds(&ms);
    token = (nabto_stamp_t) (sec*1000 + ms);
    return token;
}
