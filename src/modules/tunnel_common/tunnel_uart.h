//#ifndef WIN32
#ifndef _TUNNEL_UART_H_
#define _TUNNEL_UART_H_
#include <modules/tunnel_common/tunnel_common.h>

/* Uart tunnel specific functions */
void uart_tunnel_set_default_device(const char* device);
const char* uart_tunnel_get_default_device();

void unabto_tunnel_uart_steal_port(tunnel* tun, tunnel* tunnels, size_t tunnelsLength);    

void unabto_tunnel_uart_close_stream_reader(tunnel* tunnel);

void unabto_tunnel_uart_parse_command(tunnel* t, tunnel_event_source event_source, tunnel* tunnels, size_t tunnelsLength);
void unabto_tunnel_uart_event(tunnel* tunnel, tunnel_event_source event_source);

void uart_forward(tunnel* tunnel);
void unabto_forward_uart(tunnel* tunnel);
bool open_uart(tunnel* tunnel);


#endif // _TUNNEL_UART_H_
//#endif
