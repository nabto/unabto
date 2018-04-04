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
void unabto_psk_connection_handle_request(nabto_socket_t socket, const nabto_endpoint* peer, const nabto_packet_header* header);
void unabtp_psk_connection_handle_exception_request(const nabto_packet_header* header);
void unabto_psk_connection_handle_connect_request();
void unabto_psk_connection_handle_verify_requst();
void unabto_psk_connection_send_connect_response(nabto_connect* connection, );
void unabto_psk_connection_send_verify_response(nabto_connect* connection, );
void unabto_psk_connection_send_connect_error_response(nabto_socket_t socket, nabto_endpoint peer, uint32_t cpNsi, uint32_t spNsi, uint32_t errorCode);

#endif
