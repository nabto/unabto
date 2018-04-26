#ifndef _UNABTO_PSK_CONNECTION_H_
#define _UNABTO_PSK_CONNECTION_H_

#include <unabto/unabto_env_base.h>
#include <unabto/unabto_context.h>
#include <unabto/unabto_connection.h>

/**
 * This file defines the state and functions used by local psk
 * connections.
 *
 * Architecture. All connection packets starts in unabto_connection.c
 * but they are redirected to these functions if they are local psk
 * connection packets.
 */

void unabto_psk_connection_dispatch_request(nabto_socket_t socket, const nabto_endpoint* peer, const nabto_packet_header* header);
void unabto_psk_connection_handle_exception_request(const nabto_packet_header* header);

// Create a new connection and free it again if the initial packet cannot be correctly parsed.
void unabto_psk_connection_create_new_connection(nabto_socket_t socket, const nabto_endpoint* peer, const nabto_packet_header* header);

// Initialise a connection based on the first packet received.
bool unabto_psk_connection_handle_connect_request(nabto_socket_t socket, const nabto_endpoint* peer, const nabto_packet_header* header, nabto_connect* connection);

void unabto_psk_connection_dispatch_connect_request(nabto_socket_t socket, const nabto_endpoint* peer, const nabto_packet_header* header);
void unabto_psk_connection_handle_verify_request(nabto_socket_t socket, const nabto_endpoint* peer, const nabto_packet_header* header, nabto_connect* connection);
void unabto_psk_connection_dispatch_verify_request(nabto_socket_t socket, const nabto_endpoint* peer, const nabto_packet_header* header);
void unabto_psk_connection_send_connect_response(nabto_socket_t socket, const nabto_endpoint* peer, nabto_connect* connection);
void unabto_psk_connection_send_verify_response(nabto_socket_t socket, const nabto_endpoint* peer, nabto_connect* connection);
void unabto_psk_connection_send_connect_error_response(nabto_socket_t socket, const nabto_endpoint* peer, uint32_t cpNsi, uint32_t spNsi, uint32_t errorCode);

void unabto_psk_connection_send_verify_error_response(nabto_socket_t socket, const nabto_endpoint* peer, nabto_connect* connection, uint32_t errorCode);

void unabto_psk_connection_send_error_response(nabto_socket_t socket, const nabto_endpoint* peer, uint32_t cpNsi, uint32_t spNsi, uint32_t errorCode, uint8_t type);

void unabto_psk_connection_init_connection(nabto_connect* connection);

#endif
