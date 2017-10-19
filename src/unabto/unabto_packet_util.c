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
    if (buf + hlen > end) {
       return 0;
    }

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
        if (buf + hlen + 8 > end) {
            return 0;
        }
        memcpy(header->nsi_co, buf + hlen, 8);
        hlen += 8;
    }
    else
        memset(header->nsi_co, 0, 8);

    /* Read tag from packet header (optional) */
    if (header->flags & NP_PACKET_HDR_FLAG_TAG) {
        if (buf + hlen + 2 > end) {
            return 0;
        }
        READ_U16(header->tag, buf + hlen);
        hlen += 2;
    }
    else
        header->tag  = 0;

    header->hlen = hlen;
    return hlen;
}

uint8_t* nabto_wr_header(uint8_t* buf, const uint8_t* end, const nabto_packet_header* hdr)
{
    uint8_t* ptr = buf;
    if ((end - ptr) < NP_PACKET_HDR_MIN_BYTELENGTH) {
        return NULL;
    }
    
    WRITE_FORWARD_U32(ptr, hdr->nsi_cp);
    WRITE_FORWARD_U32(ptr, hdr->nsi_sp);
    WRITE_FORWARD_U8(ptr, hdr->type);
    WRITE_FORWARD_U8(ptr, hdr->version);
    WRITE_FORWARD_U8(ptr, hdr->rsvd);
    WRITE_FORWARD_U8(ptr, hdr->flags);
    WRITE_FORWARD_U16(ptr, hdr->seq);
    WRITE_FORWARD_U16(ptr, hdr->len);

    if (hdr->flags & NP_PACKET_HDR_FLAG_NSI_CO) {
        if ((end - ptr) < 8) {
            return NULL;
        }
        memcpy(ptr, hdr->nsi_co, 8); ptr += 8;
    }
    
    if (hdr->flags & NP_PACKET_HDR_FLAG_TAG) {
        if ((end - ptr) < 2) {
            return NULL;
        }
        WRITE_FORWARD_U16(ptr, hdr->tag);
    }
    return ptr;
}


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

const uint8_t* unabto_read_payload(const uint8_t* begin, const uint8_t* end, struct unabto_payload_packet* payload)
{
    ptrdiff_t bufferLength = end - begin;
    const uint8_t* ptr = begin;
    if (bufferLength == 0) {
        // end reached expected
        return NULL;
    }
    if (bufferLength < SIZE_PAYLOAD_HEADER) {
        NABTO_LOG_ERROR(("unexpected end of payloads"));
        return NULL;
    }
    payload->begin = begin;
    READ_FORWARD_U8(payload->type, ptr);
    READ_FORWARD_U8(payload->flags, ptr);
    READ_FORWARD_U16(payload->length, ptr);

    // check that the payload length is not greater than the buffer.
    if (payload->length > bufferLength) {
        return NULL;
    }

    payload->dataBegin = ptr;
    payload->dataEnd = begin + payload->length;
    payload->dataLength = payload->length - SIZE_PAYLOAD_HEADER;
    return payload->dataEnd;
}

