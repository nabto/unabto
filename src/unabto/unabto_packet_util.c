/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
/**
 * @file
 * Nabto Nabto uServer packet utilities - Implementation.
 */

#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_UNABTO

#include "unabto_packet_util.h"

#if NABTO_ENABLE_CONNECTIONS

#include "unabto_util.h"
#include "unabto_logging.h"
#include "unabto_memory.h"
#include <unabto_version.h>

#include <string.h>

/** Magic values */
enum {
    PROTOVERSION = 2 /**< Nabto Protocol Version */
};

/******************************************************************************/

uint16_t nabto_rd_header(const uint8_t* buf, const uint8_t* end, nabto_packet_header* header)
{
    uint16_t hlen = NP_PACKET_HDR_MIN_BYTELENGTH;

    /* Buffer must contain at least minimum header size bytes */
    if (buf + hlen > end)
       return 0;

    /* Read fixed part of packet header */
    READ_U32(header->nsi_cp, buf + 0);
    READ_U32(header->nsi_sp, buf + 4);
    READ_U8(header->type   , buf + 8);
    READ_U8(header->version, buf + 9);
    READ_U8(header->rsvd   , buf + 10);
    READ_U8(header->flags  , buf + 11);
    READ_U16(header->seq   , buf + 12);
    READ_U16(header->len   , buf + 14);

    /* Length field must be the number of bytes in the buffer */
    if (header->len != end - buf) {
        NABTO_LOG_TRACE(("len: %i!=%i", header->len, (int)(end - buf)));
        return 0;
    }

    /* Read NSI.co from packet header (optional) */
    if (header->flags & NP_PACKET_HDR_FLAG_NSI_CO) {
        if (buf + hlen + 8 > end)
            return 0;
        memcpy(header->nsi_co, buf + hlen, 8);
        hlen += 8;
    }
    else
        memset(header->nsi_co, 0, 8);

    /* Read tag from packet header (optional) */
    if (header->flags & NP_PACKET_HDR_FLAG_TAG) {
        if (buf + hlen + 2 > end)
            return 0;
        READ_U16(header->tag, buf + hlen);
        hlen += 2;
    }
    else
        header->tag  = 0;

    header->hlen = hlen;
    return hlen;
} /* uint16_t nabto_rd_header(const uint8_t* buf, const uint8_t* end, nabto_packet_header_t* hdr, uint8_t expType, uint8_t expFlags) */

/******************************************************************************/

uint16_t nabto_rd_payload(const uint8_t* buf, const uint8_t* end, uint8_t* type)
{
    /*uint8_t flags;*/
    uint16_t len;

    if (buf + SIZE_PAYLOAD_HEADER > end) {
        return 0;
    }

    /* Read payload header */
    READ_U8(*type, buf);
    /*READ_U8(flags, buf + 1); * flags ignored */
    READ_U16(len , buf + 2);

    // The length should at least be the header size
    if (len < SIZE_PAYLOAD_HEADER) {
        return 0;
    }
    
    // The length of the payload should be smaller than the buffer.
    if (end - buf < len) {
        return 0;
    }

    return len - SIZE_PAYLOAD_HEADER;
}

bool find_payload(uint8_t* start, uint8_t* end, uint8_t type, uint8_t** payloadStart, uint16_t* payloadLength) {
    uint8_t payloadType;
    uint16_t length;
    do {
        length = nabto_rd_payload(start, end, &payloadType);
        if (length > 0) {
            if (payloadType == type) {
                *payloadLength = length;
                *payloadStart = start;
                return true;
            } else {
                start += length+NP_PAYLOAD_HDR_BYTELENGTH;
            }
        }
    } while (length > 0);
    return false;
}

/******************************************************************************/
/******************************************************************************/

