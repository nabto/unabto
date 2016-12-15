#ifndef _UNABTO_TUNNEL_ECHO_
#define _UNABTO_TUNNEL_ECHO_

#include <modules/tunnel/unabto_tunnel_common.h>

void unabto_tunnel_echo_forward(tunnel* tunnel);
void unabto_tunnel_echo_parse_command(tunnel* tunnel, tunnel_event_source event_source);

void unabto_tunnel_echo_event(tunnel* tunnel, tunnel_event_source event_source);

#endif
