#ifndef _TUNNEL_TCP_H_
#define _TUNNEL_TCP_H_
#include <modules/tunnel_common/tunnel_common.h>
void tcp_forward(tunnel* tunnel);
void unabto_forward_tcp(tunnel* tunnel);
bool opening_socket(tunnel* tunnel);
bool open_socket(tunnel* tunnel);

bool unabto_tunnel_tcp_parse_command(tunnel* tunnel);

void unabto_tunnel_tcp_set_default_host(const char* host);
void unabto_tunnel_tcp_set_default_port(uint16_t port);

const char* unabto_tunnel_tcp_get_default_host();
uint16_t    unabto_tunnel_tcp_get_default_port();

#ifndef UNABTO_TUNNEL_TCP_DEFAULT_PORT
#define UNABTO_TUNNEL_TCP_DEFAULT_PORT 22
#endif

#ifndef UNABTO_TUNNEL_TCP_DEFAULT_HOST
#define UNABTO_TUNNEL_TCP_DEFAULT_HOST "127.0.0.1"
#endif

#endif // _TUNNEL_TCP_H_
