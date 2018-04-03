#ifndef _UNABTO_PSK_CONNECTION_H_
#define _UNABTO_PSK_CONNECTION_H_

/**
 * This file defines the state and functions used by local psk
 * connections.
 *
 * Architecture. All connection packets starts in unabto_connection.c
 * but they are redirected to these functions if they are local psk
 * connection packets.
 */

struct unabto_connection_psk_ctx {
    
};

void unabto_psk_connection_handle_u_connect_psk_request();
void unabto_psk_connection_handle_u_verify_psk_requst();
void unabto_psk_connection_send_u_connect_psk_response();
void unabto_psk_connection_send_u_verify_psk_response();


#endif