uint8_t* insert_header(uint8_t* buf, uint32_t cpnsi, uint32_t spnsi, uint8_t type, bool rsp, uint16_t seq, uint16_t tag, uint8_t* nsico)
{
    uint8_t flags = NP_PACKET_HDR_FLAG_NONE;
    
    if (rsp)
        flags |= NP_PACKET_HDR_FLAG_RESPONSE;
    if (tag)
        flags |= NP_PACKET_HDR_FLAG_TAG;
    if (nsico)
        flags |= NP_PACKET_HDR_FLAG_NSI_CO;

    /* Write fixed part of packet header */
    WRITE_U32(buf,       cpnsi);   buf += 4; /* NSI.cp              */
    WRITE_U32(buf,       spnsi);   buf += 4; /* NSI.sp              */
    WRITE_U8(buf,         type);   buf += 1; /* type: U_INVITE, ... */
    WRITE_U8(buf, PROTOVERSION);   buf += 1; /* version             */
    WRITE_U8(buf,            0);   buf += 1; /* reserved            */
    WRITE_U8(buf,        flags);   buf += 1; /* flags               */
    WRITE_U16(buf,         seq);   buf += 2; /* seq                 */
  /*WRITE_U16(buf,         len);*/ buf += 2; /*   length to be patched later */

    /* Write controller nsi to the packet header (optional) */
    if (nsico) {
        memcpy(buf, (const void*)nsico, 8);     buf += 8;
    }

    /* Write tag to packet header (optional) */
    if (tag) {
        WRITE_U16(buf,     tag);   buf += 2;
    }

    return buf; /* return end of packet header */
}

/******************************************************************************/

uint8_t* insert_data_header(uint8_t* buf, uint32_t nsi, uint8_t* nsico, uint16_t tag)
{
    return insert_header(buf, 0, nsi, DATA, false, 0, tag, nsico);
}

/******************************************************************************/

uint8_t* insert_payload(uint8_t* buf, uint8_t type, const uint8_t* content, size_t size)
{
    /* Add payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes) */
    WRITE_U8(buf,                     type);                      buf += 1;
    WRITE_U8(buf, NP_PAYLOAD_HDR_FLAG_NONE);                      buf += 1;
    WRITE_U16(buf, (uint16_t)(size + NP_PAYLOAD_HDR_BYTELENGTH)); buf += 2;
    
    /* Add payload data */
    if (content && size) {
        memcpy(buf, content, size);
        buf += size;
    }

    return buf; /* return end of payload */
} /* uint8_t* insert_payload(uint8_t* buf, uint8_t type, const uint8_t* content, size_t size) */

/******************************************************************************/

uint8_t* insert_optional_payload(uint8_t* buf, uint8_t type, const uint8_t* content, size_t size)
{
    uint8_t* ptr = buf + 1; // to set optional bit
    uint8_t* res = insert_payload(buf, type, content, size);
    *ptr |= NP_PAYLOAD_HDR_FLAG_OPTIONAL; /* modify payload header flags */

    return res; /* return end of payload */
} /* uint8_t* insert_optional_payload(uint8_t* buf, uint8_t type, const uint8_t* content, size_t size) */

uint8_t* insert_capabilities(uint8_t* buf, bool cap_encr_off) {
    uint8_t* ptr;
    uint32_t mask;
    uint32_t bits;

    /* Build capabilities for this device */
    mask = PEER_CAP_MICRO |
           PEER_CAP_TAG |
           PEER_CAP_ASYNC |
           PEER_CAP_FB_TCP_U |
           PEER_CAP_UDP |
           PEER_CAP_ENCR_OFF;
    bits = PEER_CAP_MICRO | PEER_CAP_TAG | PEER_CAP_UDP;
#if NABTO_MICRO_CAP_ASYNC
    bits |= PEER_CAP_ASYNC;
#endif
#if NABTO_ENABLE_TCP_FALLBACK
    if (nmc.nabtoMainSetup.enableTcpFallback) {
        bits |= PEER_CAP_FB_TCP_U;
    }
#endif

    if (cap_encr_off) {
        bits |= PEER_CAP_ENCR_OFF;
    }

    /* Write CAPABILITY payload into packet */
    ptr = insert_payload(buf, NP_PAYLOAD_TYPE_CAPABILITY, 0, 9);
    *ptr = 0;             ptr++; /*type*/
    WRITE_U32(ptr, bits); ptr+=4;
    WRITE_U32(ptr, mask); ptr+=4;
    return ptr;
}


