/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
/**
 * @file
 * Nabto uServer attachment and remote connect protocol - Implementation.
 */

#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_UNABTO

#include "unabto_env_base.h"

#if NABTO_ENABLE_REMOTE_ACCESS

#include "unabto_attach.h"
#include "unabto_connection.h"
#include "unabto_packet_util.h"
#include "unabto_util.h"
#include "unabto_logging.h"
#include "unabto_app.h"
#include "unabto_external_environment.h"
#include "unabto_version.h"
#include "unabto_context.h"
#include "unabto_main_contexts.h"
#include "unabto_crypto.h"
#include "unabto_memory.h"
#include "unabto_packet.h"
#include "unabto_dns_fallback.h"

#include <string.h>
#include <stdlib.h>

#define NABTO_MICRO_CAP_ASYNC  1   ///< 1 if the MICRO is able to use async dialogue, else 0

/**
 * Write change of state to the event log
 * @param oldst  the old state
 * @param newst  the new state
 */
#define NABTO_STATE_LOG(oldst, newst)  NABTO_LOG_INFO(("State change from %" PRItext " to %" PRItext, stName(oldst), stName(newst)))

/** Magic values */
enum {
    V_UD         = 5  /**< Version Type: UDEVICE  */
};

/** Timer values */
enum {
    INTERVAL_INITIAL_CONFIGURATION_CHECK = 10000,
    INTERVAL_BS_INVITE   = 2000,
    INTERVAL_GSP_INVITE  = 2000,
    INTERVAL_DNS_FALLBACK = 2000,
    INTERVAL_DNS_RESOLVE = 100,
    INTERVAL_ERROR_RETRY_BASE = 2000 // base time from an error occurs to we retry.
};

/** Calculate exponential fallback interval. @param BASE  base interval. @param NUM  event counter. @return the interval  */
#define EXP_WAIT(BASE, NUM) ((BASE)<<(MIN((NUM), 4)))

/******************************************************************************/
/******************************************************************************/

#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_INFO)

/**
 * Get a printable state name
 * @param state  the state
 * @return textual representation
 */
text stName(nabto_state state)
{
    switch (state) {
        case NABTO_AS_IDLE     :                      return "IDLE";
        case NABTO_AS_WAIT_DNS :                      return "WAIT_DNS";
        case NABTO_AS_WAIT_BS  :                      return "WAIT_BS";
        case NABTO_AS_WAIT_DNS_FALLBACK_OPEN_SOCKET : return "WAIT_DNS_FALLBACK_OPEN_SOCKET";
        case NABTO_AS_WAIT_DNS_FALLBACK_BS:           return "WAIT_DNS_FALLBACK_BS";
        case NABTO_AS_WAIT_DNS_FALLBACK_UDP_BS:       return "WAIT_DNS_FALLBACK_UDP_BS";
        case NABTO_AS_WAIT_GSP :                      return "WAIT_GSP";
        case NABTO_AS_ATTACHED :                      return "ATTACHED";
    }
    return "??";
}

#endif

#if NABTO_ENABLE_STATUS_CALLBACKS
#define REPORT_STATUS_CALLBACK(state) do { unabto_attach_state_changed(state); } while(0)
#else
#define REPORT_STATUS_CALLBACK(state)
#endif

/**
 * Set a new state, and log the change
 * @param newState  the new state
 */
#define SET_CTX_STATE(newState)                   \
    if (nmc.context.state != newState) \
    {                                        \
        NABTO_STATE_LOG(nmc.context.state, newState); \
        REPORT_STATUS_CALLBACK(newState);         \
        nmc.context.state = newState;                 \
    }

/**
 * Set a new state, and log the change
 * @param newState  the new state
 * Only if the state is changed, the counter is reset
 */
#define SET_CTX_STATE_CNT(newState)                     \
    if (nmc.context.state != newState)                  \
    {                                              \
        NABTO_STATE_LOG(nmc.context.state, newState);   \
        REPORT_STATUS_CALLBACK(newState);               \
        nmc.context.state = newState;                   \
        nmc.context.counter = 0;  \
    }

/**
 * Set a new state, set the stamp and log the change
 * @param ctx    pointer to the context
 * @param intv   the interval to add to currrent time (in millisecs)
 */
#define SET_CTX_STAMP(intv) do { nabtoSetFutureStamp(&nmc.context.timestamp, (intv)); } while (0)

/**
 * Set a new state, set the stamp and log the change
 * @param newst  the new state
 * @param intv   the interval to add to currrent time (in millisecs)
 */
#define SET_CTX_STATE_STAMP(newst, intv) do { SET_CTX_STATE_CNT(newst); SET_CTX_STAMP(intv); } while (0)

/******************************************************************************/

void handle_ok_invite_event(void);

/**
 * Initialise the verification data.
 * @param verif  the verification data.
 */
static void verification_init(verification_t* verif)
{
    verif->sum = 0;
}

/**
 * Add buffer to the verification data.
 * @param verif  the verification data
 * @param buf    the buffer
 * @param end    the end of the buffer
 */
static void verification_add(verification_t* verif, const uint8_t* buf, const uint8_t* end)
{
    /*uint32_t old = verif->sum;*/
    const uint8_t* ptr = buf;
    while (ptr < end) verif->sum += *ptr++;
    /*NABTO_TRACE("verification_add [" << (void*)verif << "] " << end - buf << " bytes, value before: "
        << std::hex << old << ", after: " << verif->sum << std::dec << '\n' << nabto::BufPH(buf, end - buf));*/
}

/**
 * Complete the calculation of the verification and insert it into the buffer.
 * @param verif  the verification data
 * @param buf    the buffer
 * @param ptr    the position to insert the verify payload
 * @return       the size of the completed message
 */
