#include <modules/network/tcp/unabto_tcp.h>
#include <modules/network/tcp/windows/unabto_tcp_windows.h>
#include <modules/network/winsock/unabto_winsock.h>

#include <winsock2.h>
#include <windows.h>
#include <io.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#define MSG_NOSIGNAL 0

enum unabto_tcp_status unabto_tcp_read(struct unabto_tcp_socket* sock, void* buf, const size_t len, size_t* read) {
    int status;

	status = recv(sock->socket, buf, len, 0);
    if (status < 0) {
        if (WSAGetLastError() == WSAEWOULDBLOCK) {
            return UTS_WOULD_BLOCK;
        } else {
            NABTO_LOG_ERROR(("unabto_tcp_read failed"));
            return UTS_FAILED;
        }
    } else if (status == 0) {
        NABTO_LOG_INFO(("TCP connection closed by peer"));
        unabto_tcp_close(sock);
        return UTS_FAILED;
    } else {
        NABTO_LOG_INFO(("buf: %s", (char*)buf));
        *read = status;
        return UTS_OK;
    }

    return UTS_OK;
}

enum unabto_tcp_status unabto_tcp_write(struct unabto_tcp_socket* sock, const void* buf, const size_t len, size_t* written) {
    int status;
    NABTO_LOG_TRACE(("Writing %i bytes to tcp socket", len));
    status = send(sock->socket, (const char*)buf, len, MSG_NOSIGNAL);
    NABTO_LOG_TRACE(("tcp send status: %i", status));
    if (status > 0) {
        *written = status;
        return UTS_OK;
    } else if (status < 0) {
        if (WSAGetLastError() == WSAEWOULDBLOCK) {
            return UTS_WOULD_BLOCK;
        } else {
            NABTO_LOG_ERROR(("Send of tcp packet failed"));
            unabto_tcp_close(sock);
            return UTS_FAILED; 
        }
    }
    return UTS_OK;
}

enum unabto_tcp_status unabto_tcp_close(struct unabto_tcp_socket* sock){
    if (sock->socket == INVALID_SOCKET) {
        NABTO_LOG_ERROR(("trying to close invalid socket"));
    } else {
        closesocket(sock->socket);
        sock->socket = INVALID_SOCKET;
    }
    return UTS_OK;
}


enum unabto_tcp_status unabto_tcp_shutdown(struct unabto_tcp_socket* sock){
    shutdown(sock->socket, SD_BOTH);
    return UTS_OK;
}


enum unabto_tcp_status unabto_tcp_open(struct unabto_tcp_socket* sock){
	int flags = 1;

    if(!unabto_winsock_initialize()){
        NABTO_LOG_ERROR(("unabto_winsock_initialize failed"));
        return UTS_FAILED;
    }
    sock->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(sock->socket, IPPROTO_TCP, TCP_NODELAY, (char *) &flags, sizeof(int)) != 0) {
        NABTO_LOG_ERROR(("Could not set socket option TCP_NODELAY with error: %d", WSAGetLastError()));
        return UTS_FAILED;
    }
    return UTS_OK;
}


enum unabto_tcp_status unabto_tcp_connect(struct unabto_tcp_socket* sock, nabto_endpoint *ep){
    int status;
    memset(&sock->host,0,sizeof(struct sockaddr_in));
    sock->host.sin_family = AF_INET;
    sock->host.sin_addr.s_addr = htonl(ep->addr);
    sock->host.sin_port = htons(ep->port);
    NABTO_LOG_INFO(("Connecting to %d.%d.%d.%d:%d ", MAKE_EP_PRINTABLE(*ep)));
    
    status = connect(sock->socket, (struct sockaddr*)&sock->host, sizeof(struct sockaddr_in));
   
    if (status == 0) {
        return UTS_OK;
    } else {
        if (WSAGetLastError() == WSAEINPROGRESS) {
             return UTS_CONNECTING;
        } else {
            NABTO_LOG_ERROR(("Could not connect to tcp endpoint. %s", strerror(errno)));
            unabto_tcp_close(sock);
            return UTS_FAILED;
        }
    }
}


/* Polls if socket has been connected.
 * Windows does not have async connect,
 * and this function should never be used
 */
enum unabto_tcp_status unabto_tcp_connect_poll(struct unabto_tcp_socket* sock){
    return UTS_FAILED;
}
