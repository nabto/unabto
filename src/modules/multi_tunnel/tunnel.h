#ifndef _TUNNEL_H_
#define _TUNNEL_H_

#include <unabto_platform_types.h>
#include <unabto/unabto_stream.h>
#include <unabto/unabto_memory.h>
#include <modules/tunnel_common/tunnel_common.h>
#include <modules/tunnel_common/tunnel_tcp.h>
#include <modules/tunnel_common/tunnel_uart.h>
#include <modules/tunnel_common/unabto_tunnel_echo.h>

#define UART_COMMAND "uart"
#define TCP_COMMAND "tunnel"
#define ECHO_COMMAND "echo"

extern NABTO_THREAD_LOCAL_STORAGE tunnel* tunnels;

/* Shared functions */
bool init_tunnel_module();
void deinit_tunnel_module();

void tunnel_event(tunnel* state, tunnel_event_source event_source);

/* TCP tunnel specific functions */
bool tunnel_allow_connection(const char* host, int port);

#endif // _TUNNEL_H_