static size_t verification_complete(verification_t* verif, uint8_t* buf, uint8_t* ptr)
{
    size_t   len;
    uint32_t sum;
    size_t   ix;
    uint8_t  data[12];
    memset(data, NP_PAYLOAD_VERIFY_DEV_FILLERBYTE, sizeof(data));

    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_VERIFY, 0, sizeof(data));
    len = ptr - buf + sizeof(data);
    insert_length(buf, len);
    verification_add(verif, buf, ptr);
    sum = verif->sum;
    ix = sizeof(data);
    // Write the 32bit sum in the last bytes without writing zeroes in bigendian.
    while (1) {
        data[--ix] = (uint8_t)(sum & 0xff);
        if (sum == 0) break;
        sum >>= 8;
    }
    memcpy(ptr, (const void*) data, sizeof(data));
    return len;
}

/**
 * Perform verification evaluation.
 * @param verif  the verification data
 * @param buf    the received buffer
 * @param end    end of the received buffer
 * @return       true iff the received buffer contains the correct verification
 */
bool verification_check(verification_t* verif, const uint8_t* buf, const uint8_t* end)
{
    uint32_t sum = 0;
    const uint8_t* ptr = buf;

    while (ptr < end && *ptr == NP_PAYLOAD_VERIFY_GSP_FILLERBYTE) ++ptr;
    while (ptr < end) sum = (sum << 8) + *ptr++;
//    NABTO_DEBUG("Verification from GSP: " << std::hex << verif->sum << ' ' << sum << std::dec
//        << '\n' << nabto::BufPH(buf, end - buf));
    if (verif->sum == sum) return true;
    NABTO_LOG_ERROR(("Verification failed"));
    return false;
}

/******************************************************************************/
/******************************************************************************/


/**
 * Build the U_INVITE to the Base Station or the GroupServerPeer
 * @param buf    the destination buffer
 * @param ctx    the context
 * @param toGSP  true if the invite is to the GSP (also sends a nonce from ctx)
 * @return       the size of the U_INVITE
 */
static size_t mk_invite(uint8_t* buf, bool toGSP)
{
    size_t len;
    uint8_t* ptr = insert_header(buf, 0, 0, U_INVITE, false, 0, 0, 0);
    size_t sid_len = strlen(nmc.nabtoMainSetup.id);

    NABTO_LOG_TRACE(("len=%" PRIsize, sid_len));
    NABTO_LOG_TRACE(("Send INVITE from '%s' to %" PRItext, nmc.nabtoMainSetup.id, toGSP ? "GSP" : "BS"));
    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_VERSION, 0, 2 + 4 + 4);
    WRITE_U16(ptr, NP_PAYLOAD_VERSION_TYPE_UD); ptr += 2; // version.type
    WRITE_U32(ptr, (uint32_t)RELEASE_MAJOR);    ptr += 4; // version.major
    WRITE_U32(ptr, (uint32_t)RELEASE_MINOR);    ptr += 4; // version.minor
    if (toGSP) {
        const char dummy[2] = { '-', 0 };
        const char* version;
        const char* url;
        size_t sz;

        uint16_t code;
        code = nmc.nabtoMainSetup.cryptoSuite;
        ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_NONCE, nmc.context.nonceMicro, nmc.context.nonceSize);

        if (nmc.nabtoMainSetup.version) {
            version = nmc.nabtoMainSetup.version;
            sz = strlen(version);
        } else {
            version = dummy;
            sz = 1;
        }
        ptr = insert_optional_payload(ptr, NP_PAYLOAD_TYPE_DESCR, 0, sz + 1);
        WRITE_U8(ptr, NP_PAYLOAD_DESCR_TYPE_VERSION); ptr += 1;
        memcpy(ptr, version, sz); ptr += sz;

        if (nmc.nabtoMainSetup.url) {
            url = nmc.nabtoMainSetup.url;
            sz = strlen(url);
            ptr = insert_optional_payload(ptr, NP_PAYLOAD_TYPE_DESCR, 0, sz + 1);
            WRITE_U8(ptr, NP_PAYLOAD_DESCR_TYPE_URL); ptr += 1;
            memcpy(ptr, url, sz); ptr += sz;
        } else {
            url = dummy;
        }
        NABTO_LOG_INFO(("########    U_INVITE with %" PRItext " nonce sent, version: %s URL: %s", nmc.context.nonceSize == NONCE_SIZE ? "LARGE" : "small", version, url));

        // send encryption capabilities
        ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_CAPABILITY, 0, 9 + 2*2);
        *ptr++ = 0; /* type */
        WRITE_U32(ptr,   0l); ptr += 4; // mask
        WRITE_U32(ptr,   0l); ptr += 4; // bits
        WRITE_U16(ptr,    1); ptr += 2; // number of codes
        WRITE_U16(ptr, code); ptr += 2;
    }
    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_SP_ID, 0, sid_len + 1);
    *ptr++ = NP_PAYLOAD_SP_ID_TYPE_URL; /* SPID_URL */
    memcpy(ptr, nmc.nabtoMainSetup.id, sid_len);
    
    ptr += sid_len;
    
    len = ptr - buf;

    insert_length(buf, len);

    return len;
}

static void send_controller_invite(void) {
    size_t bytes;

    NABTO_LOG_DEBUG(("Sending INVITE to Base Station: %i", nmc.context.counter));

    bytes = mk_invite(nabtoCommunicationBuffer, false); /* Send Packet (1) */
    if (bytes) {
        send_to_basestation(nabtoCommunicationBuffer, bytes, &nmc.controllerEp);
    }
}

