/*
 * Copyright (C) Nabto - All Rights Reserved.
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

void nabto_header_init(nabto_packet_header* header, uint8_t type, uint32_t cpnsi, uint32_t spnsi)
{
    memset(header, 0, sizeof(nabto_packet_header));
    header->type = type;
    header->nsi_cp = cpnsi;
    header->nsi_sp = spnsi;
}


void nabto_header_add_flags(nabto_packet_header* header, uint8_t flags)
{
    header->flags |= flags;
}


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
    ptr = write_forward_u32(ptr, end, hdr->nsi_cp);
    ptr = write_forward_u32(ptr, end, hdr->nsi_sp);
    ptr = write_forward_u8(ptr, end, hdr->type);
    ptr = write_forward_u8(ptr, end, hdr->version);
    ptr = write_forward_u8(ptr, end, hdr->rsvd);
    ptr = write_forward_u8(ptr, end, hdr->flags);
    ptr = write_forward_u16(ptr, end, hdr->seq);
    ptr = write_forward_u16(ptr, end, hdr->len);

    if (hdr->flags & NP_PACKET_HDR_FLAG_NSI_CO) {
        ptr = write_forward_mem(ptr, end, hdr->nsi_co, 8);
    }

    if (hdr->flags & NP_PACKET_HDR_FLAG_TAG) {
        ptr = write_forward_u16(ptr, end, hdr->tag);
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
    ptr = read_forward_u8(&payload->type, ptr, end);
    ptr = read_forward_u8(&payload->flags, ptr, end);
    ptr = read_forward_u16(&payload->length, ptr, end);
    if (ptr == NULL) {
        NABTO_LOG_ERROR(("unexpected end of payloads"));
        return NULL;
    }

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

uint8_t* insert_header(uint8_t* buf, const uint8_t* end, uint32_t cpnsi, uint32_t spnsi, uint8_t type, bool rsp, uint16_t seq, uint16_t tag, uint8_t* nsico)
{
    uint8_t flags = NP_PACKET_HDR_FLAG_NONE;
    size_t required = NP_PACKET_HDR_MIN_BYTELENGTH;

    if (buf == NULL) return NULL;

    if (rsp)
        flags |= NP_PACKET_HDR_FLAG_RESPONSE;
    if (tag) {
        flags |= NP_PACKET_HDR_FLAG_TAG;
        required += 2;
    }
    if (nsico) {
        flags |= NP_PACKET_HDR_FLAG_NSI_CO;
        required += 8;
    }

    if ((size_t)(end - buf) < required) return NULL;

    /* Write fixed part of packet header */
    buf = write_forward_u32(buf, end, cpnsi);        /* NSI.cp              */
    buf = write_forward_u32(buf, end, spnsi);        /* NSI.sp              */
    buf = write_forward_u8(buf, end, type);          /* type: U_INVITE, ... */
    buf = write_forward_u8(buf, end, PROTOVERSION);  /* version             */
    buf = write_forward_u8(buf, end, 0);             /* reserved            */
    buf = write_forward_u8(buf, end, flags);         /* flags               */
    buf = write_forward_u16(buf, end, seq);          /* seq                 */
    buf = write_forward_u16(buf, end, 0);            /* length to be patched later */

    /* Write controller nsi to the packet header (optional) */
    if (nsico) {
        buf = write_forward_mem(buf, end, nsico, 8);
    }

    /* Write tag to packet header (optional) */
    if (tag) {
        buf = write_forward_u16(buf, end, tag);
    }

    return buf; /* return end of packet header */
}

/******************************************************************************/

uint8_t* insert_data_header(uint8_t* buf, const uint8_t* end, uint32_t nsi, uint8_t* nsico, uint16_t tag)
{
    return insert_header(buf, end, 0, nsi, DATA, false, 0, tag, nsico);
}

bool insert_packet_length(uint8_t* buf, const uint8_t* end, uint16_t length) {
    if (buf == NULL || (size_t)(end - buf) < OFS_PACKET_LGT + 2) return false;
    WRITE_U16((uint8_t*)(buf) + OFS_PACKET_LGT, (uint16_t)(length));
    return true;
}

bool insert_packet_length_from_cursor(uint8_t* packetBegin, const uint8_t* packetEnd) {
    ptrdiff_t length;
    if (packetBegin == NULL || packetEnd == NULL) return false;
    if (packetEnd < packetBegin) return false;
    length = packetEnd - packetBegin;
    if (length > UINT16_MAX) return false;
    return insert_packet_length(packetBegin, packetEnd, (uint16_t)length);
}

/******************************************************************************/

