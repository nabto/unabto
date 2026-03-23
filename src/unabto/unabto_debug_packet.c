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
    uint8_t* end = nabtoCommunicationBuffer + header->len;
    struct unabto_payload_crypto crypto;

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
    const uint8_t* end = payload->dataBegin + payload->dataLength;
    const uint8_t* ptr = payload->dataBegin;

    ptr = read_forward_u8(&flags, ptr, end);
    ptr = read_forward_u8(&facility, ptr, end);
    ptr = read_forward_u16(&port, ptr, end);
    ptr = read_forward_u32(&ip, ptr, end);
    ptr = read_forward_u32(&expire, ptr, end);
    ptr = read_forward_u16(&patternLength, ptr, end);
    if (ptr == NULL) {
        NABTO_LOG_ERROR(("Syslog config packet too short"));
        return false;
    }
    pattern = (uint8_t*)ptr;

    if (ptr + patternLength > end) {
        NABTO_LOG_ERROR(("syslog packet log settings string too long"));
        return false;
    }

    enabled = flags & NP_PAYLOAD_SYSLOG_FLAG_ENABLE;

    return unabto_debug_syslog_config(enabled, facility, ip, port, expire, pattern, patternLength);
#else
    return false;
#endif
}

void send_debug_packet_response(nabto_packet_header* header, uint32_t notification) {
    uint8_t* buf = nabtoCommunicationBuffer;
    uint8_t* ptr = nabtoCommunicationBuffer;
    uint8_t* end = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;

    ptr = insert_header(ptr, end, header->nsi_cp, header->nsi_sp, U_DEBUG, true, header->seq, 0, NULL);

    if (ptr == NULL) {
        NABTO_LOG_ERROR(("Could not insert debug packet header"));
        return;
    }

    ptr = insert_notify_payload(ptr, end, notification);
    if (ptr == NULL) {
        NABTO_LOG_ERROR(("Could not insert notify payload"));
        return;
    }

    {
        uint16_t length;
        if (!insert_packet_length_from_cursor(buf, ptr)) {
            return;
        }
        length = (uint16_t)(ptr - buf);
        send_to_basestation(buf, length, &nmc.context.gsp);
    }
}

#endif
#endif