static void send_gsp_invite(void) {
    size_t bytes;

    NABTO_LOG_DEBUG(("Sending INVITE to GSP: %i", nmc.context.counter));
    
    bytes = mk_invite(nabtoCommunicationBuffer, true); /* Send Packet(3) */
    if (bytes) {
        if (nmc.context.nonceSize == NONCE_SIZE_OLD) {
            verification_init(&nmc.context.verif1);
            verification_init(&nmc.context.verif2);
            verification_add(&nmc.context.verif1, nabtoCommunicationBuffer, nabtoCommunicationBuffer + bytes);
            verification_add(&nmc.context.verif2, nabtoCommunicationBuffer, nabtoCommunicationBuffer + bytes);
        }
        send_to_basestation(nabtoCommunicationBuffer, bytes, &nmc.context.gsp);
    }
}


/******************************************************************************/

/**
 * Build U_ATTACH response to GSP
 * @param buf       the destination buffer
 * @param ctx       the context
 * @param seq       the sequence number
 * @param nonceGSP  the nonce received from the GSP
 * @param seedGSP   the seed from the GSP
 * @return          the size of the response
 */
static bool send_gsp_attach_rsp(uint16_t seq, const uint8_t* nonceGSP, const uint8_t* seedGSP)
{
    uint8_t* buf = nabtoCommunicationBuffer;
    uint8_t* end = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;

    bool res = false;
    uint8_t* ptr = insert_header(buf, 0, nmc.context.gspnsi, U_ATTACH, true, seq, 0, 0);

    ptr = insert_capabilities(ptr, nmc.context.clearTextData);

    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_IPX, 0, 13);
    WRITE_U32(ptr, nmc.socketGSPLocalEndpoint.addr); ptr += 4;
    WRITE_U16(ptr, nmc.socketGSPLocalEndpoint.port); ptr += 2;
    WRITE_U32(ptr, nmc.context.globalAddress.addr); ptr += 4;
    WRITE_U16(ptr, nmc.context.globalAddress.port); ptr += 2;
    WRITE_U8(ptr, nmc.context.natType); ptr++;

    ptr = insert_notify_payload(ptr, end, NP_PAYLOAD_NOTIFY_ATTACH_OK);

    if (nmc.context.nonceSize == NONCE_SIZE_OLD) {
        size_t sz;
        //NABTO_LOG_INFO(("########    U_ATTACH response without crypto payload sent"));
        sz = verification_complete(&nmc.context.verif2, buf, ptr);
        send_to_basestation(nabtoCommunicationBuffer, sz, &nmc.context.gsp);
        res = true;
    } else {
#if NABTO_ENABLE_UCRYPTO
        uint8_t tmp[NONCE_SIZE + SEED_SIZE];

        memcpy(tmp, nonceGSP, NONCE_SIZE);
        nabto_random(tmp + NONCE_SIZE, SEED_SIZE);
        unabto_crypto_reinit_c(nonceGSP, tmp + NONCE_SIZE, seedGSP);

        ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_CRYPTO, 0, 0);

        return send_and_encrypt_packet(&nmc.context.gsp, nmc.context.cryptoAttach, tmp, sizeof(tmp), ptr);

#else
        NABTO_LOG_FATAL(("AES encryption unavailable"));
#endif
    }
    return res;
}

/******************************************************************************/

/**
 * Build U_ALIVE response to GSP
 * @param buf        the destination buffer
 * @param ctx        the context
 * @param seq        the sequence number
 * @param piggySize  the size of the piggyback data (0 for none)
 * @param piggyData  the piggyback data
 * @return       the size of the response
 */
static size_t mk_gsp_alive_rsp(uint16_t seq, size_t piggySize, uint8_t* piggyData)
{
    uint8_t* buf = nabtoCommunicationBuffer;
    uint8_t* end = nabtoCommunicationBuffer+nabtoCommunicationBufferSize;
    size_t len;
    uint8_t* ptr;

    if(nabtoCommunicationBufferSize < (NP_PACKET_HDR_MIN_BYTELENGTH))
    {
      NABTO_LOG_ERROR(("Communication buffer too small for header!"));
      return 0;
    }

    ptr = insert_header(buf, 0, nmc.context.gspnsi, U_ALIVE, true, seq, 0, 0);

    ptr = insert_notify_payload(ptr, end, NP_PAYLOAD_NOTIFY_ALIVE_OK);
    if (ptr == NULL) {
        return 0;
    }
    if (piggySize) {
        ptr = insert_piggy_payload(ptr, end, piggyData, piggySize);
        if (ptr == NULL) {
            NABTO_LOG_ERROR(("Communication buffer too small for piggy!"));
            return 0;
        }
    }
    len = ptr - buf;
    insert_length(buf, len);
    return len;
}

static void send_gsp_alive_poll(void) {
    size_t olen;
    NABTO_LOG_TRACE(("Sending ALIVE POLL to GSP: %i", nmc.context.counter));
    olen = mk_gsp_alive_rsp(0, 0, 0);
    nabtoSetFutureStamp(&nmc.context.timestamp, nmc.nabtoMainSetup.gspPollTimeout);
    send_to_basestation(nabtoCommunicationBuffer, olen, &nmc.context.gsp);
} 

#if NABTO_ENABLE_DNS_FALLBACK
static void make_dns_fallback(void) {
    unabto_dns_fallback_error_code ec = unabto_dns_fallback_create_socket();
    if (ec == UDF_OK) {
        nmc.context.hasDnsFallbackSocket = true;
        nmc.context.useDnsFallback = true;
        SET_CTX_STATE_STAMP(NABTO_AS_WAIT_DNS_FALLBACK_BS, 0);
    }
}
#endif

/******************************************************************************/