uint8_t* insert_payload(uint8_t* buf, uint8_t* end, uint8_t type, const uint8_t* content, size_t size)
{
    if (buf == NULL) {
        return buf;
    }
    /* Add payload header (NP_PAYLOAD_HDR_BYTELENGTH bytes) */
    buf = write_forward_u8(buf, end, type);
    buf = write_forward_u8(buf, end, NP_PAYLOAD_HDR_FLAG_NONE);
    buf = write_forward_u16(buf, end, (uint16_t)(size + NP_PAYLOAD_HDR_BYTELENGTH));
    if (buf == NULL) { return NULL; }

    /* Add payload data */
    if (content && size) {
        buf = write_forward_mem(buf, end, content, size);
    }

    return buf; /* return end of payload */
}

/******************************************************************************/

uint8_t* insert_optional_payload(uint8_t* buf, uint8_t* end, uint8_t type, const uint8_t* content, size_t size)
{
    uint8_t* res;
    if (buf == NULL) return NULL;
    res = insert_payload(buf, end, type, content, size);
    if (res == NULL) return NULL;
    *(buf + 1) |= NP_PAYLOAD_HDR_FLAG_OPTIONAL; /* modify payload header flags */

    return res; /* return end of payload */
}

uint8_t* insert_capabilities(uint8_t* buf, uint8_t* end, bool cap_encr_off) {

    struct unabto_capabilities capabilities;
    capabilities.type = 0;

    /* Build capabilities for this device */
    capabilities.mask = PEER_CAP_MICRO |
        PEER_CAP_TAG |
        PEER_CAP_ASYNC |
        PEER_CAP_FB_TCP_U |
        PEER_CAP_UDP |
        PEER_CAP_ENCR_OFF |
        PEER_CAP_FP;
    capabilities.bits = PEER_CAP_MICRO | PEER_CAP_TAG | PEER_CAP_UDP | PEER_CAP_FP;
#if NABTO_MICRO_CAP_ASYNC
    capabilities.bits |= PEER_CAP_ASYNC;
#endif
#if NABTO_ENABLE_TCP_FALLBACK
    if (nmc.nabtoMainSetup.enableTcpFallback) {
        capabilities.bits |= PEER_CAP_FB_TCP_U;
    }
#endif

    if (cap_encr_off) {
        capabilities.bits |= PEER_CAP_ENCR_OFF;
    }

    return insert_capabilities_payload(buf, end, &capabilities, 0);
}


uint8_t* insert_ipx_payload(uint8_t* ptr, uint8_t* end) {
    uint32_t localAddr = 0;
    uint32_t globalAddr = 0;

    UNABTO_ASSERT(ptr <= end);
    if (end-ptr < NP_PAYLOAD_IPX_BYTELENGTH) {
        return NULL;
    }
    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_IPX, 0, 13);

    if (nmc.socketGSPLocalEndpoint.addr.type == NABTO_IP_V4) {
        localAddr = nmc.socketGSPLocalEndpoint.addr.addr.ipv4;
    }

    if (nmc.context.globalAddress.addr.type == NABTO_IP_V4) {
        globalAddr = nmc.context.globalAddress.addr.addr.ipv4;
    }

    ptr = write_forward_u32(ptr, end, localAddr);
    ptr = write_forward_u16(ptr, end, nmc.socketGSPLocalEndpoint.port);
    ptr = write_forward_u32(ptr, end, globalAddr);
    ptr = write_forward_u16(ptr, end, nmc.context.globalAddress.port);
    ptr = write_forward_u8(ptr, end, nmc.context.natType);

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

    ptr = write_forward_u16(ptr, end, NP_PAYLOAD_VERSION_TYPE_UD);
    ptr = write_forward_u32(ptr, end, UNABTO_VERSION_MAJOR);
    ptr = write_forward_u32(ptr, end, UNABTO_VERSION_MINOR);
    ptr = write_forward_u32(ptr, end, UNABTO_VERSION_PATCH);
    ptr = write_forward_mem(ptr, end, UNABTO_VERSION_PRERELEASE, prereleaseLength);
    ptr = write_forward_mem(ptr, end, UNABTO_VERSION_BUILD, buildLength);
    return ptr;
}


uint8_t* insert_sp_id_payload(uint8_t* ptr, uint8_t* end) {
    size_t spIdLength = strlen(nmc.nabtoMainSetup.id);

    UNABTO_ASSERT(ptr <= end);
    if ((size_t)(end - ptr) < NP_PAYLOAD_SP_ID_BYTELENGTH + spIdLength) {
        return NULL;
    }

    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_SP_ID, 0, 1+spIdLength);

    ptr = write_forward_u8(ptr, end, NP_PAYLOAD_CP_ID_TYPE_URL);
    ptr = write_forward_mem(ptr, end, nmc.nabtoMainSetup.id, spIdLength);

    return ptr;
}

