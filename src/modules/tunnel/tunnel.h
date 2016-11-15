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


extern NABTO_THREAD_LOCAL_STORAGE tunnel* tunnels;

/**
 * return false to disallow connections to the specified host:port
 */
bool tunnel_allow_connection(const char* host, int port);

#define DEFAULT_PORT 22
#define DEFAULT_HOST "127.0.0.1"

void tunnel_set_default_host(const char* host);
void tunnel_set_default_port(int port);

const char* tunnel_get_default_host();
int tunnel_get_default_port();

bool init_tunnel_module();
void deinit_tunnel_module();

void reset_tunnel_struct(tunnel* t);

void tunnel_event(tunnel* state, tunnel_event_source event_source);
void close_stream_reader(tunnel* tunnel);

#endif // _TUNNEL_H_