bool nabto_invite_event(nabto_packet_header* hdr)
{
    if (nmc.context.state != NABTO_AS_WAIT_BS &&
        nmc.context.state != NABTO_AS_WAIT_DNS_FALLBACK_BS &&
        nmc.context.state != NABTO_AS_WAIT_DNS_FALLBACK_UDP_BS) {
        NABTO_LOG_TRACE(("Received an invite response from the basestation but we are not waiting in any invite responses."));
        return false;
    }
    /* Receive Packet (2) */
    if (hdr->nsi_sp != 0 || hdr->flags != 1 ) {
        NABTO_LOG_TRACE(("Illegal header in U_INVITE response (2/0/1): (%i, %i, %i)", hdr->nsi_cp, hdr->nsi_sp, (int)hdr->flags));
    } else {
        uint8_t type;
        uint8_t* ptr = nabtoCommunicationBuffer + hdr->hlen;
        size_t res = nabto_rd_payload(ptr, nabtoCommunicationBuffer + hdr->len, &type); ptr += SIZE_PAYLOAD_HEADER;
        if (res != (NP_PAYLOAD_IPX_BYTELENGTH - NP_PAYLOAD_HDR_BYTELENGTH) || type != NP_PAYLOAD_TYPE_IPX) {
            NABTO_LOG_TRACE(("Illegal payload in U_INVITE response from BS(13/%i): %i/%i", NP_PAYLOAD_TYPE_IPX, (int) res, (int) type));
        } else {
            READ_U32(nmc.context.gsp.addr,        ptr); ptr += 4;
            READ_U16(nmc.context.gsp.port,        ptr); ptr += 2;
            READ_U32(nmc.context.globalAddress.addr, ptr); ptr += 4;
            READ_U16(nmc.context.globalAddress.port, ptr);
            NABTO_LOG_TRACE(("Using GSP at " PRIep, MAKE_EP_PRINTABLE(nmc.context.gsp)));
            /* the next packet is not sent to the sender of this packet (BS),
             * therefore the state change is the only action - the time_event
             * will send the next packet.
             */
            handle_ok_invite_event();
            return true;
        }
    }
    return false;
}

void handle_ok_invite_event(void)
{
    
    if (nmc.context.state == NABTO_AS_WAIT_BS) {
        SET_CTX_STATE_STAMP(NABTO_AS_WAIT_GSP, 0);
        return;
    }
    if (nmc.context.state == NABTO_AS_WAIT_DNS_FALLBACK_BS) {
#if NABTO_ENABLE_DNS_FALLBACK
        if (nmc.nabtoMainSetup.enableDnsFallback) {
            if (nmc.nabtoMainSetup.forceDnsFallback) {
                // force dns fallback
                SET_CTX_STATE_STAMP(NABTO_AS_WAIT_GSP, 0);
                return;
            }
        }
#endif
        nmc.context.useDnsFallback = false;
        SET_CTX_STATE_STAMP(NABTO_AS_WAIT_DNS_FALLBACK_UDP_BS, 0);
        return;
    }
    if (nmc.context.state == NABTO_AS_WAIT_DNS_FALLBACK_UDP_BS) {
        nmc.context.useDnsFallback = false;
        SET_CTX_STATE_STAMP(NABTO_AS_WAIT_GSP, 0);
        return;
    }
}

/******************************************************************************/
/**
 * Handle the attach event after initial protocol checking
 * @param hdr   the decoded packet header
 * @param ptr   pointer to reading position in received buffer
 * @return      true iff fully treated
 * A response may be sent
 */
