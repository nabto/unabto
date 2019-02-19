/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/**
 * @file
 * Nabto uServer common protocol - Implementation.
 */

#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_UNABTO

#include <string.h>
#include "unabto_env_base.h"
#include "unabto_message.h"
#include "unabto_connection.h"
#include "unabto_attach.h"
#include "unabto_push.h"
#include "unabto_app.h"
#include "unabto_app_adapter.h"
#include "unabto_packet.h"
#include "unabto_util.h"
#include "unabto_logging.h"
#include "unabto_debug_packet.h"
#include "unabto_memory.h"
#include "unabto_external_environment.h"
#include <unabto/unabto_tcp_fallback.h>
#include <unabto/unabto_psk_connection.h>

/*
 * Except the UATTACH protocol, this is the packet/message formats:
 *
 * Request:
 * ========
 * word  content
 * ----+-------
 * 0     header (version, RSP, reserved, mainOpCode)
 * 1     sequence idunabto_message.c
 * 2     nabto field
 * -- Applications:
 * 3     request number
 * 4..   parameters
 * -- Discovery:
 * 3..   device name (zero term)
 *
 * Response:
 * =========
 * word  content
 * ----+-------
 * 0     header (same as in request with RSP set)
 * 1     sequence id (same as in request)
 * 2     nabto field
 * 3..   response values
 *
 */


#if NABTO_ENABLE_LOCAL_ACCESS
static void nabto_message_local_discovery_event(uint16_t ilen, nabto_endpoint* peer);
#endif
#if NABTO_ENABLE_LOCAL_ACCESS_LEGACY_PROTOCOL
static void nabto_message_local_legacy_application_event(uint16_t ilen, nabto_endpoint* peer);
#endif

#if NABTO_ENABLE_LOCAL_ACCESS
/**
 * When room available, insert a string preceeded by a type field (1 byte), truncate when necesary.
 * @param buf   start of writing area
 * @param end   end of writing area (after)
 * @param type  the type
 * @param str   the string
 * @return      the number of bytes written
 */
static size_t add_typed_string(uint8_t* buf, uint8_t* end, uint8_t type, const char* str) {
    size_t avail = end - buf;
    const char dummy[2] = { '-', 0 };
    if (str == 0) {
        str = dummy;
    }
    if (avail > 1) {
        // need room for at least type and zeroterm
        size_t len = strlen(str) + 1;
        size_t tmp = (len < avail) ? len : avail - 1; // 1 byte used for type
        *buf = type;
        memcpy(buf + 1, str, tmp);
        return 1 + tmp; // add 1 is for 'type'
    }
    return 0;
}
#endif

#if NABTO_ENABLE_LOCAL_ACCESS
void nabto_message_local_event(message_event* event, uint16_t ilen) {
    uint32_t localHdr;
 
    READ_U32(localHdr, nabtoCommunicationBuffer);

    NABTO_LOG_TRACE(("localHdr: %" PRIu32 " (%" PRItext ")", localHdr, ((localHdr < NP_PACKET_HDR_MIN_NSI_CP) ? "legacy opcode" : "client nsi")));

    if (localHdr < NP_PACKET_HDR_MIN_NSI_CP) {
        // old fashioned local packet.
        // this is either a discovery or an legacy application event.
        uint8_t mainOpcode = (uint8_t)localHdr;
        switch (mainOpcode) {
            case NP_LEGACY_PACKET_HDR_TYPE_DISCOVERY:
            {
                nabto_message_local_discovery_event(ilen, &event->udpMessage.peer);
                break;
            }
            case NP_LEGACY_PACKET_HDR_TYPE_APPLICATION:
            {
#if NABTO_ENABLE_LOCAL_ACCESS_LEGACY_PROTOCOL
                nabto_message_local_legacy_application_event(ilen, &event->udpMessage.peer);
#endif
                break;
            }
            default:
            {
                NABTO_LOG_ERROR(("Unknown legacy opcode: 0x%" PRIu8, mainOpcode));
                return;
            }
        }
    } else {
#if NABTO_ENABLE_LOCAL_CONNECTION
        // remote event or local nabto connection
        nabto_message_event(event, ilen);
#endif
    }
}
#endif

/**
 * The legacy header has the following structure.
 * <legacy_header> ::= <hdr> <seq> <nabto>
 * <hdr> ::= <uint32>
 * <seq> ::= <uint32>
 * <nabto> ::= <uint32>
 *
 * This is 12 bytes in total.
 */

