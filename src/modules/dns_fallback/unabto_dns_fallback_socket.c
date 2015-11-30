#include "unabto_dns_fallback_socket.h"
#include <unabto/unabto_external_environment.h>

#include "errno.h"

bool unabto_dns_fallback_init() {
    return true;
}

bool unabto_dns_fallback_close() {
    return true;
}

static int socketDescriptor;

unabto_dns_fallback_error_code unabto_dns_fallback_create_socket() {

    socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketDescriptor == -1) {
        return UDF_SOCKET_CREATE_FAILED;
    }

    struct sockaddr_in sa;
    int status;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_port = htons(0);
    
    status = bind(socketDescriptor, (struct sockaddr*)&sa, sizeof(sa));
    if (status < 0) {
        NABTO_LOG_ERROR(("Unable to bind socket: (%i) '%s'.", errno, strerror(errno)));
        close(socketDescriptor);
        socketDescriptor = -1;
        return UDF_SOCKET_CREATE_FAILED;
    }

    nabto_bsd_set_nonblocking(&socketDescriptor);
    return UDF_OK;
}

bool unabto_dns_fallback_close_socket() {
    close(socketDescriptor);
    socketDescriptor = -1;
}

size_t unabto_dns_fallback_send_to(uint8_t* buf, size_t bufSize, uint32_t addr, uint16_t port)
{
    int res;
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(addr);
    sa.sin_port = htons(port);
    res = sendto(socketDescriptor, buf, (int)bufSize, 0, (struct sockaddr*)&sa, sizeof(sa));
    if (res < 0) {
        int status = errno;
        NABTO_NOT_USED(status);
        NABTO_LOG_ERROR(("ERROR: %s (%i) in nabto_write()", strerror(status), (int) status));
    }
    return res;
}

size_t unabto_dns_fallback_recv_from(uint8_t* buf, size_t bufferSize, uint32_t* addr, uint16_t* port)
{
    ssize_t res;
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    socklen_t addrlen = sizeof(sa);
    res = recvfrom(socketDescriptor, buf, bufferSize, 0, (struct sockaddr*)&sa, &addrlen);
    if (res > 0) {
        *addr = ntohl(sa.sin_addr.s_addr);
        *port = ntohs(sa.sin_port);
        return res;
    }
    return 0;
}