static uint8_t handle_actual_attach(nabto_packet_header* hdr, uint8_t* ptr)
{
    uint8_t result = NP_PAYLOAD_ATTACH_STATS_STATUS_FAILED;
    uint8_t* end = nabtoCommunicationBuffer + hdr->len;
    uint16_t res;
    uint8_t type;

    uint8_t nonceGSP[NONCE_SIZE];
    uint8_t* seedGSP = 0;

    memcpy(/*ctx->*/nonceGSP, (const void*) ptr, nmc.context.nonceSize); ptr += nmc.context.nonceSize;
    res = nabto_rd_payload(ptr, end, &type); ptr += SIZE_PAYLOAD_HEADER;
    if (nmc.context.nonceSize == NONCE_SIZE_OLD) {
        NABTO_LOG_TRACE(("########    U_ATTACH without cryptographic payload received"));
        if (res != 12 || type != NP_PAYLOAD_TYPE_VERIFY || ptr + res != end) {
            NABTO_LOG_TRACE(("Illegal payload in U_ATTACH request(12/%d/12): %d/%d/%d", NP_PAYLOAD_TYPE_VERIFY, res, (int)type, (int)(end - ptr)));
            result = NP_PAYLOAD_ATTACH_STATS_STATUS_VERIFICATION_FAILED;
            goto final;
        } else {
            verification_add(&nmc.context.verif1, nabtoCommunicationBuffer, end - 12);
            verification_add(&nmc.context.verif2, nabtoCommunicationBuffer, end);
            if (!verification_check(&nmc.context.verif1, ptr, end)) {
                result = NP_PAYLOAD_ATTACH_STATS_STATUS_VERIFICATION_FAILED;
                goto final;
            }
        }
    } else if (nmc.context.nonceSize == NONCE_SIZE) {
        NABTO_LOG_TRACE(("########    U_ATTACH WITH cryptographic payload received"));
        if (type != NP_PAYLOAD_TYPE_CRYPTO || ptr + res != end ) {
            NABTO_LOG_TRACE(("Illegal payload in U_ATTACH request(len/ %i /avail): %i / %i / %td", NP_PAYLOAD_TYPE_CRYPTO, res, (int)type, (end - ptr)));
            goto final;
        } else {
            uint16_t code;
            uint16_t verifSize;
            READ_U16(code, ptr); ptr += 2;
            NABTO_LOG_TRACE(("before verify"));
            if (!unabto_verify_integrity(nmc.context.cryptoAttach, code, nabtoCommunicationBuffer, (uint16_t)(end - nabtoCommunicationBuffer), &verifSize)) {
                NABTO_LOG_TRACE(("U_ATTACH Integrity fail"));
                result = NP_PAYLOAD_ATTACH_STATS_STATUS_INTEGRITY_CHECK_FAILED;
                goto final;
            } else {
                NABTO_LOG_TRACE(("verifSize: %i end-ptr %td", verifSize, end-ptr));
                {
                    uint16_t dlen;
                    if (!unabto_decrypt(nmc.context.cryptoAttach, ptr, (uint16_t)(end - ptr - verifSize), &dlen)) {
                        NABTO_LOG_TRACE(("U_ATTACH Decryption fail"));
                        result = NP_PAYLOAD_ATTACH_STATS_STATUS_DECRYPTION_FAILED;
                        goto final;
                    } else if (dlen < NONCE_SIZE) {
                        NABTO_LOG_TRACE(("U_ATTACH Decryption fail, missing nonce data"));
                        result = NP_PAYLOAD_ATTACH_STATS_STATUS_NONCE_VERIFICATION_FAILED;
                        goto final;
                    } else if (memcmp((const void*) ptr, (const void*) nmc.context.nonceMicro, NONCE_SIZE) != 0) {
                        NABTO_LOG_TRACE(("U_ATTACH Decryption fail, challenge not met"));
                        result = NP_PAYLOAD_ATTACH_STATS_STATUS_NONCE_VERIFICATION_FAILED;
                        goto final;
                    } else if (dlen != NONCE_SIZE + SEED_SIZE) {
                        NABTO_LOG_TRACE(("U_ATTACH Decryption fail, missing seed data"));
                        result = NP_PAYLOAD_ATTACH_STATS_STATUS_NONCE_VERIFICATION_FAILED;
                        goto final;
                    }
                }
                seedGSP = ptr + NONCE_SIZE;
            }
        }
    }
    nmc.context.gspnsi = hdr->nsi_sp; /* to be required in next messages */
    NABTO_LOG_DEBUG(("GSP-ID(nsi): %u", nmc.context.gspnsi));
    if(send_gsp_attach_rsp(hdr->seq, nonceGSP, seedGSP)) { /* Send packet (5) */
        SET_CTX_STATE_STAMP(NABTO_AS_ATTACHED, nmc.nabtoMainSetup.gspPollTimeout);
        result =  NP_PAYLOAD_ATTACH_STATS_STATUS_OK;
    }

final:
    return result;
}

bool nabto_attach_event(nabto_packet_header* hdr)
{
    uint8_t result = NP_PAYLOAD_ATTACH_STATS_STATUS_FAILED;
    if (nmc.context.state != NABTO_AS_WAIT_GSP) {
        NABTO_LOG_TRACE(("Received an attach event from the GSP but we didn't expect such a packet so it will be discarded."));
        return false;
    }
    if (hdr->nsi_sp == 0) {
        NABTO_LOG_TRACE(("hdr->nsi_sp = 0"));
    }
    
    {
        /* Receive Packet (4) */
        uint8_t* end = nabtoCommunicationBuffer + hdr->len;
        
        uint16_t res = hdr->hlen;
        NABTO_LOG_TRACE(("received ATTACH event"));
        
        if (hdr->flags != 0) {
            NABTO_LOG_TRACE(("Illegal header in U_ATTACH request (2/0): %" PRIu32 "/%i", hdr->nsi_cp, (int)hdr->flags));
        } else {
            uint8_t type;
            uint8_t* ptr = nabtoCommunicationBuffer + res;
            res = nabto_rd_payload(ptr, end, &type); ptr += SIZE_PAYLOAD_HEADER;
            if (res != 6 || type != NP_PAYLOAD_TYPE_EP) {
                NABTO_LOG_TRACE(("Illegal payload in U_ATTACH request(6/%i): %i/%i", NP_PAYLOAD_TYPE_EP, res, (int)type));
            } else {
                nabto_endpoint ep;

                READ_U32(ep.addr, ptr); ptr += 4;
                READ_U16(ep.port, ptr); ptr += 2;
                NABTO_NOT_USED(ep); /* Needed to avoid warning -> error */

                /**
                 * If the endpoint given from the attach differs from the endpoint address 
                 * the controller gave then, we are behind a symmetric nat.
                 */
                if (!EP_EQUAL(ep, nmc.context.globalAddress)) {
                    nmc.context.natType = NP_PAYLOAD_IPX_NAT_SYMMETRIC;
                }

                NABTO_LOG_DEBUG(("nmc.ctx.privat     : " PRIep, MAKE_EP_PRINTABLE(nmc.socketGSPLocalEndpoint)));
                NABTO_LOG_DEBUG(("nmc.ctx.global     : " PRIep, MAKE_EP_PRINTABLE(nmc.context.globalAddress)));
                
                res = nabto_rd_payload(ptr, end, &type); ptr += SIZE_PAYLOAD_HEADER;
                // Here, we verify that the nonce is present and has the expected size
                if (res != nmc.context.nonceSize || type != NP_PAYLOAD_TYPE_NONCE) {
                    NABTO_LOG_TRACE(("Illegal payload in U_ATTACH request: %" PRIu16 " %i ",res ,(int)type));
                } else {
                    result = handle_actual_attach(hdr, ptr);
                }
            }
        }
    }
    if (result != NP_PAYLOAD_ATTACH_STATS_STATUS_OK) {
        send_basestation_attach_failure(result);
        return false;
    }
    return true;
}

