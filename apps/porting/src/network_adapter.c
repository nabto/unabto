#include <unabto/unabto_external_environment.h>
#include <unabto/unabto_logging.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>


bool nabto_init_socket(uint16_t* localPort, nabto_socket_t* sock) {
    nabto_socket_t sd;
    
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd == -1) {
        NABTO_LOG_ERROR(("Unable to create socket: (%i) '%s'.", errno, strerror(errno)));
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
        
        if (status < 0) {
            NABTO_LOG_ERROR(("Unable to bind socket: (%i) '%s'.", errno, strerror(errno)));
            close(sd);
            return false;
        }
        int flags = fcntl(sd, F_GETFL, 0);
        if (flags == -1) flags = 0;
        fcntl(sd, F_SETFL, flags | O_NONBLOCK);
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
    
    NABTO_LOG_TRACE(("Socket opened: ip=" PRIip ", port=%u", MAKE_IP_PRINTABLE(localAddr), (int)*localPort));
    
    return true;
}

void nabto_close_socket(nabto_socket_t* sock) {
    if (sock && *sock != NABTO_INVALID_SOCKET) {
        close(*sock);
        *sock = NABTO_INVALID_SOCKET;
    }
}

ssize_t nabto_read(nabto_socket_t sock,
                   uint8_t*       buf,
                   size_t         len,
                   uint32_t*      addr,
                   uint16_t*      port)
{
    int res;
    struct sockaddr_in sa;
    int salen = sizeof(sa);
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    res = recvfrom(sock, (char*) buf, (int)len, 0, (struct sockaddr*)&sa, &salen);
    
    if (res >= 0) {
        *addr = ntohl(sa.sin_addr.s_addr);
        *port = ntohs(sa.sin_port);
    }
    return res;
}

ssize_t nabto_write(nabto_socket_t sock,
                 const uint8_t* buf,
                 size_t         len,
                 uint32_t       addr,
                 uint16_t       port)
{
    int res;
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(addr);
    sa.sin_port = htons(port);
    res = sendto(sock, buf, (int)len, 0, (struct sockaddr*)&sa, sizeof(sa));
    if (res < 0) {
        NABTO_LOG_ERROR(("ERROR: %i in nabto_write()", (int) errno));
    }
    return res;
}
