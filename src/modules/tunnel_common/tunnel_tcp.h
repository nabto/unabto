#ifndef _TUNNEL_TCP_H_
#define _TUNNEL_TCP_H_
#include <modules/tunnel_common/tunnel_common.h>
void tcp_forward(tunnel* tunnel);
void unabto_forward_tcp(tunnel* tunnel);
bool opening_socket(tunnel* tunnel);
bool open_socket(tunnel* tunnel);




#endif // _TUNNEL_TCP_H_
