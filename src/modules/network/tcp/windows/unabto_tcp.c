#include <modules/network/tcp/unabto_tcp.h>
#include <modules/network/tcp/unix/unabto_tcp_unix.h>

#include <winsock2.h>
#include <windows.h>
#include <io.h>


unabto_tcp_status unabto_tcp_write(struct unabto_tcp_socket* sock, const void* buf, const size_t len, size_t* written){
    return UTS_OK;
}

unabto_tcp_status unabto_tcp_close(struct unabto_tcp_socket* sock){
    return UTS_OK;
}


unabto_tcp_status unabto_tcp_shutdown(struct unabto_tcp_socket* sock){
    return UTS_OK;
}


unabto_tcp_status unabto_tcp_open(struct unabto_tcp_socket* sockfd){
    return UTS_OK;
}


unabto_tcp_status unabto_tcp_connect(struct unabto_tcp_socket* sock, nabto_endpoint ep){
    return UTS_OK;
}


/* Polls if socket has been connected
 */
unabto_tcp_status unabto_tcp_connect_poll(struct unabto_tcp_socket* sock){
    return UTS_OK;
}