uint8_t* insert_stats_payload(uint8_t* ptr, uint8_t* end, uint8_t stats_event_type) {
    UNABTO_ASSERT(ptr <= end);
    if (end-ptr < NP_PAYLOAD_STATS_BYTELENGTH) {
        return NULL;
    }
    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_STATS, 0, 1);
    ptr = write_forward_u8(ptr, end, stats_event_type);
    return ptr;
}

uint8_t* insert_notify_payload(uint8_t* ptr, uint8_t* end, uint32_t notifyValue) {

    UNABTO_ASSERT(ptr  <= end);
    if (end-ptr < NP_PAYLOAD_NOTIFY_BYTELENGTH) {
        return NULL;
    }

    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_NOTIFY, 0, 4);
    ptr = write_forward_u32(ptr, end, notifyValue);
    return ptr;
}

uint8_t* insert_piggy_payload(uint8_t* ptr, uint8_t* end, uint8_t* piggyData, uint16_t piggySize) {
    UNABTO_ASSERT(ptr  <= end);
    if ((uint16_t)(end-ptr) < (NP_PAYLOAD_PIGGY_SIZE_WO_DATA + piggySize)) {
        return NULL;
    }

    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_PIGGY, 0, 4 + piggySize);
    ptr = write_forward_u32(ptr, end, 0);
    ptr = write_forward_mem(ptr, end, (const void*) piggyData, piggySize);

    return ptr;
}

uint8_t* insert_nonce_payload(uint8_t* ptr, uint8_t* end, const uint8_t* nonceData, uint16_t nonceSize)
{
    return insert_payload(ptr, end, NP_PAYLOAD_TYPE_NONCE, nonceData, nonceSize);
}

uint8_t* insert_random_payload(uint8_t* ptr, uint8_t* end, uint8_t* randomData, uint16_t randomSize)
{
    return insert_payload(ptr, end, NP_PAYLOAD_TYPE_RANDOM, randomData, randomSize);
}

uint8_t* insert_capabilities_payload(uint8_t* ptr, uint8_t* end, struct unabto_capabilities* capabilities, uint16_t encryptionCodes)
{
    uint16_t dataLength = 9;
    if (end < ptr || ptr == NULL) {
        return NULL;
    }

    if (encryptionCodes > 0) {
        dataLength += 2 + encryptionCodes * 2;
    }

    if (end - ptr < 4 + dataLength) {
        return NULL;
    }

    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_CAPABILITY, 0, dataLength);
    if (ptr == NULL) {
        return NULL;
    }

    ptr = write_forward_u8(ptr, end, capabilities->type);
    ptr = write_forward_u32(ptr, end, capabilities->bits);
    ptr = write_forward_u32(ptr, end, capabilities->mask);

    if (encryptionCodes > 0) {
        ptr = write_forward_u16(ptr, end, encryptionCodes);
    }
    // the caller needs to insert the encryption codes
    return ptr;
}


bool unabto_payload_read_push(struct unabto_payload_packet* payload, struct unabto_payload_push* push){
    const uint8_t* end = payload->dataBegin + payload->dataLength;
    const uint8_t* ptr = payload->dataBegin;
    if (payload->type != NP_PAYLOAD_TYPE_PUSH) {
        return false;
    }
    ptr = read_forward_u32(&push->sequence, ptr, end);
    ptr = read_forward_u16(&push->pnsId, ptr, end);
    ptr = read_forward_u8(&push->flags, ptr, end);
    return ptr != NULL;
}
bool unabto_payload_read_ipx(struct unabto_payload_packet* payload, struct unabto_payload_ipx* ipx)
{
    const uint8_t* end = payload->dataBegin + payload->dataLength;
    const uint8_t* ptr = payload->dataBegin;
    if (payload->type != NP_PAYLOAD_TYPE_IPX) {
        return false;
    }

    ipx->haveSpNsi = false;
    ipx->haveFullNsi = false;

    ptr = read_forward_u32(&ipx->privateIpAddress, ptr, end);
    ptr = read_forward_u16(&ipx->privateIpPort, ptr, end);
    ptr = read_forward_u32(&ipx->globalIpAddress, ptr, end);
    ptr = read_forward_u16(&ipx->globalIpPort, ptr, end);
    ptr = read_forward_u8(&ipx->flags, ptr, end);
    if (ptr == NULL) {
        return false;
    }

    if (payload->length >=  NP_PAYLOAD_IPX_NSI_BYTELENGTH) {
        ptr = read_forward_u32(&ipx->spNsi, ptr, end);
        if (ptr == NULL) {
            return false;
        }
        ipx->haveSpNsi = true;
    }

    if (payload->length >= NP_PAYLOAD_IPX_FULL_NSI_BYTELENGTH) {
        ptr = read_forward_mem(ipx->coNsi, ptr, end, 8);
        ptr = read_forward_u32(&ipx->cpNsi, ptr, end);
        if (ptr == NULL) {
            return false;
        }
        ipx->haveFullNsi = true;
    }
    return true;
}

