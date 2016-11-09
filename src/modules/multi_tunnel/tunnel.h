#ifndef _TUNNEL_H_
#define _TUNNEL_H_

#include <unabto_platform_types.h>
#include <unabto/unabto_stream.h>
#include <unabto/unabto_memory.h>
#include <modules/tunnel_common/tunnel_common.h>
#include <modules/tunnel_common/tunnel_tcp.h>
#include <modules/tunnel_common/tunnel_uart.h>

#define UART_COMMAND "uart"
#define TCP_COMMAND "tunnel"
#define ECHO_COMMAND "echo"

extern NABTO_THREAD_LOCAL_STORAGE tunnel* tunnels;

/* Shared functions */
bool init_tunnel_module();
void deinit_tunnel_module();

void reset_tunnel_struct(tunnel* t);
void reset_unknown_tunnel_struct(tunnel* t);

void tunnel_event(tunnel* state, tunnel_event_source event_source);

/* Uart tunnel specific functions */
void uart_tunnel_set_default_device(const char* device);
const char* uart_tunnel_get_default_device();



/* TCP tunnel specific functions */
bool tunnel_allow_connection(const char* host, int port);

#define DEFAULT_PORT 22
#define DEFAULT_HOST "127.0.0.1"

void tunnel_set_default_host(const char* host);
void tunnel_set_default_port(int port);

const char* tunnel_get_default_host();
int tunnel_get_default_port();


#endif // _TUNNEL_H_