/******************************************************************************/
/******************************************************************************/

bool nabto_alive_event(nabto_packet_header* hdr)
{
    if (nmc.context.state != NABTO_AS_ATTACHED) {
        NABTO_LOG_TRACE(("Received alive event, but we are not attached yet, so wwe discard the alive"));
        return false;
    }
    /* Receive Packet (6) */
    /* the packet usually contains a TIME payload */
    if (hdr->nsi_sp != nmc.context.gspnsi) {
        // probably from old attached context, this device is restarted, let caller log
        return false;
    }
    if (nmc.context.state != NABTO_AS_ATTACHED) {
        NABTO_LOG_TRACE(("Alive when not attached ignored"));
    } else if (hdr->flags != 0) {
        NABTO_LOG_TRACE(("Illegal header in U_ALIVE request:%" PRIu32 "/%i", hdr->nsi_cp, (int)hdr->flags));
    } else {
        size_t   olen;
        size_t   piggySize;
        uint8_t* piggyData;

#if NABTO_SET_TIME_FROM_ALIVE
        uint8_t  type;
        uint8_t* ptr = nabtoCommunicationBuffer + hdr->hlen;
        uint16_t res = nabto_rd_payload(ptr, nabtoCommunicationBuffer + hdr->len, &type); ptr += SIZE_PAYLOAD_HEADER;
        if (res < 5 || type != NP_PAYLOAD_TYPE_TIME) {
            NABTO_LOG_TRACE(("Illegal TIME in U_ALIVE request:%u %u", res, type));
        } else {
            uint8_t  timeType;
            uint32_t timeStamp;
            READ_U8(timeType, ptr); ptr += 1;
            READ_U32(timeStamp, ptr);
            NABTO_NOT_USED(timeType);
            setTimeFromGSP(timeStamp);
            NABTO_LOG_TRACE(("TIME in U_ALIVE request(%u): %u", (int)timeType, timeStamp));
        }
#endif
        NABTO_LOG_TRACE((PRInsi " Alive received", MAKE_NSI_PRINTABLE(hdr->nsi_cp, hdr->nsi_sp, 0)));

        NABTO_LOG_TRACE(("Sending ALIVE response to GSP s=%" PRIu16, hdr->seq));

        /* Send Packet (7): */

#if NABTO_ENABLE_EVENTCHANNEL
        /*
         * fill_event_buffer should return a pointer to a butter_t 
         * which can first be released when the application restarts or
         * 
         * We hold the eventdata in a buffer if we need to resend it later.
         */
        
        // create an input event buffer and fill it if there's data
        if (hdr->seq != nmc.context.piggyOldHeaderSequence) {
            // The old piggyback message has been received by the GSP, ask for a new one
            
            /**
             * The keep alive response consists of 
             *  * A minimal nabto header.
             *  * A notify payload
             *  * An optional piggyback message
             */
            const uint16_t maxPiggySize = nabtoCommunicationBufferSize - NP_PACKET_HDR_MIN_BYTELENGTH - NP_PAYLOAD_NOTIFY_BYTELENGTH - NP_PAYLOAD_PIGGY_SIZE_WO_DATA;

            nmc.context.piggyBuffer = get_event_buffer2(maxPiggySize);
        } // Else resend the old piggy data.

        if (nmc.context.piggyBuffer != NULL && nmc.context.piggyBuffer->size > 4) {
            NABTO_LOG_TRACE(("piggy buffer size %i, hdr->seq %i", nmc.context.piggyBuffer->size, hdr->seq));
            piggySize = nmc.context.piggyBuffer->size;
            piggyData = nmc.context.piggyBuffer->data;
        } else
#endif // NABTO_ENABLE_EVENTCHANNEL
        {
            piggySize = 0;
            piggyData = 0;
        }

        olen = mk_gsp_alive_rsp(hdr->seq, piggySize, piggyData);
        if (olen) {
            nmc.context.piggyOldHeaderSequence = hdr->seq;
            nabtoSetFutureStamp(&nmc.context.timestamp, nmc.nabtoMainSetup.gspPollTimeout);
            send_to_basestation(nabtoCommunicationBuffer, olen, &nmc.context.gsp);
            nmc.context.counter = 0;
        }
    }
    return true; /* accept or ignore silently */
}




/******************************************************************************/

void fix_for_broken_routers(void) {
    /**
     * fix for NABTO-1012 
     *
     * On some routers the nat mechanishm fails without any
     * notice. This means at some point a udp socket can go into a
     * state where it's possible to send data, but all received data
     * is blocked by the router.
     *
     * If there are no connections and we are not attached then it's
     * safe to refresh the remote communication socket.
     */
    if (unabto_count_active_connections() == 0) {
        nabto_close_socket(&nmc.socketGSP);
        nmc.socketGSPLocalEndpoint.port = 0;
        if (!nabto_init_socket(nmc.nabtoMainSetup.ipAddress, &nmc.socketGSPLocalEndpoint.port, &nmc.socketGSP)) {
            nmc.socketGSP = NABTO_INVALID_SOCKET;
        }
    }
}

