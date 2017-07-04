#include <unabto/unabto_tcp_fallback.h>
#include <unabto/unabto_memory.h>

#if NABTO_ENABLE_TCP_FALLBACK

#define NABTO_TCP_FALLBACK_HANDSHAKE_DELAY 100

bool build_handshake_packet(nabto_connect* con, uint8_t* buffer, size_t bufferLength, size_t* packetLength);

bool unabto_tcp_fallback_time_event(nabto_connect* con) {
    if (con->tcpFallbackConnectionState == UTFS_CONNECTING) {
        nabtoSetFutureStamp(&con->tcpFallbackConnectionStamp, NABTO_TCP_FALLBACK_HANDSHAKE_DELAY);
    } else if (con->tcpFallbackConnectionState == UTFS_CONNECTED) {
        uint8_t handshakePacket[1500];
        size_t handshakePacketLength;
        if (!build_handshake_packet(con, handshakePacket, 1500, &handshakePacketLength)) {
            NABTO_LOG_ERROR((PRI_tcp_fb "Could not create handshake packet.", TCP_FB_ARGS(con)));
            unabto_tcp_fallback_close(con);
            return false;
        }
        
        if (unabto_tcp_fallback_write(con, handshakePacket, handshakePacketLength) != UTFE_OK) {
            NABTO_LOG_ERROR((PRI_tcp_fb "Could not send handshake packet.", TCP_FB_ARGS(con)));
            return false;
        }
        con->tcpFallbackConnectionState = UTFS_HANDSHAKE_SENT;
        return true;
    }
        
    return false;
}

bool build_handshake_packet(nabto_connect* con, uint8_t* buffer, size_t bufferLength, size_t* packetLength) {
    uint8_t* packetPtr;
    uint8_t* bufferEnd = buffer+bufferLength;
    
    /*
     * type,   1 bytes
     * flags,  1 bytes
     * nsi cp, 4 bytes
     * nsi sp, 4 bytes
     * nsi co, 4 bytes
     * id, 20 bytes
     */
    size_t nonceLength = 38;
    uint8_t nonce[38];
    uint8_t* noncePtr = nonce;
    
    packetPtr = insert_header(buffer, con->cpnsi, con->spnsi, NP_PACKET_HDR_TYPE_GW_CONN_U, false, 0, 0, con->consi);

    //NOTE: These type and flags must match the connection attributes set
    //in the unabto_tcp_fallback_connect_thread when a connection is
    //established.
    WRITE_U8(noncePtr, NP_GW_CONN_U_TYPE_TCP); noncePtr+=1;
    WRITE_U8(noncePtr, NP_GW_CONN_U_FLAG_RELIABLE); noncePtr+=1;
    WRITE_U32(noncePtr, con->cpnsi); noncePtr+=4;
    WRITE_U32(noncePtr, con->spnsi); noncePtr+=4;
    memcpy(noncePtr, con->consi, 8); noncePtr+=8;
    memcpy(noncePtr, con->gatewayId, 20);

    packetPtr = insert_payload(packetPtr, bufferEnd, NP_PAYLOAD_TYPE_NONCE, nonce, nonceLength);
    
    *packetLength = packetPtr - buffer;

    insert_length(buffer, *packetLength);
    return true;
}

bool nabto_fallback_connect_u_event(uint16_t packetLength, nabto_packet_header* hdr) {
    nabto_connect* con;
    uint8_t* startOfPayloads;
    uint8_t* endOfPayloads;
    struct unabto_payload_packet nonce;
    uint8_t flags;
    const uint8_t* ptr;
    bool isReliable;
    bool hasKeepAlive;

    con = nabto_find_connection(hdr->nsi_sp);
    if (con == 0) {
        NABTO_LOG_TRACE((PRInsi " No connection found.", MAKE_NSI_PRINTABLE(hdr->nsi_cp, hdr->nsi_sp, 0)));
        return false;
    }

    startOfPayloads = nabtoCommunicationBuffer + hdr->hlen;
    endOfPayloads = nabtoCommunicationBuffer + packetLength;

    if (!unabto_find_payload(startOfPayloads, endOfPayloads, NP_PAYLOAD_TYPE_NONCE, &nonce)) {
        NABTO_LOG_ERROR((PRI_tcp_fb "No nonce payload in GW_CONN_U packet.", TCP_FB_ARGS(con)));
        return false;
    }

    if (nonce.dataLength < 2) {
        NABTO_LOG_ERROR((PRI_tcp_fb "The payload received is too small to contain both flags and type.", TCP_FB_ARGS(con)));
        return false;
    }

    ptr = nonce.dataBegin;

    /*READ_U8(type, ptr);*/  ptr++;
    READ_U8(flags, ptr); ptr++;

    isReliable   = (flags & NP_GW_CONN_U_FLAG_RELIABLE) != 0;
    hasKeepAlive = (flags & NP_GW_CONN_U_FLAG_KEEP_ALIVE) != 0;

    con->tcpFallbackConnectionState = UTFS_READY_FOR_DATA;

    // If the connection is still resolving upgrade it to a fallback connection.
    if (isReliable) {
        con->fbConAttr |= CON_ATTR_NO_RETRANSMIT;
        NABTO_LOG_DEBUG((PRI_tcp_fb "connection needs no retransmition", TCP_FB_ARGS(con)));
    }

    if (!hasKeepAlive) {
        con->fbConAttr |= CON_ATTR_NO_KEEP_ALIVE;
        NABTO_LOG_DEBUG((PRI_tcp_fb "connection needs no keep alive", TCP_FB_ARGS(con)));
    } else {
        NABTO_LOG_DEBUG((PRI_tcp_fb "connection needs keep alive", TCP_FB_ARGS(con)));
    }
    
    return true;
}

void unabto_tcp_fallback_socket_closed(nabto_connect* con) {
    if (con->state == CS_CONNECTED && con->type == NCT_REMOTE_RELAY_MICRO) {
        con->state = CS_CONNECTING;
        con->conAttr = CON_ATTR_DEFAULT;
        if (con->relayIsActive) {
            NABTO_LOG_DEBUG((PRI_tcp_fb "closing full connection as relay was active when closed", TCP_FB_ARGS(con)));
            nabto_release_connection_req(con);
        } else {
            NABTO_LOG_DEBUG((PRI_tcp_fb "leaving connection open as relay was not active when closed", TCP_FB_ARGS(con)));
        }
    }
}

#endif
