#ifndef WIN32
#ifndef _TUNNEL_UART_H_
#define _TUNNEL_UART_H_
#include <modules/tunnel_common/tunnel_common.h>

void uart_forward(tunnel* tunnel);
void unabto_forward_uart(tunnel* tunnel);
bool open_uart(tunnel* tunnel);
bool tunnel_send_init_message(tunnel* tunnel, const char* msg);

#endif // _TUNNEL_UART_H_
#endif
