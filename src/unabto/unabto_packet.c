/*
 * Copyright (C) Nabto - All Rights Reserved.
 */

/**
 * @file
 * Nabto uServer common protocol - Implementation.
 */

#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_UNABTO

#include <unabto/unabto_env_base.h>

#if NABTO_ENABLE_CONNECTIONS

#include "unabto_packet.h"
#include "unabto_main_contexts.h"
#include "unabto_connection.h"
#include "unabto_crypto.h"
#include "unabto_stream.h"
#include "unabto_app_adapter.h"
#include "unabto_logging.h"
#include "unabto_util.h"
#include "unabto_packet_util.h"
#include "unabto_memory.h"
#include "unabto_external_environment.h"

#include <unabto/unabto_tcp_fallback.h>

#include <string.h>
#include <stdlib.h>

#define MAX_HANDLERS 5
static NABTO_THREAD_LOCAL_STORAGE unabto_packet_data_handler_entry handlers[MAX_HANDLERS];


void unabto_packet_init_handlers(void) {
    memset(handlers, 0, sizeof(handlers));
}

bool unabto_packet_set_handler(uint16_t startTag, uint16_t endTag, unabto_packet_data_handler handler, void* userData) {
    int i;
    for (i = 0; i < MAX_HANDLERS; i++) {
        if (handlers[i].handler == 0) {
            handlers[i].startTag = startTag;
            handlers[i].endTag = endTag;
            handlers[i].handler = handler;
            handlers[i].userData = userData;
            return true;
        }
    }
    return false;
}

unabto_packet_data_handler_entry* unabto_packet_find_handler(uint16_t tag) {
    int i;
    for (i = 0; i < MAX_HANDLERS; i++) {
        if (handlers[i].startTag <= tag && handlers[i].endTag >= tag) {
            return &handlers[i];
        }
    }
    return 0;
}


/**
 * When a data packet is received we have several ways of handling
 * it. But all data packets has a crypto payload which we need to
 * verify and decrypt.
 *
 * So we iterate through the payloads, find the crypto payload, checks
 * the integrity and decrypts the data. If this succeedes we call a
 * function which handles the data based on the tag type.
 */

void nabto_packet_event(message_event* event, nabto_packet_header* hdr)
{
    uint8_t*                   firstPayload;
    uint16_t                   vlen;
    nabto_connect*             con;
    uint8_t*                   start;
    uint16_t                   expectedCode;
    uint16_t                   ilen = hdr->len;
    struct unabto_payload_packet cryptoPayload;
    uint16_t                   dlen;
    unabto_packet_data_handler_entry* handler;


    /* remove compiler warning about unused variable since the variable is only used 
     * if trace output is enabled
     */
    NABTO_NOT_USED(expectedCode);

    /* assert: hdr.len >= hdr.hlen */

    NABTO_LOG_TRACE(("received hdr with tag: %i", hdr->tag));

    con = nabto_find_connection(hdr->nsi_sp);
    if (con == 0) {
        NABTO_LOG_TRACE((PRInsi " No connection found.", MAKE_NSI_PRINTABLE(hdr->nsi_cp, hdr->nsi_sp, 0)));
        return;
    }

#if NABTO_ENABLE_TCP_FALLBACK
    if (event->type == MT_TCP_FALLBACK) {
        if (con->tcpFallbackConnectionState != UTFS_READY_FOR_DATA) {
            NABTO_LOG_ERROR((PRInsi " Fallback data on a connection which is not in data state", MAKE_NSI_PRINTABLE(hdr->nsi_cp, hdr->nsi_sp, 0)));
        }
    }
#endif
    
    expectedCode = con->cryptoctx.code;

    firstPayload = nabtoCommunicationBuffer + hdr->hlen;

    if (!unabto_find_payload(firstPayload,nabtoCommunicationBuffer +ilen, NP_PAYLOAD_TYPE_CRYPTO, &cryptoPayload)) {
        NABTO_LOG_ERROR(("Packet without crypto payload"));
        return;
    }

    {
        struct unabto_payload_crypto crypto;
        if (!unabto_payload_read_crypto(&cryptoPayload, &crypto)) {
            NABTO_LOG_ERROR(("cannot read crypto payload."));
            return;
        }

        if (!unabto_verify_integrity(&con->cryptoctx, crypto.code, nabtoCommunicationBuffer, ilen, &vlen)) {
            NABTO_LOG_TRACE((PRInsi " Failing integrity check", MAKE_NSI_PRINTABLE(0, hdr->nsi_sp, 0)));
            return;
        }

        if (crypto.dataLength < vlen) {
            NABTO_LOG_TRACE((PRInsi " Message too short for decryption: %i, %i", MAKE_NSI_PRINTABLE(0, hdr->nsi_sp, 0), crypto.dataLength, vlen));
            return;
        }
        start = (uint8_t*)crypto.dataBegin;
        if (!unabto_decrypt(&con->cryptoctx, start, (uint16_t)crypto.dataLength - vlen, &dlen)) {
            NABTO_LOG_TRACE((PRInsi " Error decrypting message: %i, %i", MAKE_NSI_PRINTABLE(0, hdr->nsi_sp, 0), crypto.dataLength, vlen));
            return;
        }
    }
    
    /* The packet is verified to come from the one we are
     * communicating with on this connections.
     * so update, the connection accordingly. */
    
    nabto_connection_event(con, event);

    con->stats.packetsReceived++;
    con->stats.bytesReceived += dlen;

    // Now the decrypted data is at start, the length of the decrypted data is dlen.

    handler = unabto_packet_find_handler(hdr->tag);

    if (handler == 0) {
        NABTO_LOG_ERROR(("no handler for data with tag %i", hdr->tag));
    } else {
        handler->handler(con, hdr, start, (uint16_t)dlen, nabtoCommunicationBuffer+hdr->hlen, nabtoCommunicationBuffer+ilen, event, handler->userData);
    }
}

