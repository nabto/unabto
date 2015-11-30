#include "unabto_debug_packet.h"
#include "unabto_memory.h"
#include "unabto_util.h"
#include "unabto_logging.h"
#include "unabto_packet_util.h"
#include "unabto_external_environment.h"
#include "unabto_packet.h"

#if NABTO_ENABLE_CONNECTIONS
#if NABTO_ENABLE_DEBUG_PACKETS

static bool handle_syslog_config(uint8_t* ptr, uint16_t length);
static bool handle_debug_packet(message_event* event, nabto_packet_header* header);
static void send_debug_packet_response(nabto_packet_header* header, uint32_t notification);

void unabto_debug_packet(message_event* event, nabto_packet_header* header) {
    if (!handle_debug_packet(event, header)) {
        send_debug_packet_response(header, NP_PAYLOAD_NOTIFY_DEBUG_ERROR);
    } else {
        send_debug_packet_response(header, NP_PAYLOAD_NOTIFY_DEBUG_OK);
    }
}

bool handle_debug_packet(message_event* event, nabto_packet_header* header) {
    
    uint8_t* buf = nabtoCommunicationBuffer;
    uint8_t* end = nabtoCommunicationBuffer+nabtoCommunicationBufferSize;
    uint8_t* cryptoStart;
    uint16_t cryptoLength;

    buf += header->hlen;

    if (!find_payload(buf, end, NP_PAYLOAD_TYPE_CRYPTO, &cryptoStart, &cryptoLength)) {
        NABTO_LOG_ERROR(("No crypto payload in debug packet."));
        return false;
    }

    if (cryptoLength < 2 + SIZE_PAYLOAD_HEADER) {
        NABTO_LOG_ERROR(("Crypto packet too short."));
        return false;
    }

    {
        uint16_t code;
        uint16_t verifSize;
        READ_U16(code, cryptoStart + SIZE_PAYLOAD_HEADER);
        if (!unabto_verify_integrity(nmc.context.cryptoConnect, code, nabtoCommunicationBuffer, header->len, &verifSize)) {
            NABTO_LOG_DEBUG(("U_DEBUG Integrity verification failed"));
            return false;
        }
    }

    {
        uint8_t* syslogConfigStart;
        uint16_t syslogConfigLength;
        if (!find_payload(buf, end, NP_PAYLOAD_TYPE_SYSLOG_CONFIG, &syslogConfigStart, &syslogConfigLength)) {
            NABTO_LOG_ERROR(("No syslog config packet which is the only one understand at the moment."));
            return false;
        }
        
        if (!handle_syslog_config(syslogConfigStart, syslogConfigLength)) {
            return false;
        }
    }

    return true;
}

bool handle_syslog_config(uint8_t* ptr, uint16_t length) {
#if NABTO_ENABLE_DEBUG_SYSLOG_CONFIG
	uint16_t configStringLength;
	uint8_t flags;
    uint8_t facility;
    uint16_t port;
    uint32_t ip;
    uint32_t expire;
    uint8_t* pattern;
    uint16_t patternLength;
    bool enabled;
    if (length < (NP_PAYLOAD_SYSLOG_CONFIG_SIZE_WO_STRINGS - SIZE_PAYLOAD_HEADER + 2)) {
        NABTO_LOG_ERROR(("Syslog config packet too short"));
        return false;
    }

    ptr += SIZE_PAYLOAD_HEADER;

    configStringLength = length - (NP_PAYLOAD_SYSLOG_CONFIG_SIZE_WO_STRINGS - SIZE_PAYLOAD_HEADER);

    READ_FORWARD_U8(flags, ptr);
    READ_FORWARD_U8(facility, ptr);
    READ_FORWARD_U16(port, ptr);
    READ_FORWARD_U32(ip, ptr);
    READ_FORWARD_U32(expire, ptr);
    READ_FORWARD_U16(patternLength, ptr);
    pattern = ptr;
    
    if (length < (NP_PAYLOAD_SYSLOG_CONFIG_SIZE_WO_STRINGS - SIZE_PAYLOAD_HEADER + 2 + patternLength)) {
        NABTO_LOG_ERROR(("syslog packet log settings string too long"));
        return false;
    }

    enabled = flags & NP_PAYLOAD_SYSLOG_FLAG_ENABLE;

    return unabto_debug_syslog_config(enabled, facility, ip, port, expire, pattern, patternLength);
#else
    return false;
#endif
}

void send_debug_packet_response(nabto_packet_header* header, uint32_t notification) 
{
    uint8_t* buf = nabtoCommunicationBuffer;
    uint8_t* ptr = nabtoCommunicationBuffer;
    uint8_t* end = nabtoCommunicationBuffer+nabtoCommunicationBufferSize;
    
    ptr = insert_header(ptr, header->nsi_cp, header->nsi_sp, U_DEBUG, true, header->seq, 0, NULL);
    
    if (ptr == NULL) {
        NABTO_LOG_ERROR(("Could not insert debug packet header"));
        return;
    }

    ptr = insert_notify_payload(ptr, end, NP_PAYLOAD_NOTIFY_DEBUG_OK);
    if (ptr == NULL) {
        NABTO_LOG_ERROR(("Could not insert notify payload"));
        return;
    }

    {
        uint16_t length = ptr - buf;
        insert_length(buf, length);
        send_to_basestation(buf, length, &nmc.context.gsp);
    }

}

#endif
#endif
