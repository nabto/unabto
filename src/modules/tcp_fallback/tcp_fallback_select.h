#ifndef _TCP_FALLBACK_SELECT_H_
#define _TCP_FALLBACK_SELECT_H_

#include <unabto_platform_types.h>

#include <modules/network/epoll/unabto_epoll.h>
#include <modules/network/select/unabto_select.h>

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>

#ifndef WINCE
#include <io.h>
#endif
typedef SOCKET tcp_fallback_socket;
#else
typedef int tcp_fallback_socket;
#endif


typedef struct unabto_tcp_fallback_connection {
    uint8_t sendBuffer[65536];
    size_t  sendBufferLength;
    size_t  sendBufferSent;
    uint8_t recvBuffer[65536];
    size_t  recvBufferLength;
    tcp_fallback_socket socket;
    struct sockaddr_in fbHost;
} unabto_tcp_fallback_connection;

void unabto_tcp_fallback_select_add_to_read_fd_set(fd_set* readFds, int* maxReadFd);
void unabto_tcp_fallback_select_add_to_write_fd_set(fd_set* writeFds, int* maxWriteFd);

void unabto_tcp_fallback_select_read_sockets(fd_set* readFds);
void unabto_tcp_fallback_select_write_sockets(fd_set* writeFds);

#if NABTO_ENABLE_EPOLL
void unabto_tcp_fallback_epoll_event(struct epoll_event* event);
#endif

struct nabto_connect_s;

void unabto_tcp_fallback_read_ready(struct nabto_connect_s* con);
void unabto_tcp_fallback_write_ready(struct nabto_connect_s* con);
#endif