/*****************************************************
 * TODO: move the rest of this file to other modules *
 *****************************************************/

/**
 * Send an ACK notification to the client.
 * @param con   the connection
 * @param hdr   the received header
 *
 * The ACK is sent as a packet with a NOTIFICATION.
 */
static void send_ack(nabto_connect* con, nabto_packet_header* hdr)
{
    uint8_t buf[SIZE_HEADER_MAX + NP_PAYLOAD_NOTIFY_BYTELENGTH];
    uint8_t* end = buf + SIZE_HEADER_MAX + NP_PAYLOAD_NOTIFY_BYTELENGTH;
    uint16_t hlen  = hdr->hlen;
    uint8_t* ptr = buf;
    
    memcpy(buf, (const void*) nabtoCommunicationBuffer, hlen);
    ptr += hlen;

    ptr = insert_notify_payload(ptr, end, NOTIFY_MICRO_ACK);
    
    insert_length(buf, ptr-buf);

    add_flags(buf, NP_PACKET_HDR_FLAG_RESPONSE); // | NP_PACKET_HDR_FLAG_EXCEPTION);
    NABTO_LOG_TRACE(("(." PRInsi ".) send Dialogue-Ack to client, seq: %" PRIu16, MAKE_NSI_PRINTABLE(0, hdr->nsi_sp, 0), hdr->seq));
    
    nabto_write_con(con, buf, ptr-buf);
}

/******************************************************************************/

bool send_exception(nabto_connect* con, nabto_packet_header* hdr, uint32_t aer)
{
    uint8_t  buf[SIZE_HEADER_MAX + SIZE_PAYLOAD_HEADER + SIZE_CODE + 48]; // maximum length cryptosuites implemented as of aug 2012
    uint8_t* end = buf + sizeof(buf);
    uint8_t* dataStart;
    uint8_t* cryptoPayloadStart;
    uint8_t* ptr = buf;
    
    ptr = nabto_wr_header(buf, end, hdr);
    
    add_flags(buf, NP_PACKET_HDR_FLAG_RESPONSE);
    add_flags(buf, NP_PACKET_HDR_FLAG_EXCEPTION);
    
    cryptoPayloadStart = ptr;
    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_CRYPTO, 0, 0);
    ptr += SIZE_CODE; // make room for crypto suite code
    dataStart = ptr;
    // write exception value
    WRITE_FORWARD_U32(ptr, (uint32_t)aer);
    
    return send_and_encrypt_packet_con(con, buf, end, dataStart, sizeof(uint32_t), cryptoPayloadStart);
}

