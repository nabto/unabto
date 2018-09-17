#include <unabto/unabto_psk_connection.h>
#include <unabto/unabto_connection_util.h>
#include <unabto/unabto_memory.h>
#include <unabto/unabto_app.h>
#include <unabto/unabto_crypto.h>
#include <unabto/unabto_packet.h>
#include <unabto/unabto_util.h>

#if NABTO_ENABLE_LOCAL_PSK_CONNECTION

void unabto_psk_connection_dispatch_request(nabto_socket_t socket, const nabto_endpoint* peer, const nabto_packet_header* header)
{
    if (header->type == NP_PACKET_HDR_TYPE_U_CONNECT_PSK && (header->flags & NP_PACKET_HDR_FLAG_EXCEPTION)) {
        return unabto_psk_connection_handle_exception_request(header);
    } else if (header->type == NP_PACKET_HDR_TYPE_U_CONNECT_PSK) {
        return unabto_psk_connection_dispatch_connect_request(socket, peer, header);
    } else if (header->type == NP_PACKET_HDR_TYPE_U_VERIFY_PSK) {
        return unabto_psk_connection_dispatch_verify_request(socket, peer, header);
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

    if (!unabto_psk_connection_handle_connect_request(socket, peer, header, connection)) {
        nabto_release_connection(connection);
        return;
    }
}

bool unabto_psk_connection_handle_connect_request(nabto_socket_t socket, const nabto_endpoint* peer, const nabto_packet_header* header, nabto_connect* connection)
{
    struct unabto_payload_capabilities_read capabilities;
    nabto_reset_connection(connection);
    connection->cpnsi = header->nsi_cp;
    connection->spnsi = nabto_connection_get_fresh_sp_nsi();
    connection->state = CS_CONNECTING;
    connection->psk.state = WAIT_CONNECT;
    connection->timeOut = CONNECTION_TIMEOUT;
    unabto_connection_set_future_stamp(&connection->stamp, 20000);
    connection->rendezvousConnectState.state = RS_DONE;
    connection->noRendezvous = true;
    
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
    if (!unabto_connection_util_read_capabilities(header, &capabilities)) {
        return false;
    }

    // init crypto key for the connecton
    if (!unabto_connection_util_psk_connect_init_key(connection)) {
        NABTO_LOG_WARN(("Failed to initialise crypto context for psk connection"));
        unabto_psk_connection_send_connect_error_response(socket, peer, connection->cpnsi, connection->spnsi, NP_PAYLOAD_NOTIFY_ERROR_BAD_KEY_ID);
        return false;
    }
        
    // verify the integrity of the packet if ok accept the connection request else discard the connection
    if (!unabto_psk_connection_util_verify_connect(header, connection)) {
        return false;
    }

    // verify capabilities read and add the approved subset to our connection.
    if (!unabto_connection_util_verify_capabilities(connection, &capabilities)) {
        unabto_psk_connection_send_connect_error_response(socket, peer, connection->cpnsi, connection->spnsi, NP_PAYLOAD_NOTIFY_ERROR_MISSING_CAPABILITIES);
        return false;
    }
    

    // the connection was created now send a packet back to the client.
    unabto_psk_connection_send_connect_response(socket, peer, connection);
    connection->psk.state = WAIT_VERIFY;
    return true;
}

void unabto_psk_connection_dispatch_connect_request(nabto_socket_t socket, const nabto_endpoint* peer, const nabto_packet_header* header)
{
    // find a connection. If no connection is found make a new
    // connection. If a connection is found respond with the same
    // packet as the request which created the connection.
    nabto_connect* connection;
    // U_CONNECT_PSK pacḱets has an empty sp nsi so we are using the
    // cp nsi which should be unique enough on the local lan. There is
    // a chance for collisions.
    connection = nabto_find_local_connection_cp_nsi(header->nsi_cp);
    
    if (!connection) {
        return unabto_psk_connection_create_new_connection(socket, peer, header);
    }
    
    if (connection &&
        connection->state == CS_CONNECTING &&
        connection->psk.state == WAIT_VERIFY)
    {
        // there is a connection this is a retransmission the CONNECT
        // response is either lost or still on the line. Resend response.
        return unabto_psk_connection_send_connect_response(socket, peer, connection);
    }
}

void unabto_psk_connection_handle_verify_request(nabto_socket_t socket, const nabto_endpoint* peer, const nabto_packet_header* header, nabto_connect* connection)
{
    // packet format U_VERIFY_PSK(Hdr, Enc(random_client, nonce_device))

    struct unabto_payload_crypto cryptoPayload;

    const uint8_t* payloadsBegin = unabto_payloads_begin(nabtoCommunicationBuffer, header);
    const uint8_t* payloadsEnd = unabto_payloads_end(nabtoCommunicationBuffer, header);
    uint8_t* decryptedDataBegin;
    uint16_t decryptedDataLength;

    if (!unabto_payload_find_and_read_crypto(payloadsBegin, payloadsEnd, &cryptoPayload)) {
        NABTO_LOG_WARN(("expected a crypto payload, noone found."));
        return;
    }
    if (!unabto_crypto_verify_and_decrypt(header, &connection->cryptoctx, &cryptoPayload,
                                          &decryptedDataBegin,
                                          &decryptedDataLength))
    {
        NABTO_LOG_WARN(("decryption of packet failed"));
        return;
    }

    {
        // read random and nonce from packet
        const uint8_t* cryptoPayloadsBegin = decryptedDataBegin;
        const uint8_t* cryptoPayloadsEnd = decryptedDataBegin + decryptedDataLength;

        if (!unabto_connection_util_read_random_client(cryptoPayloadsBegin, cryptoPayloadsEnd, connection)) {
            
            return;
        }

        if (!unabto_connection_util_read_and_validate_nonce_client(cryptoPayloadsBegin, cryptoPayloadsEnd, connection)) {
            return;
        }

        // we have all we got to make a new connection.
        unabto_psk_connection_init_connection(connection);
        // send verify ok to client
        unabto_psk_connection_send_verify_response(socket, peer, connection);
    }
}

void unabto_psk_connection_init_connection(nabto_connect* connection)
{
    // init crypto context
    nabto_crypto_init_aes_128_hmac_sha256_psk_context_from_handshake_data(
        &connection->cryptoctx,
        connection->psk.handshakeData.initiatorNonce,
        connection->psk.handshakeData.responderNonce,
        connection->psk.handshakeData.initiatorRandom,
        connection->psk.handshakeData.responderRandom);
    
    // change connection state
    connection->state = CS_CONNECTED;
    
    // change psk state
    connection->psk.state = CONNECTED;

    NABTO_LOG_INFO(("PSK connection created with client nsi: " PRInsi,  MAKE_NSI_PRINTABLE(connection->cpnsi, connection->spnsi, 0) ));
}

void unabto_psk_connection_dispatch_verify_request(nabto_socket_t socket, const nabto_endpoint* peer, const nabto_packet_header* header)
{
    
    // Find a connection if the state is past the verify phase send
    // the same response as the packet which made the state past the
    // verify phase.

    nabto_connect* connection;
    connection = nabto_find_connection(header->nsi_sp);
    if (connection && connection->state == CS_CONNECTING && connection->psk.state == WAIT_VERIFY) {
        // This is a new unhandled packet for the state.
        return unabto_psk_connection_handle_verify_request(socket, peer, header, connection);
    } else  if (connection && connection->state > CS_CONNECTING && connection->psk.state == CONNECTED) {
        // Probably a retransmission since the old response got lost.
        return unabto_psk_connection_send_verify_response(socket, peer, connection);
    }
    
}


void unabto_psk_connection_send_connect_response(nabto_socket_t socket, const nabto_endpoint* peer, nabto_connect* connection)
{
    // packet format:
    // U_CONNECT_PSK(Hdr, Capabilities, nonce_device, Enc(random_device, nonce_client))

    // create header
    nabto_packet_header header;
    uint8_t* ptr = nabtoCommunicationBuffer;
    uint8_t* end;
    uint8_t* cryptoPayloadStart;
    uint8_t* plaintextStart;
    uint8_t* plaintextEnd;
    uint16_t plaintextLength;
    uint16_t packetLength;
    nabto_header_init(&header, NP_PACKET_HDR_TYPE_U_CONNECT_PSK, connection->cpnsi, connection->spnsi);

    // set RSP bit
    nabto_header_add_flags(&header, NP_PACKET_HDR_FLAG_RESPONSE);


    end = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;

    ptr = nabto_wr_header(ptr, end, &header);

    // insert capabilities
    ptr = insert_capabilities_payload(ptr, end, &connection->psk.capabilities, 1);
    WRITE_FORWARD_U16(ptr, CRYPT_W_AES_CBC_HMAC_SHA256);
    
    // insert nonce from device
    ptr = insert_nonce_payload(ptr, end, connection->psk.handshakeData.responderNonce, 32);

    // pointer before crypto payload
    cryptoPayloadStart = ptr;

    // insert encrypted payload header
    ptr = insert_crypto_payload_with_payloads(ptr, end);

    // start of plaintext
    plaintextStart = ptr;
    
    // insert random_device
    ptr = insert_random_payload(ptr, end, connection->psk.handshakeData.responderRandom, 32);

    // insert nonce_client
    ptr = insert_nonce_payload(ptr, end, connection->psk.handshakeData.initiatorNonce, 32);

    plaintextEnd = ptr;

    plaintextLength = plaintextEnd - plaintextStart;
    
    // encrypt and send packet.

    if (encrypt_packet(&connection->cryptoctx, nabtoCommunicationBuffer, end, plaintextStart, plaintextLength, cryptoPayloadStart, &packetLength)) {
        nabto_write(socket, nabtoCommunicationBuffer, packetLength, &peer->addr, peer->port);
    } else {
        NABTO_LOG_WARN(("cannot encrypt packet"));
    }
}

void unabto_psk_connection_send_verify_response(nabto_socket_t socket, const nabto_endpoint* peer, nabto_connect* connection)
{
    // packet format:
    // U_VERIFY_PSK(Hdr, Notify(ok))

    uint8_t* ptr = nabtoCommunicationBuffer;
    uint8_t* end = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;
    
    // create header
   // create header
    nabto_packet_header header;
    uint16_t packetLength;
    nabto_header_init(&header, NP_PACKET_HDR_TYPE_U_VERIFY_PSK, connection->cpnsi, connection->spnsi);

    // set RSP bit
    nabto_header_add_flags(&header, NP_PACKET_HDR_FLAG_RESPONSE);

    ptr = nabto_wr_header(ptr, end, &header);
    
    // insert notify
    ptr = insert_notify_payload(ptr, end, NP_PAYLOAD_NOTIFY_CONNECT_OK);

    if (ptr == NULL) {
        NABTO_LOG_WARN(("packet encoding failed"));
        return;
    }

    packetLength = ptr - nabtoCommunicationBuffer;

    insert_length(nabtoCommunicationBuffer, packetLength);
    // send packet
    nabto_write(socket, nabtoCommunicationBuffer, packetLength, &peer->addr, peer->port);
}

void unabto_psk_connection_send_connect_error_response(nabto_socket_t socket, const nabto_endpoint* peer, uint32_t cpNsi, uint32_t spNsi, uint32_t errorCode)
{
    unabto_psk_connection_send_error_response(socket, peer, cpNsi, spNsi, errorCode, NP_PACKET_HDR_TYPE_U_CONNECT_PSK);
}

void unabto_psk_connection_send_verify_error_response(nabto_socket_t socket, const nabto_endpoint* peer, nabto_connect* connection, uint32_t errorCode)
{
    unabto_psk_connection_send_error_response(socket, peer, connection->cpnsi, connection->spnsi, errorCode, NP_PACKET_HDR_TYPE_U_VERIFY_PSK);
}

void unabto_psk_connection_send_error_response(nabto_socket_t socket, const nabto_endpoint* peer, uint32_t cpNsi, uint32_t spNsi, uint32_t errorCode, uint8_t type)
{
    // packet format <type>(Hdr(Exception = 1, RSP = 1), Notify(errorCode))
    uint8_t* ptr = nabtoCommunicationBuffer;
    uint8_t* end = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;
    
    // create header
    // create header
    nabto_packet_header header;
    uint16_t packetLength;
    nabto_header_init(&header, type, cpNsi, spNsi);

    // set RSP bit
    nabto_header_add_flags(&header, NP_PACKET_HDR_FLAG_RESPONSE);
    nabto_header_add_flags(&header, NP_PACKET_HDR_FLAG_EXCEPTION);

    ptr = nabto_wr_header(ptr, end, &header);
    
    // insert notify
    ptr = insert_notify_payload(ptr, end, errorCode);

    if (ptr == NULL) {
        NABTO_LOG_WARN(("packet encoding failed"));
        return;
    }

    packetLength = ptr - nabtoCommunicationBuffer;

    insert_length(nabtoCommunicationBuffer, packetLength);
    // send packet
    nabto_write(socket, nabtoCommunicationBuffer, packetLength, &peer->addr, peer->port);
}

#endif
