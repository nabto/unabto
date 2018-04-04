#include "unabto_psk_connection.h"
#include <unabto/unabto_memory.h>


void unabto_psk_connection_handle_request(nabto_socket_t socket, const nabto_endpoint* peer, const nabto_packet_header* header)
{
    if (header->type == NP_PACKET_HDR_TYPE_U_CONNECT_PSK && (header->flags & NP_PACKET_HDR_FLAG_EXCEPTION)) {
        return unabto_psk_connection_handle_exception_request(header);
    } else if (header->type == NP_PACKET_HDR_TYPE_U_CONNECT_PSK) {
        return unabto_psk_connection_handle_connect_request();
    } else if (header->type == NP_PACKET_HDR_TYPE_U_VERIFY_PSK) {
        return unabto_psk_connection_handle_verify_requst();
    }
}

void unabto_psk_connection_handle_exception_request(const nabto_packet_header* header)
{
    // packet structure:
    // U_CONNECT_PSK(Hdr(exception=1), notify(err) )
    uint8_t* begin = unabto_payloads_begin(nabtoCommunicationBuffer, header);
    uint8_t* end = unabto_payloads_end(nabtoCommunicationBuffer, header);
    struct unabto_payload_packet notifyPayload;
    struct unabto_payload_notify notify;
    if (!unabto_find_payload(begin, end, NP_PAYLOAD_TYPE_NOTIFY, &notifyPayload)) {
        NABTO_LOG_WARN(("packet should have contained a notification."));
        return;
    }
    if (!unabto_payload_read_notify(&notifyPayload, &notify)) {
        NABTO_LOG_WARN(("cannot read notify payload"));
        return;
    }

    if (notify.code == NP_PAYLOAD_NOTIFY_ERROR_CONNECTION_ABORTED) {
        nabto_connect* connection;
        connection = nabto_find_connection(header->nsi_sp);
        if (connection) {
            return nabto_connection_client_aborted(connection);
        }
    } else {
        NABTO_LOG_WARN(("unknown notification"));
        return;
    }
}

void unabto_psk_connection_handle_connect_request(nabto_socket_t socket, const nabto_endpoint* peer, const nabto_packet_header* header)
{
    // find a connection. If no connection is found make a new
    // connection. If a connection is found respond with the same
    // packet as the request which created the connection.
}

void unabto_psk_connection_handle_verify_requst()
{
    // find a connection if the state is past the verify phase send
    // the same response as the packet which made the state past the
    // verify phase.
}

void unabto_psk_connection_send_connect_response(nabto_socket_t socket, nabto_endpoint peer, nabto_connect* connection)
{
    // packet format:
    // U_CONNECT_PSK(Hdr, Capabilities, nonce_device, Enc(random_device, nonce_client))
    
    // create header

    // set RSP bit
    
    // insert capabilities

    // insert nonce from device

    // insert encrypted payload header

    // insert random_device

    // insert nonce_client

    // encrypt and send packet.
}

void unabto_psk_connection_send_verify_response(nabto_socket_t socket, nabto_endpoint peer, nabto_connect* connection)
{
    // packet format:
    // U_VERIFY_PSK(Hdr, Notify(ok))

    // create header

    // set rsp bit
    
    // insert notify

    // send packet
}

void unabto_psk_connection_send_verify_error_response(nabto_socket_t socket, nabto_endpoint peer, nabto_connect* connection, uint32_t errorCode)
{
    // packet format:
    // U_VERIFY_PSK(Hdr(exception=1), NOTIFY(err))

    // create header

    // set rsp bit
    // set exception bit

    // insert notify

    // send packet
}


void unabto_psk_connection_send_connect_error_response(nabto_socket_t socket, nabto_endpoint peer, uint32_t cpNsi, uint32_t spNsi, uint32_t errorCode)
{
    // packet format:
    // U_CONNECT_PSK(Hdr(exception=1), NOTIFY(err))

    // create header

    // set RSP bit
    // set exception bit
    // insert notify payload

    // send packet
}

