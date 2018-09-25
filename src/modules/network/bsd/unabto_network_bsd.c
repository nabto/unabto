/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NETWORK

#include <unabto/unabto_env_base.h>
#include <modules/network/bsd/unabto_network_bsd.h>
#include <modules/network/poll/unabto_network_poll_api.h>
#include <modules/network/epoll/unabto_epoll.h>
#include <unabto/unabto_context.h>
#include <unabto/unabto_message.h>
#include <unabto/unabto_endpoint.h>
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


static bool open_socket(nabto_socket_t* sd)
{
    // try ipv6
    //   test if ipv6 socket can ipv4
    // if not then use ipv4
    // if ipv4 does not work use ipv6 only.
    // IPV6_V6ONLY
    // see nabto4/src/util/udp_socket.hpp

    sd->sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sd->sock == -1) {
        sd->sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sd->sock == -1) {
            NABTO_LOG_ERROR(("Unable to create socket: (%i) '%s'.", errno, strerror(errno)));
            return false;
        }
        sd->type = NABTO_SOCKET_IP_V4;
        return true;
    }
#if defined(IPV6_V6ONLY)
    int no = 0;
    if (setsockopt(sd->sock, IPPROTO_IPV6, IPV6_V6ONLY, (void*) &no, sizeof(no)) != 0) {
        NABTO_LOG_TRACE(("setsocketopt failed to disable IPV6_V6ONLY trying IPv4 socket"));
        close(sd->sock);
        sd->sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sd->sock == -1) {
            NABTO_LOG_TRACE(("IPv4 socket creation failed, going back to IPv6 only"));
            sd->sock = socket(AF_INET6, SOCK_DGRAM, 0);
            if (sd->sock == -1) {
                NABTO_LOG_ERROR(("Failed to recreate IPv6 socket"));
                return false;
            }
            NABTO_LOG_TRACE(("created IPv6 socket"));
            sd->type = NABTO_SOCKET_IP_V6;
            return true;
        } else {
            NABTO_LOG_TRACE(("created IPv4 socket"));
            sd->type = NABTO_SOCKET_IP_V4;
            return true;
        }
    }
#endif
    NABTO_LOG_TRACE(("created IPv6 socket for dual stack"));
    sd->type = NABTO_SOCKET_IP_V6;
    return true;
}


