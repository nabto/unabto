#ifndef _UNABTO_DEBUG_PACKET_H_
#define _UNABTO_DEBUG_PACKET_H_

#include "unabto_message.h"

#if NABTO_ENABLE_CONNECTIONS
#if NABTO_ENABLE_DEBUG_PACKETS

void unabto_debug_packet(message_event* event, nabto_packet_header* header);

#endif
#endif

#if NABTO_ENABLE_DEBUG_SYSLOG_CONFIG
bool unabto_debug_syslog_config(bool enable, uint8_t facility, uint32_t ip, uint16_t port, uint32_t expire, uint8_t* configStr, uint16_t configStringLength);
#endif

#endif