#if NABTO_ENABLE_LOCAL_ACCESS
void nabto_message_local_discovery_event(uint16_t ilen, nabto_endpoint* peer) {
    uint8_t* buf = nabtoCommunicationBuffer;
    uint16_t bufsize = nabtoCommunicationBufferSize;
    uint16_t olen;
    uint32_t header;
    READ_U32(header, buf);

    NABTO_NOT_USED(ilen);

    NABTO_LOG_TRACE(("local_discover_event"));

    NABTO_LOG_TRACE(("discover: %s", (char*)buf + NP_LEGACY_PACKET_HDR_SIZE));

    if (strcmp(nmc.nabtoMainSetup.id, (const char*) (buf + NP_LEGACY_PACKET_HDR_SIZE)) == 0 || textcmp("*", (const char*) (buf + NP_LEGACY_PACKET_HDR_SIZE)) == 0) {
        uint8_t* ptr = buf + NP_LEGACY_PACKET_HDR_SIZE;
        uint8_t* end = buf + bufsize;
        uint32_t localIp = 0;
        
        WRITE_U32(ptr, localIp);   ptr += 4;
        WRITE_U16(ptr, nmc.nabtoMainSetup.localPort); ptr += 2;
        
        // cheating a bit using the utility meant for typed strings - same structure
        ptr += add_typed_string(ptr, end, 1 /*identifies a micro device */, nmc.nabtoMainSetup.id);
        ptr += add_typed_string(ptr, end, NP_PAYLOAD_DESCR_TYPE_VERSION, nmc.nabtoMainSetup.version);
        ptr += add_typed_string(ptr, end, NP_PAYLOAD_DESCR_TYPE_URL, nmc.nabtoMainSetup.url);
#if NABTO_ENABLE_LOCAL_CONNECTION
        {
            // Send this capability if the device handles local connections.
            char capOk[2] = {'1', 0};
            ptr += add_typed_string(ptr, end, NP_PAYLOAD_DESCR_TYPE_LOCAL_CONN, capOk);
        }
#endif

# if NABTO_ENABLE_LOCAL_PSK_CONNECTION
        {
            // Send this capability if the device handles local psk connections.
            char capOk[2] = {'1', 0};
            ptr += add_typed_string(ptr, end, NP_PAYLOAD_DESCR_TYPE_LOCAL_CONN_PSK, capOk);
        }
#endif

#if NABTO_ENABLE_LOCAL_CONNECTION || NABTO_ENABLE_LOCAL_PSK_CONNECTION
        {
            // local connections and local psk connections has
            // fingerprint capabilities in unabto these days.
            char capOk[2] = {'1', 0};
            ptr += add_typed_string(ptr, end, NP_PAYLOAD_DESCR_TYPE_FP, capOk);
        }
#endif

        olen = (uint16_t)(ptr - buf); // HSIZE is added again before returning
        header |= NP_LEGACY_PACKET_HDR_FLAG_RSP;
        WRITE_U32(buf, header);
        NABTO_LOG_TRACE(("local_discover_event -> sending rsp olen=%" PRIu16, olen));
        nabto_write(nmc.socketLocal, nabtoCommunicationBuffer, olen, &peer->addr, peer->port);
    } else {
        NABTO_LOG_TRACE(("Discover target: '%s' this device: '%s'", buf+sizeof(header), nmc.nabtoMainSetup.id));
    }
}
#endif

