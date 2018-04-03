/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/**
 * @file
 * Nabto uServer packet event - Interface.
 *
 * Handling data packets.
 */

#ifndef _UNABTO_PACKET_H_
#define _UNABTO_PACKET_H_

#include <unabto/unabto_env_base.h>

#if NABTO_ENABLE_CONNECTIONS

#include <unabto/unabto_context.h>
#include <unabto/unabto_crypto.h>
#include <unabto/unabto_packet_util.h>
#include <unabto/unabto_connection.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Handle a Nabto Packet.
 * @param peer       the endpoint of the sender
 * @param fromGSP    true if packet local unencrypted request
 * @param hdr        the decoded packet header
 */
void nabto_packet_event(message_event* event, nabto_packet_header* hdr);

/**
 * Callback to register a data handler.
 * @param  con    The connection which the data originates.
 * @param  hdr    The packet hdr.
 * @param  peer   The packet src endpoint.
 * @param  start  Start of decrypted data.
 * @param  dlen   Length of the decrypted data.
 * @param  payloadsStart  Start of payloads including the crypto payload.
 * @param  payloadsEnd    End of payloads including the crypto payload.
 */
typedef void (*unabto_packet_data_handler)(nabto_connect* con, nabto_packet_header* hdr, uint8_t* start, uint16_t dlen, uint8_t* payloadsStart, uint8_t* payloadsEnd, message_event* event, void* userData);

typedef struct {
    uint16_t startTag;
    uint16_t endTag;
    unabto_packet_data_handler handler;
    void* userData;
} unabto_packet_data_handler_entry;

bool unabto_packet_set_handler(uint16_t startTag, uint16_t endTag, unabto_packet_data_handler handler, void* userData);

void unabto_packet_init_handlers(void);

void handle_framing_ctrl_packet(nabto_connect* con, nabto_packet_header* hdr, uint8_t* start, uint16_t dlen, uint8_t* payloadsStart, uint8_t* payloadsEnd, message_event* event, void* userData);

void handle_naf_packet(nabto_connect* con, nabto_packet_header* hdr, uint8_t* start, uint16_t dlen, uint8_t* payloadsStart, uint8_t* payloadsEnd, message_event* event, void* userData);

/**
 * @param peer peer to send to
 * @param cryptoCtx  crypto context
 * @param packetStart  start of packet buffer
 * @param packetEnd  end of packet buffer
 * @param plaintextStart start of plaintext data to be encrypted.
 * @param plaintextLength length of plaintext data to be encrypted.
 * @param cryptoPayloadStart start of the crypto payload, the first byte in the payload.
 * @return true iff the packet was sent.
 */
bool send_and_encrypt_packet(nabto_endpoint* peer, nabto_crypto_context* cryptoCtx, uint8_t* packetStart, uint8_t* packetEnd, uint8_t* plaintextStart, uint16_t plaintextLength, uint8_t* cryptoPayloadStart);

/**
 * Send an exception notification to the client.
 * @param ctx   the context
 * @param con   the connection
 * @param hdr   the received header
 * @param aer   the exception
 *
 * The exception is sent in an encrypted (DATA) packet with the EXCEPTION flag set.
 */
bool send_exception(nabto_connect* con, nabto_packet_header* hdr, uint32_t aer);

    
/**
 * send and encrypt a packet on a connection.
 * @param con                     The connection.
 * @param packetBufferStart       The start of the packet.
 * @param packetBufferEnd         The end of the packet buffer.
 * @param plaintextStart          Start of data to be encrypted. 
 * @param plaintextLength         Length of data to be encrypted.
 * @param cryptoPayloadStart      Start of the crypto payload, this is the first byte in crypto payload.
 * @return true iff the packet was sent.
 * 
 * The plain text can both be inside the current buffer or a buffer
 * outside.
 */
bool send_and_encrypt_packet_con(nabto_connect* con,
                                 uint8_t* packetBufferStart,
                                 uint8_t* packetBufferEnd,
                                 uint8_t* plaintextStart,
                                 uint16_t plaintextLength,
                                 uint8_t* cryptoPayloadStart);

bool send_to_basestation(uint8_t* buffer, size_t buflen, nabto_endpoint* peer);

uint8_t* unabto_stats_write_u32(uint8_t* ptr, uint8_t* end, uint8_t type, uint32_t value);
uint8_t* unabto_stats_write_u16(uint8_t* ptr, uint8_t* end, uint8_t type, uint16_t value);
uint8_t* unabto_stats_write_u8(uint8_t* ptr, uint8_t* end, uint8_t type, uint8_t value);


#ifdef __cplusplus
} // extern "C"
#endif

#endif

#endif
