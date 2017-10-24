/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NETWORK

/**
 * @file
 * Provides the required socket operations for the uNabto framework on top of Winsock.
 *
 */

#include <winsock2.h>
#include <unabto/unabto_env_base.h>
#include <unabto/unabto_util.h>
#include "unabto_winsock.h"
#include <unabto/unabto_context.h>
#include <unabto/unabto_main_contexts.h>
#include <unabto/unabto_external_environment.h>
#include <unabto/unabto_common_main.h>

#include <modules/network/select/unabto_network_select_api.h>

#include <modules/list/utlist.h>

typedef struct socketListElement {
    nabto_socket_t socket;
    struct socketListElement *prev;
    struct socketListElement *next;
} socketListElement;

static struct socketListElement* socketList = 0;

static void unabto_winsock_shutdown(void);

bool nabto_init_socket(uint32_t localAddr, uint16_t* localPort, nabto_socket_t* sock)
{
    nabto_socket_t sd = INVALID_SOCKET;
    struct sockaddr_in sa;
    socketListElement* se;

    if(!unabto_winsock_initialize())
    {
        return false;
    }

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd == INVALID_SOCKET)
    {
        return false;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(localAddr);
    sa.sin_port = htons(*localPort);
    if (bind(sd, (struct sockaddr*)&sa, sizeof(sa)) < 0)
    {
        NABTO_LOG_ERROR(("bind() fails, the IP-address may be wrong or another process conflicts\n"));
        closesocket(sd);
        return false;
    }

    { // make the socket non-blocking
        u_long flags = 1;
        ioctlsocket(sd, FIONBIO, &flags);
    }
    
    *sock = sd;
    {
        struct sockaddr_in sao;
        int len = sizeof(sao);
        if ( getsockname(*sock, (struct sockaddr*)&sao, &len) != -1)
        {
            *localPort = htons(sao.sin_port);
        }
        else
        {
            NABTO_LOG_ERROR(("error getting local port"));
        }
    }

    se = (socketListElement*)malloc(sizeof(socketListElement));
    
    if (!se) {
        NABTO_LOG_ERROR(("Malloc of a single small list element should not fail!"));
        closesocket(sd);
        return false;
    }

    se->socket = sd;
    DL_APPEND(socketList, se);

    return true;
}

void nabto_close_socket(nabto_socket_t* sock)
{
    if (sock && *sock != INVALID_SOCKET)
    {
        socketListElement* se;
        socketListElement* found = 0;
        DL_FOREACH(socketList,se) {
            if (se->socket == *sock) {
                found = se;
                break;
            }
        }
        if (!found) {
            NABTO_LOG_ERROR(("Socket %i Not found in socket list", *sock));
        } else {
            DL_DELETE(socketList, se);
            free(se);
        }

        closesocket(*sock);
        *sock = INVALID_SOCKET;
    }
}

ssize_t nabto_read(nabto_socket_t sock, uint8_t* buf, size_t len, uint32_t* addr, uint16_t* port)
{
    int res;
    struct sockaddr_in sa;
    int salen = sizeof(sa);
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    res = recvfrom(sock, (char*) buf, (int)len, 0, (struct sockaddr*)&sa, &salen);
    
    if (res >= 0)
    {
        nabto_endpoint ep;
        *addr = ntohl(sa.sin_addr.s_addr);
        *port = ntohs(sa.sin_port);

        ep.addr = *addr;
        ep.port = *port;

        NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_TRACE, ("data from addr: " PRIep, MAKE_EP_PRINTABLE(ep)), buf, res);
    }
    else if (res == -1)
    {
        return 0;
    }
    else
    {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK && err != WSAECONNRESET)
        {
            NABTO_LOG_ERROR(("ERROR: %i in nabto_read()", err));
        }
    }

    return res;
}

ssize_t nabto_write(nabto_socket_t sock, const uint8_t* buf, size_t len, uint32_t addr, uint16_t port)
{
    int res;
    struct sockaddr_in sa;
    nabto_endpoint ep;

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(addr);
    sa.sin_port = htons(port);
    ep.addr = addr;
    ep.port = port;
    NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_TRACE, ("nabto_write " PRIep, MAKE_EP_PRINTABLE(ep)), buf, len);
    res = sendto(sock, (const char*) buf, (int)len, 0, (struct sockaddr*)&sa, sizeof(sa));

    if (res < 0)
    {
        int err = WSAGetLastError();
        NABTO_LOG_ERROR(("ERROR: %i in nabto_write()", err));
    }

    return res;
}

bool unabto_winsock_initialize(void)
{
    static bool initialized = false;

    if (!initialized)
    {
        WORD wVersionRequested;
        WSADATA wsaData;
        int err;

        /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
        wVersionRequested = MAKEWORD(2, 2);

        err = WSAStartup(wVersionRequested, &wsaData);
        if (err != 0)
        {
            /* Tell the user that we could not find a usable */
            /* Winsock DLL.                                  */
        
            NABTO_LOG_ERROR(("WSAStartup failed with error: %d\n", err));
            return false;
        }

        /* Confirm that the WinSock DLL supports 2.2.*/
        /* Note that if the DLL supports versions greater    */
        /* than 2.2 in addition to 2.2, it will still return */
        /* 2.2 in wVersion since that is the version we      */
        /* requested.                                        */

        if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
        {
            /* Tell the user that we could not find a usable */
            /* WinSock DLL.                                  */
            NABTO_LOG_ERROR(("Could not find a usable version of Winsock.dll\n"));
            WSACleanup();
            return false;
        }

        atexit(unabto_winsock_shutdown);

        initialized = true;
    }

    return initialized;
}

void unabto_winsock_shutdown(void)
{
    WSACleanup();
}


void unabto_network_select_add_to_read_fd_set(fd_set* readFds, int* maxReadFd) {
    socketListElement* se;
    DL_FOREACH(socketList, se) {
        FD_SET(se->socket, readFds);
        if (se->socket != INVALID_SOCKET && ((int)se->socket) > *maxReadFd)
           *maxReadFd = se->socket;
    }
}

void unabto_network_select_read_sockets(fd_set* readFds) {
    socketListElement* se;
    DL_FOREACH(socketList, se) {
        if (FD_ISSET(se->socket, readFds)) {
            unabto_read_socket(se->socket);
        }
    }
}

bool nabto_get_local_ip(uint32_t* ip) {
    struct sockaddr_in si_me, si_other;
    int s;
    
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) {
        NABTO_LOG_ERROR(("Cannot create socket"));
        return false;
    }
    
    memset(&si_me, 0, sizeof(si_me));
    memset(&si_other, 0, sizeof(si_me));
    //bind to local port 4567
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(4567);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    //"connect" google's DNS server at 8.8.8.8 , port 4567
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(4567);
    si_other.sin_addr.s_addr = htonl(0x08080808);
    if(connect(s,(struct sockaddr*)&si_other,sizeof si_other) == -1) {
        NABTO_LOG_ERROR(("Cannot connect to host"));
        return false;
    }

    {
        struct sockaddr_in my_addr;
        int len = sizeof my_addr;
        if(getsockname(s,(struct sockaddr*)&my_addr,&len) == -1) {
            NABTO_LOG_ERROR(("getsockname failed"));
            return false;
        }
        *ip = ntohl(my_addr.sin_addr.s_addr);
    }
    closesocket(s);
    return true;
}
