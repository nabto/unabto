/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NETWORK

#include <unabto/unabto_env_base.h>
#include <unabto/unabto_external_environment.h>
#include <modules/network/bsd/unabto_network_bsd.h>
#include <modules/network/poll/unabto_network_poll_api.h>
#include <modules/network/epoll/unabto_epoll.h>
#include <unabto/unabto_context.h>
#include <unabto/unabto_main_contexts.h>
#include <unabto/unabto_common_main.h>

#include <modules/list/utlist.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#ifdef __MACH__
#include <net/if.h>
#else
#include <linux/if.h>
#endif

#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>


#if NABTO_ENABLE_EPOLL
#include <sys/epoll.h>
#endif

#define MAX(a,b) (((a)>(b))?(a):(b))

typedef struct socketListElement {
    nabto_socket_t socket;
    struct socketListElement *prev;
    struct socketListElement *next;
} socketListElement;

static NABTO_THREAD_LOCAL_STORAGE struct socketListElement* socketList = 0;



bool nabto_init_socket(struct nabto_ip_address* localAddr, uint16_t* localPort, nabto_socket_t* sock) {
    
    nabto_socket_t sd;
    nabto_main_context* nmc;
    const char* interface;
    socketListElement* se;
    
    NABTO_LOG_TRACE(("Open socket: ip=%s" ", port=%u", nabto_context_ip_to_string(localAddr), (int)*localPort));
    
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd == -1) {
        NABTO_LOG_ERROR(("Unable to create socket: (%i) '%s'.", errno, strerror(errno)));
        return false;
    }

#ifdef SO_BINDTODEVICE
    // Used for multiple interfaces
    nmc = unabto_get_main_context();
    interface = nmc->nabtoMainSetup.interfaceName;                
    if (interface != NULL) {
        if (setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE, (void *)interface, strlen(interface)+1) < 0) {
            NABTO_LOG_FATAL(("Unable to bind to interface: %s, '%s'.", interface, strerror(errno)));
        }
        NABTO_LOG_TRACE(("Bound to device '%s'.", interface));
    }

    {
        // only reuse when user has explicitly set local port and bound to a specific device as this indicates user
        // knows what he is doing...
        int allowReuse = (localPort && *localPort != 0 && interface != NULL);
        if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (void *) &allowReuse, sizeof(int)))
        {
            NABTO_LOG_FATAL(("Unable to set option: (%i) '%s'.", errno, strerror(errno)));
            return false;
        }
    }
#endif    
    {
        int status;
        if (localAddr->type == NABTO_IP_V4) {
            struct sockaddr_in sa;
            memset(&sa, 0, sizeof(sa));
            sa.sin_family = AF_INET;
            sa.sin_addr.s_addr = htonl(localAddr->addr.ipv4);
            sa.sin_port = htons(*localPort);
            status = bind(sd, (struct sockaddr*)&sa, sizeof(sa));
        } else if (localAddr->type == NABTO_IP_V6) {
            struct sockaddr_in6 sa;
            memset(&sa, 0, sizeof(sa));
            sa.sin6_family = AF_INET6;
            memcpy(sa.sin6_addr.s6_addr, localAddr->addr.ipv6, 16);
            sa.sin6_port = htons(*localPort);
            status = bind(sd, (struct sockaddr*)&sa, sizeof(sa));
            
        } else {
            NABTO_LOG_TRACE(("unsupported ip type not opening socket."));
            return false;
        }
        
        if (status < 0) {
            NABTO_LOG_ERROR(("Unable to bind socket: (%i) '%s' localport %i", errno, strerror(errno), *localPort));
            close(sd);
            return false;
        }
#if UNABTO_NETWORK_BSD_NONBLOCKING
        nabto_bsd_set_nonblocking(&sd);
#endif
        *sock = sd;
    }
    {
        struct sockaddr_in sao;
        socklen_t len = sizeof(sao);
        if ( getsockname(*sock, (struct sockaddr*)&sao, &len) != -1) {
            *localPort = htons(sao.sin_port);
        } else {
            NABTO_LOG_ERROR(("Unable to get local port of socket: (%i) '%s'.", errno, strerror(errno)));
        }
    }
    
    NABTO_LOG_TRACE(("Socket opened: ip=%s" ", port=%u", nabto_context_ip_to_string(localAddr), (int)*localPort));
    
    
    se = (socketListElement*)malloc(sizeof(socketListElement));
    
    if (!se) {
        NABTO_LOG_FATAL(("Malloc of a single small list element should not fail!"));
        close(sd);
        return false;
    }

    se->socket = sd;
    DL_APPEND(socketList, se);

#if NABTO_ENABLE_EPOLL
    {
        unabto_epoll_event_handler_udp* eh = (unabto_epoll_event_handler_udp*) malloc(sizeof(unabto_epoll_event_handler_udp));
        eh->epollEventType = UNABTO_EPOLL_TYPE_UDP;
        eh->fd = sd;
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
        ev.data.ptr = eh;
        if (epoll_ctl(unabto_epoll_fd, EPOLL_CTL_ADD, sd, &ev) == -1) {
            NABTO_LOG_FATAL(("Could not add file descriptor to epoll set, %i: %s", errno, strerror(errno)));
        }
    }
#endif
    return true;
}

void nabto_close_socket(nabto_socket_t* sock) {
    if (sock && *sock != NABTO_INVALID_SOCKET) {
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

#if NABTO_ENABLE_EPOLL
        if (epoll_ctl(unabto_epoll_fd, EPOLL_CTL_DEL, *sock, NULL) == -1) {
            NABTO_LOG_FATAL(("Cannot remove fd from epoll set, %i: %s", errno, strerror(errno)));
        }
#endif
        close(*sock);
        *sock = NABTO_INVALID_SOCKET;

    }
}

