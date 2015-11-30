#ifndef _UNABTO_DNS_FALLBACK_DATA_H_
#define _UNABTO_DNS_FALLBACK_DATA_H_

#include <unabto_platform_types.h>
#include "unabto_dns_fallback_protocol.h"

void unabto_dns_fallback_data_get_response(unabto_dns_fallback_session* session, unabto_dns_fallback_response_header* header, uint8_t* begin, uint8_t* end);

bool unabto_dns_fallback_data_send_get_request(unabto_dns_fallback_session* session);

void unabto_dns_fallback_data_send_start_response(unabto_dns_fallback_session* session, unabto_dns_fallback_response_header* header);

void unabto_dns_fallback_data_send_chunk_response(unabto_dns_fallback_session* session, unabto_dns_fallback_response_header* header);

bool unabto_dns_fallback_data_send_packet_start_request(unabto_dns_fallback_session* session, uint16_t chunks, nabto_endpoint* ep, uint16_t totalLength);

bool unabto_dns_fallback_data_send_packet_chunk_request(unabto_dns_fallback_session* session, uint8_t* start, uint16_t dataStart, uint16_t length, uint16_t chunkId);

void unabto_dns_fallback_data_send_packet(unabto_dns_fallback_session* session, uint8_t* buffer, uint16_t bufferLength, nabto_endpoint* ep);

    
#endif