bool nabto_socket_init(uint16_t* localPort, nabto_socket_t* sock)
{
    nabto_socket_t sd;
    nabto_main_context* nmc;
    const char* interface;
    socketListElement* se;
    int status;

    NABTO_LOG_TRACE(("Open socket, port=%u", (int)*localPort));

    if (!open_socket(&sd)) {
        return false;
    }


#ifdef SO_BINDTODEVICE
    // Used for multiple interfaces
    nmc = unabto_get_main_context();
    interface = nmc->nabtoMainSetup.interfaceName;                
    if (interface != NULL) {
        if (setsockopt(sd.sock, SOL_SOCKET, SO_BINDTODEVICE, (void *)interface, strlen(interface)+1) < 0) {
            NABTO_LOG_FATAL(("Unable to bind to interface: %s, '%s'.", interface, strerror(errno)));
        }
        NABTO_LOG_TRACE(("Bound to device '%s'.", interface));
    }

    {
        // only reuse when user has explicitly set local port and bound to a specific device as this indicates user
        // knows what he is doing...
        int allowReuse = (localPort && *localPort != 0 && interface != NULL);
        if (setsockopt(sd.sock, SOL_SOCKET, SO_REUSEADDR, (void *) &allowReuse, sizeof(int)))
        {
            NABTO_LOG_FATAL(("Unable to set option: (%i) '%s'.", errno, strerror(errno)));
            return false;
        }
    }
#endif
    {
        int status;
        
        struct sockaddr_in6 sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin6_family = AF_INET6;
        memset(sa.sin6_addr.s6_addr, 0, 16);
        sa.sin6_port = htons(*localPort);
        status = bind(sd.sock, (struct sockaddr*)&sa, sizeof(sa));
                
        if (status < 0) {
            NABTO_LOG_ERROR(("Unable to bind socket: (%i) '%s' localport %i", errno, strerror(errno), *localPort));
            close(sd.sock);
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
        if ( getsockname(sock->sock, (struct sockaddr*)&sao, &len) != -1) {
            *localPort = htons(sao.sin_port);
        } else {
            NABTO_LOG_ERROR(("Unable to get local port of socket: (%i) '%s'.", errno, strerror(errno)));
        }
    }
    
    NABTO_LOG_TRACE(("Socket opened: port=%u", (int)*localPort));
    
    
    se = (socketListElement*)malloc(sizeof(socketListElement));
    
    if (!se) {
        NABTO_LOG_FATAL(("Malloc of a single small list element should not fail!"));
        close(sd.sock);
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
        if (epoll_ctl(unabto_epoll_fd, EPOLL_CTL_ADD, sd.sock, &ev) == -1) {
            NABTO_LOG_FATAL(("Could not add file descriptor to epoll set, %i: %s", errno, strerror(errno)));
        }
    }
#endif
    return true;
}

bool nabto_socket_is_equal(const nabto_socket_t* s1, const nabto_socket_t* s2) {
    return ((s1->type == s2->type) && (s1->sock == s2->sock));
}

void nabto_socket_set_invalid(nabto_socket_t* sock) {
    sock->sock = NABTO_INVALID_SOCKET;
    sock->type = NABTO_SOCKET_IP_V6;
}

void nabto_socket_close(nabto_socket_t* sock) {
    if (sock && sock->sock != NABTO_INVALID_SOCKET) {
        socketListElement* se;
        socketListElement* found = 0;
        DL_FOREACH(socketList,se) {
            if (se->socket.sock == sock->sock) {
                found = se;
                break;
            }
        }
        if (!found) {
            NABTO_LOG_ERROR(("Socket %i Not found in socket list", sock->sock));
        } else {
            DL_DELETE(socketList, se);
            free(se);
        }

#if NABTO_ENABLE_EPOLL
        if (epoll_ctl(unabto_epoll_fd, EPOLL_CTL_DEL, sock->sock, NULL) == -1) {
            NABTO_LOG_FATAL(("Cannot remove fd from epoll set, %i: %s", errno, strerror(errno)));
        }
#endif
        close(sock->sock);
        sock->sock = NABTO_INVALID_SOCKET;

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

    ssize_t recvLength = recvfrom(sock.sock, buf, len, 0, sa, &addrlen);

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
    NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_BUFFERS, ("nabto_read: %s:%u", nabto_ip_to_string(addr), *port), buf, recvLength);

    return recvLength;
}


ssize_t nabto_write(nabto_socket_t sock,
                    const uint8_t* buf,
                    size_t         len,
                    const struct nabto_ip_address* addr,
                    uint16_t       port)
{
    int res;
    struct nabto_ip_address addrConv;
    if (sock.type == NABTO_SOCKET_IP_V6) {
        struct sockaddr_in6 sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin6_family = AF_INET6;
        if (addr->type == NABTO_IP_V4) {
            nabto_ip_convert_v4_to_v4_mapped(addr, &addrConv);
            memcpy(sa.sin6_addr.s6_addr, addrConv.addr.ipv6, 16);
        } else {
            memcpy(sa.sin6_addr.s6_addr, addr->addr.ipv6, 16);
        }
        sa.sin6_port = htons(port);
        res = sendto(sock.sock, buf, (int)len, 0, (struct sockaddr*)&sa, sizeof(sa));
    } else if (sock.type == NABTO_SOCKET_IP_V4) {
        struct sockaddr_in sa;
        memset(&sa, 0, sizeof(sa));
        if (addr->type == NABTO_IP_V6) {
            if (nabto_ip_is_v4_mapped(addr)) {
                nabto_ip_convert_v4_mapped_to_v4(addr, &addrConv);
                sa.sin_addr.s_addr = htonl(addrConv.addr.ipv4);
            } else {
                NABTO_LOG_ERROR(("ERROR: Tried to write to IPv6 address on IPv4 socket"));
                return 0;
            }
        } else {
            sa.sin_addr.s_addr = htonl(addr->addr.ipv4);
        }
        sa.sin_family = AF_INET;
        sa.sin_port = htons(port);
        res = sendto(sock.sock, buf, (int)len, 0, (struct sockaddr*)&sa, sizeof(sa));
    } else {
        NABTO_LOG_TRACE(("invalid address type"));
        return 0;
    }
    NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_BUFFERS, ("nabto_write size: %" PRIsize ", %s:%u", len, nabto_ip_to_string(addr), port) ,buf, len);

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
        FD_SET(se->socket.sock, readFds);
        *maxReadFd = MAX(*maxReadFd, se->socket.sock);
    }
}

void unabto_network_select_read_sockets(fd_set* readFds) {
    socketListElement* se;
    DL_FOREACH(socketList, se) {
        if (FD_ISSET(se->socket.sock, readFds)) {
            unabto_read_socket(se->socket);
        }
    }
}

struct pollfd* unabto_network_poll_add_to_set(struct pollfd* begin, struct pollfd* end)
{
    socketListElement* se;
    DL_FOREACH(socketList, se) {
        if (begin != end) {
            begin->fd = se->socket.sock;
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
            socketListElement* se;
            DL_FOREACH(socketList, se) {
                if (se->socket.sock == begin->fd) {
                    unabto_read_socket(se->socket);
                }
            }
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

bool nabto_get_local_ipv4(struct nabto_ip_address* ip) {
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