bool unabto_find_payload(const uint8_t* begin, const uint8_t* end, uint8_t type, struct unabto_payload_packet* payload)
{
    const uint8_t* next = begin;
    do {
        next = unabto_read_payload(next, end, payload);
        if (next == NULL) {
            return false;
        }
        if (payload->type == type) {
            return true;
        }
    } while(next != NULL);
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

uint8_t* insert_payload(uint8_t* buf, uint8_t* end, uint8_t type, const uint8_t* content, size_t size)
{
    if (buf == NULL) {
        return buf;
    }
    if (end - buf < NP_PAYLOAD_HDR_BYTELENGTH) {
        return NULL;
    }
    /* Add payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes) */
    WRITE_FORWARD_U8(buf,                     type);
    WRITE_FORWARD_U8(buf, NP_PAYLOAD_HDR_FLAG_NONE);
    WRITE_FORWARD_U16(buf, (uint16_t)(size + NP_PAYLOAD_HDR_BYTELENGTH));
    
    /* Add payload data */
    if (content && size) {
        if (end - buf < (ptrdiff_t)size) {
            return NULL;
        }
        memcpy(buf, content, size);
        buf += size;
    }

    return buf; /* return end of payload */
}

/******************************************************************************/

uint8_t* insert_optional_payload(uint8_t* buf, uint8_t* end, uint8_t type, const uint8_t* content, size_t size)
{
    uint8_t* ptr = buf + 1; // to set optional bit
    uint8_t* res = insert_payload(buf, end, type, content, size);
    *ptr |= NP_PAYLOAD_HDR_FLAG_OPTIONAL; /* modify payload header flags */

    return res; /* return end of payload */
}

uint8_t* insert_capabilities(uint8_t* buf, uint8_t* end, bool cap_encr_off) {
    uint8_t* ptr;
    uint32_t mask;
    uint32_t bits;

    /* Build capabilities for this device */
    mask = PEER_CAP_MICRO |
        PEER_CAP_TAG |
        PEER_CAP_ASYNC |
        PEER_CAP_FB_TCP_U |
        PEER_CAP_UDP |
        PEER_CAP_ENCR_OFF |
        PEER_CAP_FP;
    bits = PEER_CAP_MICRO | PEER_CAP_TAG | PEER_CAP_UDP | PEER_CAP_FP;
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
    ptr = insert_payload(buf, end, NP_PAYLOAD_TYPE_CAPABILITY, 0, 9);
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
    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_IPX, 0, 13);
    
    WRITE_FORWARD_U32(ptr, nmc.socketGSPLocalEndpoint.addr);
    WRITE_FORWARD_U16(ptr, nmc.socketGSPLocalEndpoint.port);
    WRITE_FORWARD_U32(ptr, nmc.context.globalAddress.addr);
    WRITE_FORWARD_U16(ptr, nmc.context.globalAddress.port);
    WRITE_FORWARD_U8(ptr, nmc.context.natType);

    return ptr;
}

uint8_t* insert_version_payload(uint8_t* ptr, uint8_t* end)
{
    size_t prereleaseLength = strlen(UNABTO_VERSION_PRERELEASE);
    size_t buildLength = strlen(UNABTO_VERSION_BUILD);
    UNABTO_ASSERT(ptr <= end);
    if (end-ptr < (ptrdiff_t)(NP_PAYLOAD_VERSION_BYTELENGTH_PATCH + prereleaseLength + buildLength)) {
        return NULL;
    }
    
    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_VERSION, 0, (14+prereleaseLength+buildLength));

    WRITE_FORWARD_U16(ptr, NP_PAYLOAD_VERSION_TYPE_UD);
    WRITE_FORWARD_U32(ptr, UNABTO_VERSION_MAJOR);
    WRITE_FORWARD_U32(ptr, UNABTO_VERSION_MINOR);
    WRITE_FORWARD_U32(ptr, UNABTO_VERSION_PATCH);
    memcpy(ptr, UNABTO_VERSION_PRERELEASE, prereleaseLength); ptr += prereleaseLength;
    memcpy(ptr, UNABTO_VERSION_BUILD, buildLength); ptr += buildLength;
    return ptr;
}


uint8_t* insert_sp_id_payload(uint8_t* ptr, uint8_t* end) {
    size_t spIdLength = strlen(nmc.nabtoMainSetup.id);

    UNABTO_ASSERT(ptr <= end);
    if ((size_t)(end - ptr) < NP_PAYLOAD_SP_ID_BYTELENGTH + spIdLength) {
        return NULL;
    }

    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_SP_ID, 0, 1+spIdLength);

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
    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_STATS, 0, 1);
    WRITE_FORWARD_U8(ptr, stats_event_type);
    return ptr;
}

uint8_t* insert_notify_payload(uint8_t* ptr, uint8_t* end, uint32_t notifyValue) {

    UNABTO_ASSERT(ptr  <= end);
    if (end-ptr < NP_PAYLOAD_NOTIFY_BYTELENGTH) {
        return NULL;
    }
    
    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_NOTIFY, 0, 4);
    WRITE_FORWARD_U32(ptr, notifyValue);
    return ptr;
}