bool unabto_payload_read_typed_buffer(struct unabto_payload_packet* payload, struct unabto_payload_typed_buffer* buffer)
{
    const uint8_t* end = payload->dataBegin + payload->dataLength;
    const uint8_t* ptr = payload->dataBegin;
    ptr = read_forward_u8(&buffer->type, ptr, end);
    if (ptr == NULL) {
        return false;
    }
    buffer->dataBegin = ptr;
    buffer->dataEnd = payload->dataEnd;
    buffer->dataLength = payload->length - (NP_PAYLOAD_HDR_BYTELENGTH + 1);
    return true;
}

bool unabto_payload_read_gw(struct unabto_payload_packet* payload, struct unabto_payload_gw* gw)
{
    const uint8_t* end = payload->dataBegin + payload->dataLength;
    const uint8_t* ptr = payload->dataBegin;

    ptr = read_forward_u32(&gw->ipAddress, ptr, end);
    ptr = read_forward_u16(&gw->port, ptr, end);
    ptr = read_forward_u32(&gw->nsi, ptr, end);
    if (ptr == NULL) {
        return false;
    }
    gw->gwIdLength = payload->length - NP_PAYLOAD_GW_MIN_BYTELENGTH;
    gw->gwId = ptr;

    return true;
}

bool unabto_payload_read_ep(struct unabto_payload_packet* payload, struct unabto_payload_ep* ep)
{
    const uint8_t* end = payload->dataBegin + payload->dataLength;
    const uint8_t* ptr = payload->dataBegin;

    ptr = read_forward_u32(&ep->address, ptr, end);
    ptr = read_forward_u16(&ep->port, ptr, end);
    return ptr != NULL;
}

bool unabto_payload_read_crypto(struct unabto_payload_packet* payload, struct unabto_payload_crypto* crypto)
{
    const uint8_t* end = payload->dataBegin + payload->dataLength;
    const uint8_t* ptr = payload->dataBegin;

    ptr = read_forward_u16(&crypto->code, ptr, end);
    if (ptr == NULL) {
        return false;
    }
    crypto->dataBegin = ptr;
    crypto->dataEnd = payload->dataEnd;
    crypto->dataLength = payload->length - NP_PAYLOAD_CRYPTO_BYTELENGTH;
    return true;
}

bool unabto_payload_find_and_read_crypto(const uint8_t* buf, const uint8_t* end, struct unabto_payload_crypto* crypto)
{
    struct unabto_payload_packet payload;
    if (end < buf || buf == NULL) {
        return false;
    }

    if (!unabto_find_payload(buf, end, NP_PAYLOAD_TYPE_CRYPTO, &payload)) {
        return false;
    }

    if (!unabto_payload_read_crypto(&payload, crypto)) {
        return false;
    }

    return true;

}

bool unabto_payload_read_notify(struct unabto_payload_packet* payload, struct unabto_payload_notify* notify)
{
    if (payload->dataLength < 4) {
        return false;
    }
    READ_U32(notify->code, payload->dataBegin);
    return true;
}

bool unabto_payload_read_capabilities(struct unabto_payload_packet* payload, struct unabto_payload_capabilities_read* capabilities)
{
    const uint8_t* end = payload->dataBegin + payload->dataLength;
    const uint8_t* ptr = payload->dataBegin;

    ptr = read_forward_u8(&capabilities->type, ptr, end);
    ptr = read_forward_u32(&capabilities->bits, ptr, end);
    ptr = read_forward_u32(&capabilities->mask, ptr, end);
    if (ptr == NULL) {
        return false;
    }

    if (payload->dataLength >= 11) {
        ptr = read_forward_u16(&capabilities->codesLength, ptr, end);
        if (ptr == NULL) {
            return false;
        }
        if(capabilities->codesLength > ((payload->dataLength - 11)/2)) {
            NABTO_LOG_WARN(("more encryption codes said than possibly"));
            return false;
        }
        capabilities->codesStart = ptr;
    } else {
        capabilities->codesLength = 0;
    }
    return true;
}

uint8_t* unabto_payloads_begin(uint8_t* packetBegin, const nabto_packet_header* header)
{
    return packetBegin + header->hlen;
}

uint8_t* unabto_payloads_end(uint8_t* packetBegin, const nabto_packet_header* header)
{
    return packetBegin + header->len;
}

#endif