void handle_framing_ctrl_packet(nabto_connect* con, nabto_packet_header* hdr, uint8_t* dataStart, uint16_t dlen, uint8_t* payloadsStart, uint8_t* payloadsEnd, message_event* event, void* userData) {
    uint16_t olen = 0;
    uint8_t* packetStart = nabtoCommunicationBuffer;
    uint8_t* packetBufferEnd = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;
    enum {
        FRAMING_KEEP_ALIVE = 0, // Framing::CMD_KEEPALIVE
        FRAMING_CLOSED_OLD = 2, // Framing::CMD_UDT_CLOSED
        FRAMING_CLOSED     = 3  // Framing::CMD_UDP_CLOSED
    };
    
    (void)payloadsStart; (void)payloadsEnd; (void)userData; /* Unused */
    
    if (dlen < 4) {
        NABTO_LOG_TRACE((PRInsi " FRAMING CTRL short dlen=%i", MAKE_NSI_PRINTABLE(0, hdr->nsi_sp, 0), dlen));
        return;
    } else {
        uint32_t cmd;
        READ_U32(cmd, dataStart);
        switch (cmd) {
            case FRAMING_KEEP_ALIVE:
            {
                uint32_t no;
                READ_U32(no, dataStart + 4l);
                NABTO_NOT_USED(no); /* Only used for logging */
                NABTO_LOG_TRACE(("packet_event: spnsi=%" PRIu32, con->spnsi));
                NABTO_LOG_DEBUG((PRInsi " FRAMING CTRL, KEEPALIVE %" PRIu32, MAKE_NSI_PRINTABLE(0, hdr->nsi_sp, 0), no));
                nabtoSetFutureStamp(&con->stamp, con->timeOut);
                /* Keep alive, respond by echoing 8 bytes */
                olen = 8;
            }
            break;
            case FRAMING_CLOSED_OLD:
            case FRAMING_CLOSED:
                WRITE_U32(dataStart, (uint32_t)FRAMING_CLOSED);
                
#if NABTO_ENABLE_TCP_FALLBACK
                // Giant hack drop the udt closed packet if it comes
                // from a fallback connection

                if (con->type == NCT_REMOTE_RELAY_MICRO) {
                    NABTO_LOG_DEBUG((PRInsi " Received FRAMING CTRL, CLOSE, but discards it since we are using a fallback connection.", MAKE_NSI_PRINTABLE(hdr->nsi_cp, hdr->nsi_sp, 0)));
                    return;
                }
#endif                
                NABTO_LOG_DEBUG((PRInsi " FRAMING CTRL, CLOSE", MAKE_NSI_PRINTABLE(0, hdr->nsi_sp, 0)));

                nabto_release_connection_req(con);
                olen = 4;
                break;
            default:
                NABTO_LOG_DEBUG((PRInsi " FRAMING CTRL, UNKNOWN", MAKE_NSI_PRINTABLE(0, hdr->nsi_sp, 0)));
                return;
        }
    }
    
    // If we are here we need to send a response to the sender
    hdr->flags |= NP_PACKET_HDR_FLAG_RESPONSE;
    insert_flags(packetStart, hdr->flags);
    
    send_and_encrypt_packet_con(con, packetStart, packetBufferEnd, dataStart, olen, dataStart-SIZE_CODE-NP_PAYLOAD_HDR_BYTELENGTH);
}


void handle_naf_packet(nabto_connect* con, nabto_packet_header* hdr, uint8_t* start, uint16_t dlen, uint8_t* payloadsStart, uint8_t* payloadsEnd, message_event* event, void* userData)
{
    struct naf_handle_s* handle = NULL;
    
    naf_query_status nqs;
    (void)payloadsStart; (void)payloadsEnd; (void)userData; /* Unused */

#if NABTO_ENABLE_TCP_FALLBACK
    if (event->type == MT_TCP_FALLBACK) {
        con->relayIsActive = 1;
    }
#endif

    nabtoSetFutureStamp(&con->stamp, con->timeOut);
    
    nqs = framework_event_query(con, hdr, &handle);
    switch (nqs) {
        case NAF_QUERY_NEW:
            if (con->cpAsync) {
                // send ack such that the client knows that we are processing the message
                send_ack(con, hdr);
            }
            framework_event(handle, start, dlen);
            break;
        case NAF_QUERY_QUEUED:
            NABTO_LOG_TRACE((PRInsi " The Application has previously queued the request %" PRIu16, MAKE_NSI_PRINTABLE(0, hdr->nsi_sp, 0), hdr->seq));
            if (con->cpAsync) {
                nabtoSetFutureStamp(&con->stamp, con->timeOut);
                // send ack such that the client knows that the message is being processed.
                send_ack(con, hdr);
            }
            break;

        case NAF_QUERY_OUT_OF_RESOURCES:
            if (con->cpAsync) {
                send_exception(con, hdr,  NP_E_SYSTEM_ERROR);
            }
            break;
    }
}

