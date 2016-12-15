#ifndef _TUNNEL_H_
#define _TUNNEL_H_

#include <unabto_platform_types.h>
#include <unabto/unabto_stream.h>
#include <modules/tunnel_common/tunnel_common.h>
#include <modules/tunnel_common/tunnel_tcp.h>

#if defined(WIN32) || defined(WINCE)
// use winsock
#define WINSOCK 1
#endif

bool init_tunnel_module();
void deinit_tunnel_module();

void tunnel_event(tunnel* state, tunnel_event_source event_source);

#endif // _TUNNEL_H_
