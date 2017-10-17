#ifndef _UNABTO_TCP_WINDOWS_H_
#define _UNABTO_TCP_WINDOWS_H_


struct unabto_tcp_socket {
    SOCKET socket;
	struct sockaddr_in host;
};

#endif //_UNABTO_TCP_WINDOWS_H_