ssize_t nabto_read(nabto_socket_t sock,
                   uint8_t*       buf,
                   size_t         len,
                   struct nabto_ip_address*      addr,
                   uint16_t*      port)
{
    struct sockaddr_in6 saData;
    struct sockaddr* sa = (struct sockaddr*)&saData;
    nabto_endpoint ep;
    socklen_t addrlen = sizeof(saData);

    ssize_t recvLength = recvfrom(sock, buf, len, 0, sa, &addrlen);

    if (recvLength < 0 || recvLength == 0) {
        return 0;
    }

    if (sa->sa_family == AF_INET) {
        struct sockaddr_in* sa4 = (struct sockaddr_in*)(sa);
        addr->type = NABTO_IP_V4;
        addr->addr.ipv4 = ntohl(sa4->sin_addr.s_addr);
        *port = ntohs(sa4->sin_port);
    } else if (sa->sa_family == AF_INET6) {
        struct sockaddr_in6* sa6 = (struct sockaddr_in6*)(sa);
        addr->type = NABTO_IP_V6;
        memcpy(addr->addr.ipv6, sa6->sin6_addr.s6_addr, 16);
        *port = ntohs(sa6->sin6_port);
    } else {
        NABTO_LOG_TRACE(("unsupported ip scheme"));
        return 0;
    }

    ep.addr = *addr;
    ep.port = *port;
    NABTO_NOT_USED(ep);
    NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_BUFFERS, ("nabto_read: %s:%u", nabto_context_ip_to_string(addr), *port), buf, recvLength);

    return recvLength;
}


ssize_t nabto_write(nabto_socket_t sock,
                    const uint8_t* buf,
                    size_t         len,
                    const struct nabto_ip_address* addr,
                    uint16_t       port)
{
    int res;
    if (addr->type == NABTO_IP_V4) {
        struct sockaddr_in sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = htonl(addr->addr.ipv4);
        sa.sin_port = htons(port);
        res = sendto(sock, buf, (int)len, 0, (struct sockaddr*)&sa, sizeof(sa));        
    } else if (addr->type == NABTO_IP_V6) {
        struct sockaddr_in6 sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin6_family = AF_INET6;
        memcpy(sa.sin6_addr.s6_addr, addr->addr.ipv6, 16);
        sa.sin6_port = htons(port);
        res = sendto(sock, buf, (int)len, 0, (struct sockaddr*)&sa, sizeof(sa));        
    } else {
        NABTO_LOG_TRACE(("invalid address type"));
        return 0;
    }
    NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_BUFFERS, ("nabto_write size: %" PRIsize ", %s:%u", len, nabto_context_ip_to_string(addr), port) ,buf, len);

    if (res < 0) {
        int status = errno;
        NABTO_NOT_USED(status);
        if (status == EAGAIN || status == EWOULDBLOCK) {
            // expected
        } else {
            NABTO_LOG_ERROR(("ERROR: %s (%i) in nabto_write()", strerror(status), (int) status));
        }
    }
    return res;
}
void unabto_network_select_add_to_read_fd_set(fd_set* readFds, int* maxReadFd) {
    socketListElement* se;
    DL_FOREACH(socketList, se) {
        FD_SET(se->socket, readFds);
        *maxReadFd = MAX(*maxReadFd, se->socket);
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

struct pollfd* unabto_network_poll_add_to_set(struct pollfd* begin, struct pollfd* end)
{
    socketListElement* se;
    DL_FOREACH(socketList, se) {
        if (begin != end) {
            begin->fd = se->socket;
            begin->events = POLLIN;
            begin++;
        }
    }
    return begin;
}

void unabto_network_poll_read_sockets(struct pollfd* begin, struct pollfd* end)
{
    while (begin != end) {
        if (begin->revents == POLLIN) {
            unabto_read_socket(begin->fd);
        }
        begin++;
    }
}

#if NABTO_ENABLE_EPOLL
void unabto_network_epoll_read(struct epoll_event* event)
{
    unabto_epoll_event_handler_udp* udpHandler = (unabto_epoll_event_handler_udp*)event->data.ptr;
    if (udpHandler->epollEventType == UNABTO_EPOLL_TYPE_UDP) {
        bool status;
        do {
            status = unabto_read_socket(udpHandler->fd);
        } while (status);
    }
}

bool unabto_network_epoll_read_one(struct epoll_event* event)
{
    unabto_epoll_event_handler_udp* udpHandler = (unabto_epoll_event_handler_udp*)event->data.ptr;
    if (udpHandler->epollEventType == UNABTO_EPOLL_TYPE_UDP) {
        return unabto_read_socket(udpHandler->fd);
    }
    return false;
}
#endif

bool nabto_get_local_ip(struct nabto_ip_address* ip) {
    struct sockaddr_in si_me, si_other;
    int s;

    // TODO make ipv6 compatible
    
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
    si_other.sin_addr.s_addr = inet_addr("8.8.8.8");
    if(connect(s,(struct sockaddr*)&si_other,sizeof si_other) == -1) {
        NABTO_LOG_ERROR(("Cannot connect to host"));
        return false;
    }

    {
        struct sockaddr_in my_addr;
        socklen_t len = sizeof my_addr;
        if(getsockname(s,(struct sockaddr*)&my_addr,&len) == -1) {
            NABTO_LOG_ERROR(("getsockname failed"));
            return false;
        }
        ip->type = NABTO_IP_V4;
        ip->addr.ipv4 = ntohl(my_addr.sin_addr.s_addr);
    }
    close(s);

    return true;
}