#if NABTO_ENABLE_LOCAL_ACCESS_LEGACY_PROTOCOL
void nabto_message_local_legacy_application_event(uint16_t ilen, nabto_endpoint* peer) {
    unabto_query_request br;
    unabto_query_response bw;
    unabto_buffer brb;
    unabto_buffer bwb;

    application_request appreq;
    uint8_t* buf = nabtoCommunicationBuffer;
    uint8_t* ptr = buf + NP_LEGACY_PACKET_HDR_SIZE;
    uint32_t header;
    uint16_t olen;

    if (ilen < 12) {
        // invalid header
        return;
    }
    
    READ_U32(header, buf);

    READ_U32(appreq.queryId, ptr);
    appreq.clientId = NULL; // the local legacy protocol does not support client ids.
    appreq.connection = NULL;
    appreq.isLegacy = true;
    appreq.isLocal = true;

    unabto_buffer_init(&brb, ptr + 4, ilen - NP_LEGACY_PACKET_HDR_SIZE - 4);
    unabto_query_request_init(&br, &brb);

    unabto_buffer_init(&bwb, ptr, nabtoCommunicationBufferSize - NP_LEGACY_PACKET_HDR_SIZE);
    unabto_query_response_init(&bw, &bwb);

    if (application_event(&appreq, &br, &bw) != AER_REQ_RESPONSE_READY) {
        header |= NP_LEGACY_PACKET_HDR_FLAG_ERR;
    }
    olen = NP_LEGACY_PACKET_HDR_SIZE + unabto_query_response_used(&bw);
    header |= NP_LEGACY_PACKET_HDR_FLAG_RSP;
    WRITE_U32(buf, header);

    nabto_write(nmc.socketLocal, nabtoCommunicationBuffer, olen, &peer->addr, peer->port);
}
#endif

#if NABTO_ENABLE_CONNECTIONS
void nabto_message_event(message_event* event, uint16_t ilen) {
    nabto_packet_header hdr;

    if (nabto_rd_header(nabtoCommunicationBuffer, nabtoCommunicationBuffer + ilen, &hdr) == 0) {
        NABTO_LOG_DEBUG(("Invalid request message length: %" PRIu16, ilen));
        return;
    }

    if (hdr.len > ilen) {
        NABTO_LOG_DEBUG(("Packet to small to contain the full packet."));
        return;
    }

    if (event->type == MT_UDP) {
        bool fromBS = nabto_ep_is_equal(&event->udpMessage.peer, &nmc.controllerEp);
        bool fromGSP = nabto_ep_is_equal(&event->udpMessage.peer, &nmc.context.gsp);
        if (fromBS) {
            NABTO_LOG_TRACE(("Received from Base Station: %" PRIu16 " bytes", ilen));
        } else if (fromGSP) {
            NABTO_LOG_TRACE(("Received from GSP: %" PRIu16 " bytes", ilen));
        } else {
            NABTO_LOG_TRACE(("Received packet from Client, Controller or GSP: %" PRIu16 " bytes", ilen));
        }

        switch (hdr.type) {
#if NABTO_ENABLE_REMOTE_ACCESS
        case U_INVITE:
            if (nabto_invite_event(&hdr, &event->udpMessage.peer)) return;
            break;
        case U_ATTACH:
            if (nabto_attach_event(&hdr)) return;
            NABTO_LOG_TRACE(("failed to handle U_ATTACH from GSP"));
            break;
#if NABTO_ENABLE_PUSH
        case U_PUSH:
            if (fromGSP && nabto_push_event(&hdr)) return;
            break;
#endif
        case U_ALIVE:
            if (fromGSP && nabto_alive_event(&hdr)) return;
            {
                return;
            }
            break;
#endif
        case U_CONNECT:
            if (!fromGSP) {
                nabto_connect_event(event, &hdr);
                return;
            } else {
#if NABTO_ENABLE_REMOTE_ACCESS
                nabto_connect_event_from_gsp(event, &hdr);
                return;
#endif                
            }
#if NABTO_ENABLE_LOCAL_PSK_CONNECTION
        case U_CONNECT_PSK:
        case U_VERIFY_PSK:
            unabto_psk_connection_dispatch_request(event->udpMessage.socket, &event->udpMessage.peer, &hdr);
            return;
#endif            

#if NABTO_ENABLE_DEBUG_PACKETS

        case U_DEBUG:
            if (fromGSP) {
                unabto_debug_packet(event, &hdr);
                return;
            }
            break;
#endif

        }
    }

#if NABTO_ENABLE_TCP_FALLBACK
    if (event->type == MT_TCP_FALLBACK) {
        switch (hdr.type) {
        case GW_CONN_U:
            nabto_fallback_connect_u_event(ilen, &hdr);
            return;
        }
    }
#endif
            
    if (hdr.type == DATA) {
        nabto_packet_event(event, &hdr);
        return;
    }

    NABTO_LOG_TRACE(("Ignore message type %i in state %i (event type %i)", (int)hdr.type, (int)nmc.context.state, (int)event->type));
}
#endif

#if NABTO_ENABLE_CONNECTIONS
bool nabto_message_async_response_poll(void) {
    bool             result = false;

    while (framework_event_poll()) {
        result = true;
    }

    return result;
}
#endif
