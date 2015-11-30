/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NETWORK

#include "tcp_provider.h"
#include <unabto/unabto_env_base.h>
#include <unabto/unabto_util.h>

static uint32_t resolve_host_name(const char *id);

void tcp_provider_initialize(void)
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
        
        NABTO_LOG_FATAL(("WSAStartup failed with error: %d\n", err));
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
        NABTO_LOG_FATAL(("Could not find a usable version of Winsock.dll\n"));
    }
}

void tcp_provider_tick(void)
{ }

void tcp_provider_shutdown(void)
{
    WSACleanup();
}

bool tcp_provider_connect(tcp_socket* tcpSocket, text host, uint16_t port)
{
    struct sockaddr_in sa;
    
    *tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(*tcpSocket == TCP_PROVIDER_INVALID_SOCKET)
    {
        NABTO_LOG_ERROR(("Unable to connect to TCP relay service!"));
        return false;
    }
    
    {
        uint32_t hostIp = resolve_host_name(host);
        
        if(hostIp == 0)
        {
            NABTO_LOG_ERROR(("Unable to resolve host!"));
            return false;
        }

        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = htonl(hostIp);
        sa.sin_port = htons(port);
        if(connect(*tcpSocket, (struct sockaddr*)&sa, sizeof(sa)))
        {
            closesocket(*tcpSocket);
            *tcpSocket = TCP_PROVIDER_INVALID_SOCKET;

            NABTO_LOG_ERROR(("Unable to connect to host!"));

            return false;
        }
    }
    
    { // If iMode!=0, non-blocking mode is enabled.
        u_long iMode = 1;
	    ioctlsocket(*tcpSocket, FIONBIO, &iMode);
    }
    
    NABTO_LOG_TRACE(("Connection to TCP relay service established."));

    return true;
}

void tcp_provider_disconnect(tcp_socket* tcpSocket)
{
    closesocket(*tcpSocket);
    *tcpSocket = TCP_PROVIDER_INVALID_SOCKET;

    NABTO_LOG_ERROR(("TCP connection closed."));
}

uint16_t tcp_provider_read(tcp_socket* tcpSocket, uint8_t* buffer, uint16_t maximumLength)
{
    int length = recv(*tcpSocket, (char*)buffer, maximumLength, 0);

    if(length < 0) // an error occurred or there was no data available
    {
        // TODO: Implement error handling.
        //if(WSAGetLastError() != WSAEWOULDBLOCK) // was it an error?
        //{
        //    return 0;
        //}
        return 0;
    }

    return length;
}

void tcp_provider_write(tcp_socket* tcpSocket, uint8_t* buffer, uint16_t length)
{
    send(*tcpSocket, (const char*)buffer, length, 0);
}

static uint32_t resolve_host_name(const char *id)
{
    uint32_t addr = inet_addr(id); // try parsing the id as a dotted IP address

    if (addr == INADDR_NONE) // ...fallback to DNS look up
    {
        struct hostent* he = gethostbyname(id);
        if (he == 0)
        {
            return 0;
        }
        else
        {
          addr = *((uint32_t*)he->h_addr_list[0]);
        }
    }

    return htonl(addr);
}
