#include "unabto_debug_packet.h"
#include "unabto_memory.h"
#include "unabto_util.h"
#include "unabto_logging.h"
#include "unabto_packet_util.h"
#include "unabto_external_environment.h"
#include "unabto_packet.h"

#if NABTO_ENABLE_CONNECTIONS
#if NABTO_ENABLE_DEBUG_PACKETS

static bool handle_syslog_config(struct unabto_payload_packet* payload);
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
    struct unabto_payload_crypto crypto;
    uint8_t* cryptoStart;
    uint16_t cryptoLength;

    buf += header->hlen;

    {
        struct unabto_payload_packet payload;
        if (!unabto_find_payload(buf, end, NP_PAYLOAD_TYPE_CRYPTO, &payload)) {
            NABTO_LOG_ERROR(("No crypto payload in debug packet."));
            return false;
        }

        if (!unabto_payload_read_crypto(&payload, &crypto)) {
            NABTO_LOG_ERROR(("Crypto packet too short."));
            return false;
        }
    }
    {
        uint16_t verifSize;
        if (!unabto_verify_integrity(nmc.context.cryptoConnect, crypto.code, nabtoCommunicationBuffer, header->len, &verifSize)) {
            NABTO_LOG_DEBUG(("U_DEBUG Integrity verification failed"));
            return false;
        }
    }

    {
        struct unabto_payload_packet payload;
        if (!unabto_find_payload(buf, end, NP_PAYLOAD_TYPE_SYSLOG_CONFIG, &payload)) {
            NABTO_LOG_ERROR(("No syslog config packet which is the only one understand at the moment."));
            return false;
        }
        
        if (!handle_syslog_config(&payload)) {
            return false;
        }
    }

    return true;
}

bool handle_syslog_config(struct unabto_payload_packet* payload) {
#if NABTO_ENABLE_DEBUG_SYSLOG_CONFIG
    uint8_t flags;
    uint8_t facility;
    uint16_t port;
    uint32_t ip;
    uint32_t expire;
    uint8_t* pattern;
    uint16_t patternLength;
    bool enabled;
    const uint8_t* ptr;
    if (payload->length < (NP_PAYLOAD_SYSLOG_CONFIG_SIZE_WO_STRINGS + 2)) {
        NABTO_LOG_ERROR(("Syslog config packet too short"));
        return false;
    }

    ptr = payload->dataBegin;

    READ_FORWARD_U8(flags, ptr);
    READ_FORWARD_U8(facility, ptr);
    READ_FORWARD_U16(port, ptr);
    READ_FORWARD_U32(ip, ptr);
    READ_FORWARD_U32(expire, ptr);
    READ_FORWARD_U16(patternLength, ptr);
    pattern = (uint8_t*)ptr;
    
    if (payload->length < (NP_PAYLOAD_SYSLOG_CONFIG_SIZE_WO_STRINGS + 2 + patternLength)) {
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