uint8_t* insert_ipx_payload(uint8_t* ptr, uint8_t* end) {
    UNABTO_ASSERT(ptr <= end);
    if (end-ptr < NP_PAYLOAD_IPX_BYTELENGTH) {
        return NULL;
    }
    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_IPX, 0, 13);
    
    WRITE_FORWARD_U32(ptr, nmc.socketGSPLocalEndpoint.addr);
    WRITE_FORWARD_U16(ptr, nmc.socketGSPLocalEndpoint.port);
    WRITE_FORWARD_U32(ptr, nmc.context.globalAddress.addr);
    WRITE_FORWARD_U16(ptr, nmc.context.globalAddress.port);
    WRITE_FORWARD_U8(ptr, nmc.context.natType);

    return ptr;
}

uint8_t* insert_version_payload(uint8_t* ptr, uint8_t* end) {
    UNABTO_ASSERT(ptr <= end);
    if (end-ptr < NP_PAYLOAD_VERSION_BYTELENGTH) {
        return NULL;
    }
    
    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_VERSION, 0, 10);


    WRITE_FORWARD_U16(ptr, NP_PAYLOAD_VERSION_TYPE_UD);
    WRITE_FORWARD_U32(ptr, RELEASE_MAJOR);
    WRITE_FORWARD_U32(ptr, RELEASE_MINOR);
    
    return ptr;
}


uint8_t* insert_sp_id_payload(uint8_t* ptr, uint8_t* end) {
    size_t spIdLength = strlen(nmc.nabtoMainSetup.id);

    UNABTO_ASSERT(ptr <= end);
    if ((size_t)(end - ptr) < NP_PAYLOAD_SP_ID_BYTELENGTH + spIdLength) {
        return NULL;
    }

    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_SP_ID, 0, 1+spIdLength);

    WRITE_FORWARD_U8(ptr, NP_PAYLOAD_CP_ID_TYPE_URL);
    memcpy(ptr, nmc.nabtoMainSetup.id, spIdLength);
    
    ptr += spIdLength;

    return ptr;
}

uint8_t* insert_stats_payload(uint8_t* ptr, uint8_t* end, uint8_t stats_event_type) {
    UNABTO_ASSERT(ptr <= end);
    if (end-ptr < NP_PAYLOAD_STATS_BYTELENGTH) {
        return NULL;
    }
    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_STATS, 0, 1);
    WRITE_FORWARD_U8(ptr, stats_event_type);
    return ptr;
}

uint8_t* insert_notify_payload(uint8_t* ptr, uint8_t* end, uint32_t notifyValue) {

    UNABTO_ASSERT(ptr  <= end);
    if (end-ptr < NP_PAYLOAD_NOTIFY_BYTELENGTH) {
        return NULL;
    }
    
    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_NOTIFY, 0, 4);
    WRITE_FORWARD_U32(ptr, notifyValue);
    return ptr;
}

uint8_t* insert_piggy_payload(uint8_t* ptr, uint8_t* end, uint8_t* piggyData, uint16_t piggySize) {
    UNABTO_ASSERT(ptr  <= end);
    if ((uint16_t)(end-ptr) < (NP_PAYLOAD_PIGGY_SIZE_WO_DATA + piggySize)) {
        return NULL;
    }
    
    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_PIGGY, 0, 4 + piggySize);
    WRITE_FORWARD_U32(ptr, 0);
    memcpy(ptr, (const void*) piggyData, piggySize);
    ptr += piggySize;
    
    return ptr;
}



#endif