uint8_t* insert_piggy_payload(uint8_t* ptr, uint8_t* end, uint8_t* piggyData, uint16_t piggySize) {
    UNABTO_ASSERT(ptr  <= end);
    if ((uint16_t)(end-ptr) < (NP_PAYLOAD_PIGGY_SIZE_WO_DATA + piggySize)) {
        return NULL;
    }
    
    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_PIGGY, 0, 4 + piggySize);
    WRITE_FORWARD_U32(ptr, 0);
    memcpy(ptr, (const void*) piggyData, piggySize);
    ptr += piggySize;
    
    return ptr;
}
bool unabto_payload_read_push(struct unabto_payload_packet* payload, struct unabto_payload_push* push){
    const uint8_t* ptr = payload->dataBegin;
    if (payload->type != NP_PAYLOAD_TYPE_PUSH) {
        return false;
    }
    if (payload->length < NP_PAYLOAD_PUSH_BYTELENGTH){
        return false;
    }
    READ_FORWARD_U32(push->sequence,ptr);
    READ_FORWARD_U16(push->pnsId,ptr);
    READ_FORWARD_U8(push->flags,ptr);
    return true;
}
bool unabto_payload_read_ipx(struct unabto_payload_packet* payload, struct unabto_payload_ipx* ipx)
{
    const uint8_t* ptr = payload->dataBegin;
    if (payload->type != NP_PAYLOAD_TYPE_IPX) {
        return false;
    }

    if (payload->length < NP_PAYLOAD_IPX_BYTELENGTH) {
        return false;
    }

    ipx->haveSpNsi = false;
    ipx->haveFullNsi = false;

    READ_FORWARD_U32(ipx->privateIpAddress, ptr);
    READ_FORWARD_U16(ipx->privateIpPort, ptr);
    READ_FORWARD_U32(ipx->globalIpAddress, ptr);
    READ_FORWARD_U16(ipx->globalIpPort, ptr);
    READ_FORWARD_U8(ipx->flags, ptr);

    if (payload->length >=  NP_PAYLOAD_IPX_NSI_BYTELENGTH) {
        READ_FORWARD_U32(ipx->spNsi, ptr);
        ipx->haveSpNsi = true;
    }

    if (payload->length >= NP_PAYLOAD_IPX_FULL_NSI_BYTELENGTH) {
        READ_FORWARD(ipx->coNsi, ptr, 8);
        READ_FORWARD_U32(ipx->cpNsi, ptr);
        ipx->haveFullNsi = true;
    }
    return true;
}

bool unabto_payload_read_typed_buffer(struct unabto_payload_packet* payload, struct unabto_payload_typed_buffer* buffer)
{
    const uint8_t* ptr = payload->dataBegin;
    if (payload->length < NP_PAYLOAD_HDR_BYTELENGTH + 1) {
        return false;
    }
    READ_FORWARD_U8(buffer->type, ptr);
    buffer->dataBegin = ptr;
    buffer->dataEnd = payload->dataEnd;
    buffer->dataLength = payload->length - (NP_PAYLOAD_HDR_BYTELENGTH + 1);
    return true;
}

bool unabto_payload_read_gw(struct unabto_payload_packet* payload, struct unabto_payload_gw* gw)
{
    const uint8_t* ptr = payload->dataBegin;
    if (payload->length < NP_PAYLOAD_GW_MIN_BYTELENGTH) {
        return false;
    }

    READ_FORWARD_U32(gw->ipAddress, ptr);
    READ_FORWARD_U16(gw->port, ptr);
    READ_FORWARD_U32(gw->nsi, ptr);
    gw->gwIdLength = payload->length - NP_PAYLOAD_GW_MIN_BYTELENGTH;
    gw->gwId = ptr;
    
    return true;
}

bool unabto_payload_read_ep(struct unabto_payload_packet* payload, struct unabto_payload_ep* ep)
{
    const uint8_t* ptr = payload->dataBegin;
    if (payload->length < NP_PAYLOAD_EP_BYTELENGTH) {
        return false;
    }

    READ_FORWARD_U32(ep->address, ptr);
    READ_FORWARD_U16(ep->port, ptr);
    return true;
}

bool unabto_payload_read_crypto(struct unabto_payload_packet* payload, struct unabto_payload_crypto* crypto)
{
    const uint8_t* ptr = payload->dataBegin;
    if (payload->length < NP_PAYLOAD_CRYPTO_BYTELENGTH) {
        return false;
    }
    READ_FORWARD_U16(crypto->code, ptr);
    crypto->dataBegin = ptr;
    crypto->dataEnd = payload->dataEnd;
    crypto->dataLength = payload->length - NP_PAYLOAD_CRYPTO_BYTELENGTH;
    return true;
}


#endif