/** timer event when still idle. */
void handle_as_idle(void) {
    if (!nmc.nabtoMainSetup.configuredForAttach) {
        SET_CTX_STATE_STAMP(NABTO_AS_IDLE, INTERVAL_INITIAL_CONFIGURATION_CHECK);
        return;
    }

    nabto_context_reinit_crypto();

#if NABTO_ENABLE_GET_LOCAL_IP
    nabto_get_local_ip(&nmc.socketGSPLocalEndpoint.addr);
#endif
    
    fix_for_broken_routers();
#if NABTO_ENABLE_DNS_FALLBACK
    if (nmc.nabtoMainSetup.enableDnsFallback) {
        if (nmc.context.hasDnsFallbackSocket) {
            unabto_dns_fallback_close_socket();
        }
    }
#endif
    nmc.controllerEp = nmc.nabtoMainSetup.controllerArg;
    if (nmc.nabtoMainSetup.controllerArg.addr != UNABTO_INADDR_NONE) {
        SET_CTX_STATE_STAMP(NABTO_AS_WAIT_BS, 0);
        return;
    }
    SET_CTX_STATE_STAMP(NABTO_AS_WAIT_DNS, 0);
    NABTO_LOG_INFO(("Resolving dns: %s", nmc.nabtoMainSetup.id));
    nabto_dns_resolve(nmc.nabtoMainSetup.id);
}


/** timer event when waiting for response to the DNS request. */
void handle_as_wait_dns(void) {
    nabto_dns_status_t dns_status = nabto_dns_is_resolved(nmc.nabtoMainSetup.id, &nmc.controllerEp.addr);
    switch (dns_status) {
    case NABTO_DNS_NOT_FINISHED:
        SET_CTX_STAMP(INTERVAL_DNS_RESOLVE);
        break;
    case NABTO_DNS_ERROR:
        NABTO_LOG_INFO(("DNS error (returned by application)"));
        SET_CTX_STATE_STAMP(NABTO_AS_IDLE, EXP_WAIT(INTERVAL_ERROR_RETRY_BASE, nmc.context.errorCount));
        send_basestation_attach_failure(NP_PAYLOAD_ATTACH_STATS_STATUS_DNS_LOOKUP_FAILED);
        nmc.context.errorCount += 2;
        break;
    case NABTO_DNS_OK:
        NABTO_LOG_TRACE(("Base station resolved to EP: " PRIep, MAKE_EP_PRINTABLE(nmc.controllerEp)));
        nmc.context.errorCount = 0;
        SET_CTX_STATE_STAMP(NABTO_AS_WAIT_BS, 0);
        break;
    }
}

/** timer event when waiting for the BS response to invite request. */
void handle_as_wait_bs(void) {
    
#if NABTO_ENABLE_DNS_FALLBACK
    if (nmc.nabtoMainSetup.enableDnsFallback) {
        if (nmc.nabtoMainSetup.forceDnsFallback) {
            // force dns fallback.
            SET_CTX_STATE_STAMP(NABTO_AS_WAIT_DNS_FALLBACK_OPEN_SOCKET, 0);
            return;
        }
    }
#endif
    SET_CTX_STATE_STAMP(NABTO_AS_WAIT_BS, EXP_WAIT(INTERVAL_BS_INVITE, nmc.context.counter));
    if(++nmc.context.counter > 6) {
        NABTO_LOG_INFO(("No answer from BS, trying with fallback via dns"));
        nmc.context.errorCount = 0;
#if NABTO_ENABLE_DNS_FALLBACK
        if (nmc.nabtoMainSetup.enableDnsFallback) {
            SET_CTX_STATE_STAMP(NABTO_AS_WAIT_DNS_FALLBACK_OPEN_SOCKET, INTERVAL_ERROR_RETRY_BASE);
            return;
        }
#endif
        // If dns fallback is not enabled then we end here.
        NABTO_LOG_INFO(("Could not connect to controller, retrying from beginning"));
        send_basestation_attach_failure(NP_PAYLOAD_ATTACH_STATS_STATUS_CONTROLLER_INVITE_FAILED);
        SET_CTX_STATE_STAMP(NABTO_AS_IDLE, INTERVAL_ERROR_RETRY_BASE);
        return;
    }
    
    send_controller_invite();
}

void handle_as_wait_dns_fallback_open_socket(void)
{
#if NABTO_ENABLE_DNS_FALLBACK
    SET_CTX_STATE_STAMP(NABTO_AS_WAIT_DNS_FALLBACK_OPEN_SOCKET, EXP_WAIT(INTERVAL_DNS_FALLBACK, nmc.context.counter));
    if(++nmc.context.counter > 6) {
        NABTO_LOG_INFO(("Dns fallback creation failed. Retrying from beginning"));
        send_basestation_attach_failure(NP_PAYLOAD_ATTACH_STATS_STATUS_CONTROLLER_INVITE_FAILED);
        SET_CTX_STATE_STAMP(NABTO_AS_IDLE, INTERVAL_ERROR_RETRY_BASE);
        return;
    }
    make_dns_fallback();
#endif
}

void handle_as_wait_dns_fallback_bs(void)
{
    SET_CTX_STATE_STAMP(NABTO_AS_WAIT_DNS_FALLBACK_BS, EXP_WAIT(INTERVAL_BS_INVITE, nmc.context.counter));
    if(++nmc.context.counter > 6) {
        NABTO_LOG_INFO(("No answer from BS via dns fallback, retrying from beginning"));
        nmc.context.errorCount = 0;
        send_basestation_attach_failure(NP_PAYLOAD_ATTACH_STATS_STATUS_CONTROLLER_INVITE_FAILED);
        SET_CTX_STATE_STAMP(NABTO_AS_IDLE, INTERVAL_ERROR_RETRY_BASE);
        return;
    }
    
    send_controller_invite();
}

