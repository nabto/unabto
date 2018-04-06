#include <unabto/unabto_psk_connection.h>
#include <unabto/unabto_connection_util.h>
#include <unabto/unabto_memory.h>
#include <unabto/unabto_app.h>
#include <unabto/unabto_crypto.h>
#include <unabto/unabto_packet.h>

bool unabto_local_psk_connection_get_key(const unabto_psk_id keyId, const char* clientId, const unabto_public_key_fingerprint fingerprint, unabto_psk key)
{
    // dummy implementation for now
    memset(key, 0, 16);
    return true;
}


void unabto_psk_connection_handle_request(nabto_socket_t socket, const nabto_endpoint* peer, const nabto_packet_header* header)
{
    if (header->type == NP_PACKET_HDR_TYPE_U_CONNECT_PSK && (header->flags & NP_PACKET_HDR_FLAG_EXCEPTION)) {
        return unabto_psk_connection_handle_exception_request(header);
    } else if (header->type == NP_PACKET_HDR_TYPE_U_CONNECT_PSK) {
        return unabto_psk_connection_handle_connect_request(socket, peer, header);
    } else if (header->type == NP_PACKET_HDR_TYPE_U_VERIFY_PSK) {
        return unabto_psk_connection_handle_verify_requst(socket, peer, header);
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

void unabto_psk_connection_create_new_connection(nabto_socket_t socket, const nabto_endpoint* peer, const nabto_packet_header* header)
{
    nabto_connect* connection;
    connection = nabto_reserve_connection();
    if (!connection) {
        return unabto_psk_connection_send_connect_error_response(socket, peer, header->nsi_cp, header->nsi_sp, NP_PAYLOAD_NOTIFY_ERROR_BUSY_MICRO);
    }

    if (!unabto_psk_connection_create_new_connection_init(socket, peer, header, connection)) {
        nabto_release_connection(connection);
        return;
    }

    // the connection was created now send a packet back to the client.
    unabto_psk_connection_send_connect_response(socket, peer, connection);
}

bool unabto_psk_connection_create_new_connection_init(nabto_socket_t socket, const nabto_endpoint* peer, const nabto_packet_header* header, nabto_connect* connection)
{
    // read client id and insert it into the connection
    unabto_connection_util_read_client_id(header, connection);
    
    // read fingerprint and insert it into the connection
    unabto_connection_util_read_fingerprint(header, connection);

    // read nonce_client and insert it into the connection
    if (!unabto_connection_util_read_nonce_client(header, connection)) {
        return false;
    }

    // read key id and insert it into the connection
    if (!unabto_connection_util_read_key_id(header, connection)) {
        return false;
    }

    // read capabilities and insert them into the connection
    if (!unabto_connection_util_read_capabilities(header, connection)) {
        return false;
    }

    // verify the integrity of the packet if ok accept the connection request else discard the connection
    if (!unabto_psk_connection_util_verify_connect(header, connection)) {
        return false;
    }

    return true;
}

void unabto_psk_connection_handle_connect_request(nabto_socket_t socket, const nabto_endpoint* peer, const nabto_packet_header* header)
{
    // find a connection. If no connection is found make a new
    // connection. If a connection is found respond with the same
    // packet as the request which created the connection.
    nabto_connect* connection;
    // U_CONNECT_PSK pacḱets has an empty sp nsi so we are using the
    // cp nsi which should be unique enough on the local lan. There is
    // a chance for collisions.
    connection = nabto_find_connection_cp_nsi(header->nsi_cp);
    
    if (!connection) {
        return unabto_psk_connection_create_new_connection(socket, peer, header);
    }
    
    if (connection &&
        connection->state == CS_CONNECTING &&
        connection->psk.state == WAIT_CONNECT)
    {
        // there is a connection this is a retransmission the CONNECT
        // response is either lost or still on the line. Resend response.
        return unabto_psk_connection_send_connect_response(socket, peer, connection);
    }
}

void unabto_psk_connection_handle_verify_request(nabto_socket_t socket, const nabto_endpoint* peer, const nabto_packet_header* header)
{
    // Find a connection if the state is past the verify phase send
    // the same response as the packet which made the state past the
    // verify phase.
}

void unabto_psk_connection_send_connect_response(nabto_socket_t socket, const nabto_endpoint* peer, nabto_connect* connection)
{
    // packet format:
    // U_CONNECT_PSK(Hdr, Capabilities, nonce_device, Enc(random_device, nonce_client))

    // create header
    nabto_packet_header header;
    nabto_header_init(&header, NP_PACKET_HDR_TYPE_U_CONNECT_PSK, connection->cpnsi, connection->spnsi);

    // set RSP bit
    nabto_header_add_flags(&header, NP_PACKET_HDR_FLAG_RESPONSE);

    uint8_t* ptr = nabtoCommunicationBuffer;
    uint8_t* end = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;

    ptr = nabto_wr_header(ptr, end, &header);

    // insert capabilities
    ptr = insert_capabilities(ptr, end, false);
    
    // insert nonce from device
    ptr = insert_nonce_payload(ptr, end, connection->psk.handshakeData.responderNonce, 32);

    // pointer before crypto payload
    uint8_t* cryptoPayloadStart = ptr;

    // insert encrypted payload header
    ptr = insert_crypto_payload_with_payloads(ptr, end);

    // start of plaintext
    uint8_t* plaintextStart = ptr;
    
    // insert random_device
    ptr = insert_random_payload(ptr, end, connection->psk.handshakeData.responderRandom, 32);

    // insert nonce_client
    ptr = insert_nonce_payload(ptr, end, connection->psk.handshakeData.initiatorRandom, 32);

    uint8_t* plaintextEnd = ptr;

    uint16_t plaintextLength = plaintextEnd - plaintextStart;
    
    // encrypt and send packet.

    uint16_t packetLength;
    if (encrypt_packet(&connection->cryptoctx, nabtoCommunicationBuffer, end, plaintextStart, plaintextLength, cryptoPayloadStart, &packetLength)) {
        nabto_write(socket, nabtoCommunicationBuffer, packetLength, peer->addr, peer->port);
    } else {
        NABTO_LOG_WARN(("cannot encrypt packet"));
    }
}

void unabto_psk_connection_send_verify_response(nabto_socket_t socket, const nabto_endpoint* peer, nabto_connect* connection)
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


void unabto_psk_connection_send_connect_error_response(nabto_socket_t socket, const nabto_endpoint* peer, uint32_t cpNsi, uint32_t spNsi, uint32_t errorCode)
{
    // packet format:
    // U_CONNECT_PSK(Hdr(exception=1), NOTIFY(err))

    // create header

    // set RSP bit
    // set exception bit
    // insert notify payload

    // send packet
}

