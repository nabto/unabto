/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
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
  typedef void (*unabto_packet_data_handler)(nabto_connect* con, nabto_packet_header* hdr, uint8_t* start, uint16_t dlen, uint8_t* payloadsStart, uint8_t* payloadsEnd, void* userData);

typedef struct {
    uint16_t startTag;
    uint16_t endTag;
    unabto_packet_data_handler handler;
    void* userData;
} unabto_packet_data_handler_entry;

bool unabto_packet_set_handler(uint16_t startTag, uint16_t endTag, unabto_packet_data_handler handler, void* userData);

void unabto_packet_init_handlers(void);

void handle_framing_ctrl_packet(nabto_connect* con, nabto_packet_header* hdr, uint8_t* start, uint16_t dlen, uint8_t* payloadsStart, uint8_t* payloadsEnd, void* userData);

void handle_naf_packet(nabto_connect* con, nabto_packet_header* hdr, uint8_t* start, uint16_t dlen, uint8_t* payloadsStart, uint8_t* payloadsEnd, void* userData);

bool send_and_encrypt_packet(nabto_endpoint* peer, nabto_crypto_context* cryptoCtx, uint8_t* plaintextStart, uint16_t plaintextLength, uint8_t* cryptoPayloadDataStart);

bool send_and_encrypt_packet_con(nabto_connect* con, uint8_t* plaintextStart, uint16_t plaintextLength, uint8_t* cryptoPayloadDataStart);

bool send_to_basestation(uint8_t* buffer, size_t buflen, nabto_endpoint* peer);

#ifdef __cplusplus
} // extern "C"
#endif

#endif

#endif