/**
 * Encrypt a packet
 * @param cryptoCtx crypto context
 * @param bufferStart start of packet buffer
 * @param bufferEnd   end of packet buffer
 * @param plaintextStart  start of plaintext
 * @param plaintextLength length of plaintext
 * @param cryptoPayloadStart start of crypto payload before payload header and crypto code.
 * @param len  length of encrypted packet
 * @return true iff the packet was encrypted.
 */
bool encrypt_packet(nabto_crypto_context* cryptoCtx, uint8_t* packetStart, uint8_t* packetEnd, uint8_t* plaintextStart, uint16_t plaintextLength, uint8_t* cryptoPayloadStart, uint16_t* len) {
    // Encrypt the data and insert the length and encryption code into
    // the packet
    uint8_t* cryptoPayloadEnd;
    if (!unabto_encrypt(cryptoCtx, plaintextStart, plaintextLength, cryptoPayloadStart+4+SIZE_CODE, packetEnd, &cryptoPayloadEnd)) {
        NABTO_LOG_ERROR(("failed to encrypt data"));
        return false;
    }
    
    {
        *len = (uint16_t)(cryptoPayloadEnd - packetStart);
        insert_length(packetStart, *len);
        if (!unabto_insert_integrity(cryptoCtx, packetStart, *len)) {
            NABTO_LOG_ERROR(("Integrity insertion failing"));
            return false;
        }
        NABTO_LOG_TRACE(("Integrity inserted"));
    }
    return true;
}


bool send_and_encrypt_packet_con(nabto_connect* con, uint8_t* packetStart, uint8_t* packetEnd, uint8_t* plaintextStart, uint16_t plaintextLength, uint8_t* cryptoPayloadStart) {
    uint16_t len;

    if (!encrypt_packet(&con->cryptoctx, packetStart, packetEnd, plaintextStart, plaintextLength, cryptoPayloadStart, &len)) {
        return false;
    }

    return nabto_write_con(con, packetStart, len);
}
/**
 * Send a packet where the header etc is setup correctly but the data
 * needs to be encrypted and an integrity has to be added.
 */
bool send_and_encrypt_packet(nabto_endpoint* peer, nabto_crypto_context* cryptoCtx, uint8_t* packetStart, uint8_t* packetEnd, uint8_t* plaintextStart, uint16_t plaintextLength, uint8_t* cryptoPayloadStart) {
    uint16_t len;

    if (!encrypt_packet(cryptoCtx, packetStart, packetEnd, plaintextStart, plaintextLength, cryptoPayloadStart, &len)) {
        return false;
    }

    return send_to_basestation(packetStart, len, peer);
}

bool send_to_basestation(uint8_t* buffer, size_t buflen, nabto_endpoint* peer)
{
    if (peer->addr.type != NABTO_IP_NONE && peer->port != 0) {
        return (nabto_write(nmc.socketGSP, buffer, buflen, &peer->addr, peer->port) > 0);
    } else {
        return false;
    }
}

uint8_t* unabto_stats_write_u32(uint8_t* ptr, uint8_t* end, uint8_t type, uint32_t value)
{
    if (ptr == NULL) {
        return NULL;
    }
    if (end - ptr < sizeof(uint32_t) + 2) {
        return NULL;
    }
    WRITE_FORWARD_U8(ptr, type);
    WRITE_FORWARD_U8(ptr, 6);
    WRITE_FORWARD_U32(ptr, value);
    return ptr;
}

uint8_t* unabto_stats_write_u16(uint8_t* ptr, uint8_t* end, uint8_t type, uint16_t value)
{
    if (ptr == NULL) {
        return ptr;
    }
    if (end - ptr < sizeof(uint16_t) + 2) {
        return NULL;
    }
    WRITE_FORWARD_U8(ptr, type);
    WRITE_FORWARD_U8(ptr, 4);
    WRITE_FORWARD_U16(ptr, value);
    return ptr;
}

uint8_t* unabto_stats_write_u8(uint8_t* ptr, uint8_t* end, uint8_t type, uint8_t value)
{
    if (ptr == NULL) {
        return NULL;
    }
    if (end - ptr < sizeof(uint8_t) + 2) {
        return NULL;
    }
    WRITE_FORWARD_U8(ptr, type);
    WRITE_FORWARD_U8(ptr, 3);
    WRITE_FORWARD_U8(ptr, value);
    return ptr;
}

#endif