void handle_as_wait_dns_fallback_udp_bs(void)
{
    SET_CTX_STATE_STAMP(NABTO_AS_WAIT_DNS_FALLBACK_UDP_BS, EXP_WAIT(INTERVAL_BS_INVITE, nmc.context.counter));
    if(++nmc.context.counter > 3) {
        NABTO_LOG_INFO(("No answer from BS via udp, deciding to use dns fallback"));
        nmc.context.errorCount = 0;
        nmc.context.counter = 0;
        nmc.context.useDnsFallback = true;
        SET_CTX_STATE_STAMP(NABTO_AS_WAIT_GSP, INTERVAL_ERROR_RETRY_BASE);
        return;
    }
    
    send_controller_invite();
}

/** timer event when waiting for the GSP response to attach request. */
void handle_as_wait_gsp(void) {
    if (++nmc.context.counter > 6) {
        NABTO_LOG_INFO(("No answer from GSP, retrying Base Station at " PRIep, MAKE_EP_PRINTABLE(nmc.controllerEp)));
        send_basestation_attach_failure(NP_PAYLOAD_ATTACH_STATS_STATUS_GSP_ATTACH_FAILED);
        SET_CTX_STATE_STAMP(NABTO_AS_IDLE, INTERVAL_ERROR_RETRY_BASE);
        return;
    }
    SET_CTX_STAMP(EXP_WAIT(INTERVAL_GSP_INVITE, nmc.context.counter) );

    send_gsp_invite();
}

/** timer event when attached. */
void handle_as_attached(void) {
    if (++nmc.context.counter >= nmc.nabtoMainSetup.gspTimeoutCount) {
        NABTO_LOG_INFO(("No keep-alive from GSP, retrying Base Station at " PRIep, MAKE_EP_PRINTABLE(nmc.controllerEp)));
        send_basestation_attach_failure(NP_PAYLOAD_ATTACH_STATS_STATUS_ATTACH_TIMED_OUT);
#if NABTO_ENABLE_DNS_FALLBACK
        if (nmc.nabtoMainSetup.enableDnsFallback) {
            if (nmc.context.hasDnsFallbackSocket) {
                unabto_dns_fallback_close_socket();
            }
        }
#endif
        nabto_context_reinit();
    } else {
        send_gsp_alive_poll();
    }
}

void nabto_attach_time_event(void)
{
    if (!nabtoIsStampPassed(&nmc.context.timestamp)) {
        return;
    }

    switch (nmc.context.state) {
    case NABTO_AS_IDLE:     handle_as_idle();     break;
    case NABTO_AS_WAIT_DNS: handle_as_wait_dns(); break;
    case NABTO_AS_WAIT_BS:  handle_as_wait_bs();  break;
    case NABTO_AS_WAIT_DNS_FALLBACK_OPEN_SOCKET: handle_as_wait_dns_fallback_open_socket(); break;
    case NABTO_AS_WAIT_DNS_FALLBACK_BS:          handle_as_wait_dns_fallback_bs();          break;
    case NABTO_AS_WAIT_DNS_FALLBACK_UDP_BS:      handle_as_wait_dns_fallback_udp_bs();      break;
    case NABTO_AS_WAIT_GSP: handle_as_wait_gsp(); break;
    case NABTO_AS_ATTACHED: handle_as_attached(); break;
    }
}

void nabto_network_changed(void) {
    switch (nmc.context.state) {
    case NABTO_AS_IDLE:
    case NABTO_AS_WAIT_DNS:
    case NABTO_AS_WAIT_BS:
    case NABTO_AS_WAIT_DNS_FALLBACK_OPEN_SOCKET:
    case NABTO_AS_WAIT_DNS_FALLBACK_BS:
    case NABTO_AS_WAIT_DNS_FALLBACK_UDP_BS:
    case NABTO_AS_WAIT_GSP:
        SET_CTX_STATE_STAMP(NABTO_AS_IDLE, 0);
        break;
    default: break;
    }
}

void nabto_change_context_state(nabto_state state)
{
    SET_CTX_STATE_STAMP(state, 0);
}


uint8_t* insert_attach_stats_payload(uint8_t* ptr, uint8_t* end, uint8_t statusCode) {
    uint8_t flags;

    if (end-ptr < NP_PAYLOAD_ATTACH_STATS_BYTELENGTH) {
        return NULL;
    }

    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_ATTACH_STATS, 0, 3);

    WRITE_FORWARD_U8(ptr, NP_PAYLOAD_ATTACH_STATS_VERSION);

    WRITE_FORWARD_U8(ptr, statusCode);

    flags = 0;

    if (nmc.context.nonceSize == NONCE_SIZE) {
        flags |= NP_PAYLOAD_ATTACH_STATS_FLAGS_SECURE_ATTACH;
    }

    WRITE_FORWARD_U8(ptr, flags);

    return ptr;
}

void send_basestation_attach_failure(uint8_t statusCode) {
    uint16_t length;
    uint8_t* ptr = insert_header(nabtoCommunicationBuffer, 0, 0, NP_PACKET_HDR_TYPE_STATS, false, 0, 0, 0);
    uint8_t* end = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;

    ptr = insert_stats_payload(ptr, end, NP_PAYLOAD_STATS_TYPE_UNABTO_ATTACH_FAILED);
    if (ptr == NULL) {
        return;
    }

    ptr = insert_version_payload(ptr, end);
    if (ptr == NULL) {
        return;
    }

    ptr = insert_sp_id_payload(ptr, end);
    if (ptr == NULL) {
        return;
    }

    ptr = insert_attach_stats_payload(ptr, end, statusCode);

    length = ptr - nabtoCommunicationBuffer;
    insert_length(nabtoCommunicationBuffer, length);

    if (nmc.controllerEp.addr == UNABTO_INADDR_NONE) {
        NABTO_LOG_TRACE(("There is no valid address to send statistics packets to"));
    } else {
        send_to_basestation(nabtoCommunicationBuffer, length, &nmc.controllerEp);
    }
}


#endif 
