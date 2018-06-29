/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_STREAM

/**
 * @file
 * Nabto uServer stream packet on unreliable con - Implementation.
 */
#if NABTO_ENABLE_STREAM_STANDALONE
#include "unabto_streaming_types.h"
#else
#include "unabto_env_base.h"
#endif

#if NABTO_ENABLE_STREAM && NABTO_ENABLE_MICRO_STREAM

#if NABTO_ENABLE_STREAM_STANDALONE
#include "unabto_streaming_types.h"
#else
#include "unabto_stream.h"
#include "unabto_stream_event.h"

#include "unabto_packet.h"

#include "unabto_util.h"
#include "unabto_main_contexts.h"
#include "unabto_memory.h"
#endif

#include <unabto/unabto_util.h>
#include <unabto/unabto_external_environment.h>
#include <unabto/unabto_stream_environment.h>
#include <unabto/unabto_stream_types.h>
#include <unabto/unabto_stream_window.h>
#include <unabto/unabto_protocol_defines.h>
#include <unabto/unabto_logging.h>
#include <unabto/unabto_next_event.h>

#include <string.h>
#include <math.h>

/** Limit a value. @param value the value. @param min lower limit. @param max upper limit. @return the limited value */
#define LIMITED(value, min, max)  ((value) >= (max) ? (max) : (value) <= (min) ? (min) : (value))

/// Difference (modulo 2**16) between sequence numbers.
/// @param s1  the largest sequence number
/// @param s2  the smallest sequence number
/// @return    the ddifference
#define SEQ_DIFF(s1, s2) ((uint16_t)(s1 - s2))

#define CWND_INITIAL_VALUE 4
#define SLOWSTART_MIN_VALUE 2


enum {
    //SEQ_EXT = 0x01,
    //ACK_EXT = 0x02,

    ACK     = NP_PAYLOAD_WINDOW_FLAG_ACK,
    RST     = NP_PAYLOAD_WINDOW_FLAG_RST,
    FIN     = NP_PAYLOAD_WINDOW_FLAG_FIN,
    SYN     = NP_PAYLOAD_WINDOW_FLAG_SYN
};

static bool nabto_stream_tcb_is_ack_on_fin(struct nabto_stream_tcb* tcb, struct nabto_win_info* win);
static bool nabto_stream_tcb_handle_fin(struct nabto_stream_tcb* tcb, struct nabto_win_info* win);

static void nabto_stream_check_new_data_xmit(struct nabto_stream_s* stream);
/**
 * return true if it's ok to see if new data can be transferred afterwards.
 * if return false, do not try to send new data, the communication layer is probably congested.
 */
static bool nabto_stream_check_retransmit_data(struct nabto_stream_s* stream);
static void unabto_stream_mark_segment_for_retransmission(struct nabto_stream_tcb* tcb, x_buffer* xbuf);
static void nabto_stream_state_transition(struct nabto_stream_s* stream, nabto_stream_tcb_state new_state);

/**
 * This function is used when enquing data into the transmit buffers
 */
static bool congestion_control_accept_more_data(struct nabto_stream_tcb* tcb);

static NABTO_THREAD_LOCAL_STORAGE uint16_t idSP__ = 0;       /**< the one and only */
static NABTO_THREAD_LOCAL_STORAGE uint16_t idCP__ = 0;       /**< the one and only */
static uint16_t nabto_stream_next_sp_id(void);
static uint16_t nabto_stream_next_cp_id(void);

static bool send_syn(struct nabto_stream_s* stream);
static bool send_syn_ack(struct nabto_stream_s* stream);
static bool send_syn_or_syn_ack(struct nabto_stream_s* stream, uint8_t type);

static bool unabto_stream_insert_sack_pair(uint32_t begin, uint32_t end, struct nabto_stream_sack_data* sackData);

void unabto_stream_dump_state(struct nabto_stream_s* stream);

/**
 * Initialize state
 */
static void nabto_init_stream_state(unabto_stream * stream, const struct nabto_win_info* info);

#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_DEBUG)
/**
 * convert a window type to a const string
 */
static const char* unabto_stream_type_to_string(uint8_t winType);
#endif

#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_TRACE)

static text stateNameW(nabto_stream_tcb_state state);

#endif

/******************************************************************************/
/******************************************************************************/

#ifndef LOG_STATE
#define LOG_STATE(s)
#endif

/**
 * Set streamState and log state change. @param s  the stream. @param newst  the new state
 */
#define SET_STATE(s, newst) \
    if (s->u.tcb.streamState != newst) { \
        NABTO_LOG_TRACE(("%" PRIu16 " STATE: %" PRItext " -> %" PRItext, s->streamTag, stateNameW(s->u.tcb.streamState), stateNameW(newst))); \
        nabto_stream_state_transition(s, newst); \
        LOG_STATE(s); \
    }

void nabto_stream_state_transition(struct nabto_stream_s* stream, nabto_stream_tcb_state new_state) {
    if (stream->u.tcb.streamState == new_state) return;
    stream->u.tcb.streamState = new_state;
    switch(new_state) {
        case ST_IDLE:
            break;
        case ST_SYN_SENT:
        case ST_SYN_RCVD:
            nabtoSetFutureStamp(&stream->u.tcb.timeoutStamp, 0);
            break;
        case ST_ESTABLISHED:
            // Ask the application to accept the stream;
            unabto_stream_accept(stream);
            break;
        case ST_FIN_WAIT_1:
            stream->u.tcb.retransCount = 0;
            nabtoSetFutureStamp(&stream->u.tcb.timeoutStamp, 0);
            stream->applicationEvents.writeClosed = true;
            break;
        case ST_FIN_WAIT_2:
            break;
        case ST_CLOSING:
            stream->applicationEvents.readClosed = true;
            break;
        case ST_TIME_WAIT:
            stream->u.tcb.retransCount = 0;
            nabtoSetFutureStamp(&stream->u.tcb.timeoutStamp, 0);
            stream->applicationEvents.readClosed = true;
            break;
        case ST_CLOSE_WAIT:
            stream->applicationEvents.readClosed = true;
        case ST_LAST_ACK:
            stream->u.tcb.retransCount = 0;
            nabtoSetFutureStamp(&stream->u.tcb.timeoutStamp, 0);
            stream->applicationEvents.writeClosed = true;
            break;
        case ST_CLOSED:
            stream->applicationEvents.closed = true;
            unabto_stream_send_stats(stream, NP_PAYLOAD_STATS_TYPE_DEVICE_STREAM_CLOSE);
            break;
        case ST_CLOSED_ABORTED:
            /**
             * Since we have aborted the stream in a unknown way so we
             * kills the connection. Such that the other end of the
             * stream knows that something not expected happened on
             * this stream.
             */
            // We cannot simply kill the connection since we maybe
            // just wanted to send an unsupported rst packet.
            // nabto_release_connection_req(stream->connection);
            stream->applicationEvents.closed = true;
            unabto_stream_send_stats(stream, NP_PAYLOAD_STATS_TYPE_DEVICE_STREAM_CLOSE);
            break;
    default:
        break;
    }
}

/**
 * Limit the stream to the least capabilities of the incoming syn and
 * our capabilities.
 */
void nabto_limit_stream_config(struct nabto_stream_tcb* tcb, const struct nabto_stream_tcb_config* newCfg)
{
    struct nabto_stream_tcb_config* initialCfg = &tcb->initialConfig;
    struct nabto_stream_tcb_config* cfg = &tcb->cfg;

    cfg->recvPacketSize = LIMITED(newCfg->recvPacketSize, 0, initialCfg->recvPacketSize);
    cfg->recvWinSize    = LIMITED(newCfg->recvWinSize,    1, initialCfg->recvWinSize);
    cfg->xmitPacketSize = LIMITED(newCfg->xmitPacketSize, 0, initialCfg->xmitPacketSize);
    cfg->xmitWinSize    = LIMITED(newCfg->xmitWinSize,    1, initialCfg->xmitWinSize);
    cfg->maxRetrans     = LIMITED(newCfg->maxRetrans,     0, initialCfg->maxRetrans);
    cfg->timeoutMsec    = LIMITED(newCfg->timeoutMsec,    initialCfg->timeoutMsec, initialCfg->timeoutMsec);
    cfg->enableWSRF     = newCfg->enableWSRF;
    cfg->enableSACK     = newCfg->enableSACK;
}

void nabto_init_stream_tcb_state(struct nabto_stream_tcb* tcb, const struct nabto_win_info* info, struct nabto_stream_s* stream)
{
    nabto_limit_stream_config(tcb, &info->u.syn.cfg);

    tcb->xmitSeq = 30;         /* some arbitrary value    */
    tcb->xmitFirst = tcb->xmitSeq;
    tcb->xmitLastSent = tcb->xmitSeq;
    tcb->recvNext = info->seq + 1; /* initial value in tcb */
    tcb->recvTop = tcb->recvNext;
    tcb->recvMax = tcb->recvNext;
    tcb->retransCount = 0;
    tcb->maxAdvertisedWindow = info->ack + stream->u.tcb.cfg.xmitWinSize;

    unabto_stream_congestion_control_init(tcb);
}

/**
 * This is called when a syn|ack is received on a stream.
 */
void nabto_limit_stream_config_syn_ack(struct nabto_stream_s* stream, const struct nabto_win_info* info)
{
    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    nabto_limit_stream_config(&stream->u.tcb, &info->u.syn.cfg);
    stream->idSP = info->idSP;
    // This is a bug it should have been info->seq+1
    stream->u.tcb.recvNext = info->seq;
    stream->u.tcb.recvTop = stream->u.tcb.recvNext;
    stream->u.tcb.recvMax = stream->u.tcb.recvNext;
    stream->u.tcb.xmitSeq++;

    tcb->xmitFirst = tcb->xmitSeq;
    tcb->xmitLastSent = tcb->xmitSeq;
    tcb->maxAdvertisedWindow = info->ack + stream->u.tcb.cfg.xmitWinSize;
}

void nabto_init_stream_state_initiator(struct nabto_stream_s* stream)
{
    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    tcb->cfg = tcb->initialConfig;
    tcb->xmitSeq = 10;
    tcb->retransCount = 0;
    unabto_stream_congestion_control_init(tcb);
}

static void reset_xbuf(x_buffer* xbuf) {
    if (xbuf->xstate != B_IDLE) {
        unabto_stream_free_send_segment(xbuf->buf);
        xbuf->buf = NULL;
    }
    memset(xbuf, 0, sizeof(x_buffer));
}

/******************************************************************************/

/**
 * The printable window statename. @param state  the state. @return the state name
 */
text stateNameW(nabto_stream_tcb_state state)
{
    switch(state) {
        case ST_IDLE:           return "wIDLE";
        case ST_SYN_SENT:       return "wSYN_SENT";
        case ST_SYN_RCVD:       return "wSYN_RCVD";
        case ST_ESTABLISHED:    return "wOPEN";
        case ST_FIN_WAIT_1:     return "wFIN_WAIT_1";
        case ST_FIN_WAIT_2:     return "wFIN_WAIT_2";
        case ST_CLOSING:        return "wCLOSING";
        case ST_TIME_WAIT:      return "wTIME_WAIT";
        case ST_CLOSE_WAIT:     return "wCLOSE_WAIT";
        case ST_LAST_ACK:       return "wLAST_ACK";
        case ST_CLOSED:         return "wCLOSED";
        case ST_CLOSED_ABORTED: return "wCLOSED_ABORTED";
        default:                return "w???";
    }
}

text nabto_stream_tcb_state_name(const struct nabto_stream_tcb* tcb) {
    return stateNameW(tcb->streamState);
}


void nabto_stream_tcb_open(struct nabto_stream_s* stream) {
    nabto_init_stream_state_initiator(stream);
    stream->idCP = nabto_stream_next_cp_id();
    stream->state = STREAM_IN_USE;
    SET_STATE(stream, ST_SYN_SENT);
    nabtoSetFutureStamp(&stream->u.tcb.timeoutStamp, 0);
}

/******************************************************************************/
/******************************************************************************/

size_t nabto_stream_tcb_read(struct nabto_stream_s * stream, const uint8_t** buf, unabto_stream_hint* hint)
{
    // recv while recvNext < recvTop


    if (nabto_stream_tcb_is_closed(stream)) {
        *hint =  UNABTO_STREAM_HINT_STREAM_CLOSED;
        return 0;
    }
    {
        struct nabto_stream_tcb* tcb = &stream->u.tcb;
        int ix = tcb->recvNext % tcb->cfg.recvWinSize;
        r_buffer* rbuf = &tcb->recv[ix];
        size_t avail = rbuf->size - rbuf->used;

        if (tcb->recvNext == tcb->recvFinSeq) {
            tcb->recvNext++;
            *hint = UNABTO_STREAM_HINT_READ_OR_WRITE_ERROR;
            return 0;
        } else if (tcb->recvNext == tcb->recvFinSeq+1) {
            *hint = UNABTO_STREAM_HINT_READ_OR_WRITE_ERROR;
            return 0;
        }

        *hint = UNABTO_STREAM_HINT_OK;
        if (avail > 0 && rbuf->seq == tcb->recvNext) {
            NABTO_LOG_TRACE(("Retrieving data from slot=%i size=%i", ix, (uint16_t)avail));
            *buf = (const uint8_t*) rbuf->buf + rbuf->used;
            NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_BUFFERS, ("Read from stream"), *buf, avail);
            return avail;
        }
    }
    return 0;
}

/******************************************************************************/

bool nabto_stream_tcb_ack(struct nabto_stream_s * stream, const uint8_t* buf, size_t used) {
    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    int ix = tcb->recvNext % tcb->cfg.recvWinSize;
    r_buffer* rbuf = &tcb->recv[ix];
    uint16_t avail = rbuf->size - rbuf->used;
    if (used > avail) {
        NABTO_LOG_ERROR(("Trying to acknowledge %u bytes from %p, but only %u was retrieved in last nabto_stream_read()",(int)used, buf, avail));
    } else if (buf != rbuf->buf + rbuf->used) {
        NABTO_LOG_ERROR(("Trying to acknowledge %u bytes from %p , but %p is awaiting acknowledgement", (int)used, buf, rbuf->buf + rbuf->used));
    } else {
        NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_BUFFERS, ("Ack on stream"), buf, used);

        if (used < avail) {
            rbuf->used += (uint16_t)used;
        } else {
            bool windowHasOpened;
            rbuf->size = rbuf->used = 0;
            tcb->recvNext++;  /* rolling the receive window because the slot has become idle */
            NABTO_LOG_TRACE(("slot %i is now empty, next slot %i, tcb->recvNext %u", ix, tcb->recvNext % tcb->cfg.recvWinSize, tcb->recvNext));
            windowHasOpened = tcb->cfg.enableWSRF && (tcb->cfg.xmitWinSize - ((tcb->recvTop - tcb->recvNext)) == 1);
            nabto_stream_tcb_check_xmit(stream, false, windowHasOpened);
        }
        return true;
    }
    return false;
}

/******************************************************************************/

size_t nabto_stream_tcb_write(struct nabto_stream_s * stream, const uint8_t* buf, size_t size) {

    size_t queued = 0;

    if (buf && size) {
        struct nabto_stream_tcb * tcb = &stream->u.tcb;
//        NABTO_DECLARE_LOCAL_MODULE(nabto::Log::STREAM);
        while (size && (tcb->xmitSeq < (tcb->xmitFirst + tcb->cfg.xmitWinSize)) &&
               (tcb->xmitSeq < tcb->maxAdvertisedWindow) &&
               congestion_control_accept_more_data(tcb))
        {
            uint16_t sz;
            int ix = tcb->xmitSeq % tcb->cfg.xmitWinSize;
            x_buffer* xbuf = &tcb->xmit[ix];
            if (xbuf->xstate != B_IDLE) {
                NABTO_LOG_FATAL(("xbuf->xstate != B_IDLE, xmitCount=%u finSent=%u", (int)tcb->xmitLastSent - tcb->xmitFirst, tcb->finSequence));
            }
            reset_xbuf(xbuf);

            sz = size > tcb->cfg.xmitPacketSize ? tcb->cfg.xmitPacketSize : (uint16_t)size;
            NABTO_LOG_TRACE(("-------- nabto_stream_write %i bytes, seq=%i into ix=%i", sz, tcb->xmitSeq, ix));

            xbuf->buf = unabto_stream_alloc_send_segment();
            if (xbuf->buf == NULL) {
                stream->blockedOnMissingSendSegment = true;
                break;
            }
            memcpy(xbuf->buf, (const void*) &buf[queued], sz);
            xbuf->size = sz;
            xbuf->seq = tcb->xmitSeq;
            xbuf->xstate = B_DATA;

            queued += sz;
            size -= sz;
            ++tcb->xmitSeq;   // done here or after sending?

            if ((tcb->xmitSeq - tcb->xmitFirst) == 1) {
                // restart retransmission timer
                NABTO_LOG_TRACE(("restart retransmission timer %" PRIu16, tcb->cCtrl.rto));
                nabtoSetFutureStamp(&tcb->dataTimeoutStamp, tcb->cCtrl.rto);
            }
        }
        if (queued) {
            NABTO_LOG_TRACE(("-------- nabto_stream_write calls nabto_stream_tcb_check_xmit"));
            nabto_stream_check_new_data_xmit(stream);
        }
    }

    stream->stats.sentBytes += queued;
    if (stream->stats.timeFirstMBSent == 0 && stream->stats.sentBytes >= 1048576) {
        stream->stats.timeFirstMBSent = unabto_stream_get_duration(stream);
    }
    return queued;
}

/******************************************************************************/

size_t nabto_stream_tcb_can_write(struct nabto_stream_s * stream) {
    struct nabto_stream_tcb * tcb = &stream->u.tcb;

    // same check as the while uses in send.
    if ((tcb->xmitSeq < (tcb->xmitFirst + tcb->cfg.xmitWinSize)) &&
        (tcb->xmitSeq < tcb->maxAdvertisedWindow) &&
        congestion_control_accept_more_data(tcb) &&
        unabto_stream_can_alloc_send_segment())
    {
        return tcb->cfg.xmitPacketSize;
    } else {
        return 0;
    }
}

/******************************************************************************/

/**
 * Build and send a data/ack-packet belonging to a message stream.
 * @param stream   the stream
 * @param seq      the sequence number
 * @param data     pointer to the data (may be 0 if size is 0)
 * @param size     number of bytes in data
 * @return         true iff sent (and then stream->ackSent is updated)
 *
 * Parameters
 * - retrans  the retransmission number (for logging)
 * - ix       the internal stream number (for logging)
 */
static bool send_data_packet(struct nabto_stream_s* stream, uint32_t seq, uint8_t* data, uint16_t size, unsigned int retrans, int ix)
{
    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    struct nabto_stream_sack_data sackData;
    uint32_t ackNumber = unabto_stream_ack_number_to_send(tcb);
    NABTO_NOT_USED(ackNumber);
    NABTO_NOT_USED(tcb);

    unabto_stream_create_sack_pairs(stream, &sackData);

    if (build_and_send_packet(stream, ACK, seq, 0, 0, data, size, &sackData)) {
        if (size) {
            tcb->cCtrl.cwnd -= 1;
        }

        NABTO_LOG_DEBUG(("%" PRIu16 " <-- [%" PRIu32 ",%" PRIu32 "] DATA(isRetrans: %u), %" PRIu16 " bytes from ix=%i (xmitFirst/Count/flightSize = %" PRIu32 "/%" PRIu32 "/%i), advertisedWindow %" PRIu16, stream->streamTag, seq, ackNumber, retrans, size, ix, tcb->xmitFirst, tcb->xmitLastSent - tcb->xmitFirst, tcb->cCtrl.flightSize, unabto_stream_advertised_window_size(tcb)));

        return true;
    } else {
        NABTO_LOG_TRACE(("Failed to send stream data packet!"));
    }
    return false;
}

/******************************************************************************/

static bool send_rst(struct nabto_stream_s* stream)
{
    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    NABTO_LOG_DEBUG(("%" PRIu16 " <-- [%" PRIu32 ",%" PRIu32 "] RST ", stream->streamTag, tcb->xmitSeq, unabto_stream_ack_number_to_send(tcb)));
    return build_and_send_packet(stream, RST, tcb->xmitSeq, NULL, 0, 0, 0, NULL);
}

static bool send_syn(struct nabto_stream_s* stream)
{
    struct nabto_stream_tcb * tcb = &stream->u.tcb;
    bool ret = send_syn_or_syn_ack(stream, (SYN));

    NABTO_NOT_USED(tcb);
    NABTO_LOG_DEBUG(("%" PRIu16 " <-- [%" PRIu32 ",%" PRIu32 "] SYN (RETRANS: %" PRIu16 ")",
                     stream->streamTag, tcb->xmitSeq, tcb->recvNext, tcb->retransCount));
    // Not using tcb->cfg.timeoutMsec here since we are negotiating
    // it with the other end.
    nabtoSetFutureStamp(&tcb->timeoutStamp, stream->staticConfig.defaultStreamTimeout);
    nabtoSetFutureStamp(&tcb->ackStamp, 2*stream->staticConfig.defaultStreamTimeout);
    return ret;
}

static bool send_syn_ack(struct nabto_stream_s* stream)
{
    struct nabto_stream_tcb * tcb = &stream->u.tcb;
    bool ret = send_syn_or_syn_ack(stream, (SYN|ACK));

    NABTO_NOT_USED(tcb);
    NABTO_LOG_DEBUG(("%" PRIu16 " <-- [%" PRIu32 ",%" PRIu32 "] SYN|ACK (RETRANS: %" PRIu16 ")",
                     stream->streamTag, tcb->xmitSeq, tcb->recvNext, tcb->retransCount));
    // Not using tcb->cfg.timeoutMsec here since we are negotiating it
    // with the other end. but since this is acknowledging the syn the
    // configuration has almost already happened.
    nabtoSetFutureStamp(&tcb->timeoutStamp, stream->staticConfig.defaultStreamTimeout);
    nabtoSetFutureStamp(&tcb->ackStamp, 2*stream->staticConfig.defaultStreamTimeout);
    return ret;
}

/**
 * Send the response to a SYN message.
 * @param stream  the stream
 * @return true if syn-ack send
 */
static bool send_syn_or_syn_ack(struct nabto_stream_s* stream, uint8_t type)
{
    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    uint16_t options = 0;
    uint8_t tmp[14];

    if (tcb->cfg.enableWSRF) {
        options |= NP_PAYLOAD_STREAM_FLAG_WSRF;
    }
    if (tcb->cfg.enableSACK) {
        options |= NP_PAYLOAD_STREAM_FLAG_SACK;
    }
    WRITE_U16(tmp +  0, options);
    WRITE_U16(tmp +  2, tcb->cfg.recvPacketSize);
    WRITE_U16(tmp +  4, tcb->cfg.recvWinSize);
    WRITE_U16(tmp +  6, tcb->cfg.xmitPacketSize);
    WRITE_U16(tmp +  8, tcb->cfg.xmitWinSize);
    WRITE_U16(tmp + 10, tcb->cfg.maxRetrans);
    WRITE_U16(tmp + 12, tcb->cfg.timeoutMsec);

    if (!build_and_send_packet(stream, type, tcb->xmitSeq, tmp, sizeof(tmp), 0, 0, NULL)) {
        return false;
    }

    NABTO_LOG_DEBUG(("%" PRIu16 " <-- [%" PRIu32 ",%" PRIu32 "] %" PRItext " (xmitFirst/Count/flightSize = %" PRIu32 "/%" PRIu32 "/%i), advertisedWindow %" PRIu16,
                     stream->streamTag,
                     tcb->xmitSeq,
                     unabto_stream_ack_number_to_send(tcb),
                     unabto_stream_type_to_string(type),
                     tcb->xmitFirst,
                     tcb->xmitLastSent - tcb->xmitFirst,
                     tcb->cCtrl.flightSize,
                     unabto_stream_advertised_window_size(tcb)));

    return true;
}

/******************************************************************************/

/**
 * Send a FIN ACK message.
 * @param stream  the stream
 * @return        true iff sent
 */
static bool send_FIN_ACK(struct nabto_stream_s* stream)
{
    struct nabto_stream_tcb* tcb = &stream->u.tcb;

    /**
     * We send a FIN | ACK after all outstanding data has been acked.
     * This means xmitFirst points to the window location we should send next.
     */

    uint32_t oldFinSequence = tcb->finSequence;
    uint32_t oldSeq = tcb->xmitFirst;
    uint32_t oldXmitSeq = tcb->xmitSeq;

    if (tcb->finSequence == 0) {
        tcb->finSequence = tcb->xmitSeq;
        tcb->xmitFirst = tcb->xmitSeq;
        tcb->xmitSeq++;
    }

    {
        // The fin has the sequence number xmitSeq-1
        if (build_and_send_packet(stream, FIN|ACK, tcb->finSequence, 0, 0, 0, 0, NULL)) {
            NABTO_LOG_DEBUG(("%i <-- [%i,%i] FIN|ACK, RETRANS: %i", stream->streamTag, tcb->finSequence, tcb->recvNext, tcb->retransCount));

            tcb->xmitLastSent = tcb->finSequence+1;
            return true;
        } else {
            // reset the state
            tcb->finSequence = oldFinSequence;
            tcb->xmitFirst = oldSeq;
            tcb->xmitSeq = oldXmitSeq;
        }
    }
    return false;
}

/******************************************************************************/

bool nabto_stream_check_retransmit_data(struct nabto_stream_s* stream) {
    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    uint32_t i;
    int ix;
    x_buffer* xbuf;
    ix = tcb->xmitFirst % tcb->cfg.xmitWinSize;
    xbuf = &tcb->xmit[ix];

    // Check if we have any sent but unacked data.
    if (xbuf->xstate != B_SENT) {
        return true;
    }

    if (unabto_stream_is_connection_reliable(stream)) {
        // We should not retransmit data on a reliable connection.
        return true;
    }

    if (tcb->retransmitSegmentCount == 0) {
        return true;
    }
    
    for (i = tcb->xmitFirst; i < tcb->xmitLastSent; i++) {
        int ix = i % tcb->cfg.xmitWinSize;
        xbuf = &tcb->xmit[ix];

        if (tcb->retransmitSegmentCount == 0) {
            return true;
        }
        
        if (!unabto_stream_congestion_control_can_send(tcb)) {
            return false;
        }

        // Check if we are above the most recent sent buffer
        // if so there can be no retransmits since the data has not been sent.

        if (xbuf->xstate == B_SENT && xbuf->shouldRetransmit) {
            xbuf->nRetrans++;
            if (!send_data_packet(stream, xbuf->seq, xbuf->buf, xbuf->size, xbuf->nRetrans, ix)) {

                // we can't send data do not try to send more data.
                xbuf->nRetrans--;
                return false;
            }

            stream->stats.sentResentPackets++;

            xbuf->shouldRetransmit = false;
            tcb->retransmitSegmentCount--;
            xbuf->sentStamp = nabtoGetStamp();
            xbuf->ackedAfter = 0;
        }
    }
    return true;
}


void nabto_stream_check_new_data_xmit(struct nabto_stream_s* stream) {
    // Check for sending data which has not been sent the first time yet.
    // This should be called when new data is queued, data is acked or
    // other events which could change the amount of available data
    // or the flight size.

    // When a packet is sent and changes status from B_DATA to B_SENT
    // remember to increment xmitLastSent.

    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    uint32_t i;

    UNABTO_ASSERT(tcb->xmitFirst <= tcb->xmitLastSent);
    UNABTO_ASSERT(tcb->xmitLastSent <= tcb->xmitSeq);
    for (i = tcb->xmitLastSent; i < tcb->xmitSeq; i++) {
        int ix = i % tcb->cfg.xmitWinSize;
        x_buffer* xbuf = &tcb->xmit[ix];
        if (xbuf->xstate != B_DATA) {
            // No more fresh data to send.
            break;
        }

        if (!(unabto_stream_congestion_control_can_send(tcb))) {
            NABTO_LOG_TRACE(("Congestion control limits data sending"));
            break;
        } else if (!send_data_packet(stream, xbuf->seq, xbuf->buf, xbuf->size, xbuf->nRetrans, ix)) {
            // we can't send data do not try to send more data.
            NABTO_LOG_TRACE(("Cannot send data, probably because of congestion."));
            break;
        }

        // data was sent increment xmitLastSent etc.
        {
            // flightSize is exactly the number of transmitbuffers in the state B_SENT!
            tcb->cCtrl.flightSize++;
            unabto_stream_stats_observe(&tcb->ccStats.flightSize, (double)tcb->cCtrl.flightSize);
            xbuf->xstate = B_SENT;
        }
        xbuf->shouldRetransmit = false;
        xbuf->ackedAfter = 0;

        tcb->xmitLastSent = i+1; // +1 since it's the sequence number right after the last sent.
        xbuf->sentStamp = nabtoGetStamp();
    }
}

void unabto_stream_mark_all_xmit_buffers_as_timed_out(struct nabto_stream_tcb* tcb) {
    uint32_t i;
    int ix;
    x_buffer* xbuf;

    for (i = tcb->xmitFirst; i < tcb->xmitLastSent; i++) {
        ix = i % tcb->cfg.xmitWinSize;
        xbuf = &tcb->xmit[ix];
        if (xbuf->xstate == B_SENT) {
            unabto_stream_mark_segment_for_retransmission(tcb, xbuf);
        }
    }
}

void nabto_stream_tcb_check_xmit(struct nabto_stream_s* stream, bool isTimedEvent, bool windowHasOpened)
{
    struct nabto_stream_tcb* tcb = &stream->u.tcb;

    if (tcb->streamState == ST_CLOSED || tcb->streamState == ST_CLOSED_ABORTED) {
        return;
    }

    switch (tcb->streamState) {
        case ST_SYN_SENT:
            if (nabtoIsStampPassed(&tcb->timeoutStamp)) {
                if (tcb->retransCount > tcb->cfg.maxRetrans) {
                    NABTO_LOG_ERROR(("retransCount > tcb->cfg.maxRetrans, releasing the stream"));
                    unabto_stream_release(stream);
                } else {
                    if (send_syn(stream)) {
                        tcb->retransCount++;
                    }
                }
            }
            break;
        case ST_SYN_RCVD:
            if (nabtoIsStampPassed(&tcb->timeoutStamp)) {
                if (tcb->retransCount > tcb->cfg.maxRetrans) {
                    NABTO_LOG_INFO(("retransCount > tcb->cfg.maxRetrans, releasing the stream"));
                    // The stream is first accepted and hence released by an application after
                    // the state has passed ST_SYN_RCVD.
                    // Instead of switching to ST_CLOSED just release the resource.
                    unabto_stream_release(stream);
                } else {
                    if (send_syn_ack(stream)) {
                        tcb->retransCount++;
                    }
                }
            }
            break;

        case ST_ESTABLISHED:
        case ST_FIN_WAIT_1:
        case ST_CLOSING:
        case ST_CLOSE_WAIT:
        case ST_LAST_ACK:
            // after fin has been sent, no more data is going to be sent.
            if (tcb->finSequence == 0) {
                if (isTimedEvent) {
                    if ((tcb->xmitLastSent > tcb->xmitFirst) && nabtoIsStampPassed(&tcb->dataTimeoutStamp)) {
                        unabto_stream_congestion_control_timeout(stream);
                        nabtoSetFutureStamp(&tcb->dataTimeoutStamp, tcb->cCtrl.rto);
                        unabto_stream_mark_all_xmit_buffers_as_timed_out(tcb);
                    }
                }
                if (nabto_stream_check_retransmit_data(stream)) {
                    // no reason to check xmit of new data if old data cannot be sent.
                    nabto_stream_check_new_data_xmit(stream);
                }
            }
            break;
        case ST_TIME_WAIT:
            if (nabtoIsStampPassed(&tcb->timeoutStamp)) {
                SET_STATE(stream, ST_CLOSED);
            }
            break;


        default:
            break;
    }

    /* Check for rensend of FIN */
// TODO
    switch (tcb->streamState) {
        case ST_FIN_WAIT_1:
        case ST_CLOSING:
        case ST_LAST_ACK:
            /**
             * A FIN has been sent else check that all outstanding
             * data has been acked and send a FIN.
             */
            if (tcb->finSequence != 0 ||
                (tcb->xmitSeq == tcb->xmitFirst) ) {
                if (nabtoIsStampPassed(&tcb->timeoutStamp)) {
                    if (tcb->retransCount > tcb->cfg.maxRetrans) {
                        NABTO_LOG_INFO(("retransCount > tcb->cfg.maxRetrans, releasing the stream"));
                        SET_STATE(stream, ST_CLOSED);
                    } else {
                        if (send_FIN_ACK(stream)) {
                            nabtoSetFutureStamp(&tcb->timeoutStamp, tcb->cfg.timeoutMsec);
                            tcb->retransCount++;
                        }
                    }
                }
            }
            break;
        default: // Compiler warning removal.
            break;

    }
    if (tcb->streamState >= ST_ESTABLISHED) {
        /* Check for sending an ACK without data */
        {
            bool ackReadyForConsumedData = (tcb->ackSent != tcb->recvTop);

            bool sendWSRFAck = (tcb->cfg.enableWSRF && (tcb->ackWSRFcount < tcb->cfg.maxRetrans) && nabtoIsStampPassed(&tcb->ackStamp));

            if (ackReadyForConsumedData ||
                windowHasOpened ||
                sendWSRFAck) {
                if (send_data_packet(stream, tcb->xmitLastSent, 0, 0, 0, 0)) {
                    if (ackReadyForConsumedData) {
                        tcb->ackWSRFcount = 0;
                    } else {
                        ++tcb->ackWSRFcount;
                        NABTO_LOG_TRACE(("WSRF: EXTRA ACK (%i) %i", tcb->ackWSRFcount, tcb->recvNext));
                    }
                }
            }
        }
    }

    // Set timestamps in the future if they are irrelevant
    // s.t. an event based approach is possible.
    if (nabtoIsStampPassed(&tcb->timeoutStamp)) {
        nabtoSetFutureStamp(&tcb->timeoutStamp, tcb->cfg.timeoutMsec);
    }
    if (nabtoIsStampPassed(&tcb->ackStamp)) {
        nabtoSetFutureStamp(&tcb->ackStamp, tcb->cfg.timeoutMsec);
    }
}

uint16_t unabto_stream_advertised_window_size(struct nabto_stream_tcb* tcb)
{
    uint16_t recvWinSize = tcb->cfg.recvWinSize;
    if (tcb->cfg.enableWSRF && tcb->streamState >= ST_ESTABLISHED) {
        /**
         * Calculate the number of used receive slots and tell the
         * other end how large our window is right now.
         */
        recvWinSize -= (tcb->recvTop - tcb->recvNext) ;
    }
    return recvWinSize;
}

uint32_t unabto_stream_ack_number_to_send(struct nabto_stream_tcb* tcb)
{
    uint32_t recvNext = tcb->recvNext;
    if (tcb->cfg.enableWSRF && tcb->streamState >= ST_ESTABLISHED) {
        recvNext = tcb->recvTop;
    }
    return recvNext;
}
/******************************************************************************/

/** Classification of received sequence number */
typedef enum { SEQ_EXPECTED, SEQ_RETRANS, SEQ_INVALID } recv_seq_t;

/**
 * Validate the xmit sequence number.
 * @param stream  the stream
 * @param seq     the sequence number
 * @param dlen    the length of data
 * @return
 */
static recv_seq_t accept_xmit_seq(struct nabto_stream_tcb* tcb, uint32_t seq, int dlen)
{
    uint32_t limit = tcb->cfg.recvWinSize;
    /**
     * If dlen == 0 then we should also accept a sequence number which is
     * one higher than the current receive windows since it's allowed to
     * send acks with sequence numer set to one about the current maximum
     * sequence number in the window.
     */
    if (dlen == 0) {
        limit += 1;
    }
    NABTO_LOG_TRACE(("Accept_xmit_seq, seq: %i, dlen %i ", seq, dlen));
    /* NOTE: The code below only works because we are using unsigned
     * arithmetics. The code will break for signed integers.
     */
    /* EQUIV: if (tcb->recvNext <= seq && seq < tcb->recvNext + limit) */
    if (seq - tcb->recvNext < limit) {
        return SEQ_EXPECTED;
    }

    /* EQUIV: if (tcb->recvNext - limit <= seq && seq <= tcb->recvNext) */
    if (tcb->recvNext - seq <= limit) {
        NABTO_LOG_DEBUG(("Data retransmission, seq: %i, expect: [%i..%i]", seq, tcb->recvNext, tcb->recvNext + limit - 1));
        return SEQ_RETRANS;
    }
    NABTO_LOG_ERROR(("Bad seq: %" PRIu32 ", expect: [%" PRIu32 "..%" PRIu32 "%" PRItext, seq, tcb->recvNext, tcb->recvNext + limit, (dlen == 0) ? "]" : "["));
    return SEQ_INVALID;
}

/******************************************************************************/

void update_ack_after_packet(struct nabto_stream_s* stream, uint32_t seq) {
    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    uint32_t xmitFirstSeq = tcb->xmitFirst;
    uint16_t ixOfAckedSeq = seq % tcb->cfg.xmitWinSize;
    uint32_t i;

    nabto_stamp_t sentStampOfAckedSeq = tcb->xmit[ixOfAckedSeq].sentStamp;

    for (i = xmitFirstSeq; i < seq; i++) {
        nabto_stamp_t sentStampOfThisSeq;
        uint16_t ix = i % tcb->cfg.xmitWinSize;
        x_buffer* xbuf = &tcb->xmit[ix];
        if (xbuf->xstate != B_SENT) {
            continue;
        }
        sentStampOfThisSeq = xbuf->sentStamp;
        // If the windows packet was sent before the packet was sent
        // this ack acks, then count the window packets unordered ack
        // count.
        if (nabtoStampLess(&sentStampOfThisSeq, &sentStampOfAckedSeq)) {
            NABTO_LOG_TRACE(("Increment ACK after for seq %i beacuse ack on %i was received", i, seq));
            xbuf->ackedAfter++;
            if (xbuf->ackedAfter == 2) {
                unabto_stream_congestion_control_adjust_ssthresh_after_triple_ack(tcb);
                stream->stats.reorderedOrLostPackets++;
                unabto_stream_mark_segment_for_retransmission(tcb, xbuf);
            }
        }
    }
}

/**
 * we have received an ack for the top of the receive window.
 */
bool update_triple_ack(struct nabto_stream_s* stream, uint32_t ackSeq)
{
    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    uint16_t ix = ackSeq % tcb->cfg.xmitWinSize;
    x_buffer* xbuf = &tcb->xmit[ix];
    if (xbuf->xstate == B_SENT && xbuf->nRetrans == 0) {
        xbuf->ackedAfter++;
        if (xbuf->ackedAfter == 2) {
            unabto_stream_congestion_control_adjust_ssthresh_after_triple_ack(tcb);
            stream->stats.reorderedOrLostPackets++;
            unabto_stream_mark_segment_for_retransmission(tcb, xbuf);
        }
    }
    return (xbuf->ackedAfter >= 2);
}

/**
 * ack data
 * @param ack_start start of area of the sequence numbers which is acked
 * @param ack_end   end of area of sequence numbers which is acked
 * @return true if data has been acked.
 */
bool handle_ack(struct nabto_stream_s* stream, uint32_t ack_start, uint32_t ack_end) {
    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    bool dataAcked = false;
    uint32_t xmitFirstSeq = tcb->xmitFirst;

    if (ack_end < ack_start) {
        return false;
    }

    // This is the first unacknowledged packet sequence number in the window.


    NABTO_LOG_TRACE(("%" PRIu16 " acking: [%" PRIu32 "..%" PRIu32 "] xmitFirstSeq: %" PRIu32, stream->streamTag, ack_start, ack_end, xmitFirstSeq));
    // Check that what we are acking is contained in the current active window.


    if (ack_start >= xmitFirstSeq) { // we are in the window
        uint32_t i;
        // Check that we are not acking more than is actually sent.

        for (i = ack_start; i < ack_end; i++) {
            int ix = i % tcb->cfg.xmitWinSize;
            bool ackOnStartOfWindow = (i == tcb->xmitFirst);
            x_buffer* xbuf = &tcb->xmit[ix];

            if (i > tcb->xmitLastSent) {
                NABTO_LOG_ERROR(("trying to ack segment which has not been sent yet"));
                break;
            }

            // Only update the last segment in the ack sequence since
            // it will have the most correct timing.
            if (i == tcb->xmitFirst && i == (ack_end - 1)) {
                unabto_stream_update_congestion_control_receive_stats(stream, ix);
            }

            // Mark segment as acked.
            if (xbuf->xstate == B_SENT) {
                update_ack_after_packet(stream, i);
                unabto_stream_congestion_control_handle_ack(tcb, ix, ackOnStartOfWindow);
                NABTO_LOG_TRACE(("setting buffer %i for seq %i to IDLE", ix, i));
                {
                    // flightSize is exactly the number of transmitbuffers in the state B_SENT!
                    tcb->cCtrl.flightSize--;
                    unabto_stream_stats_observe(&tcb->ccStats.flightSize, (double)tcb->cCtrl.flightSize);
                    xbuf->xstate = B_IDLE;
                    unabto_stream_free_send_segment(xbuf->buf);
                    xbuf->buf = NULL;
                    if (xbuf->shouldRetransmit) {
                        tcb->retransmitSegmentCount--;
                        xbuf->shouldRetransmit = false;
                    }
                }
                dataAcked = true;
            }
            // If we are acking in the start of the window we need to
            // increase the pointers and decrease counts to move the window.
            if (ackOnStartOfWindow) {
                NABTO_LOG_TRACE(("ix == tcb->xmitFirst"));

                tcb->xmitFirst++;

                // reset data timeout since we have received ack on data.
                nabtoSetFutureStamp(&tcb->dataTimeoutStamp, tcb->cCtrl.rto);

                dataAcked = true;
            }
        }
    }
    return dataAcked;
}

/**
 * return true if data has been acked.
 */
static bool handle_sack(struct nabto_stream_s* stream, struct nabto_stream_sack_data* sackData) {
    uint8_t i;
    bool dataAcked = false;
    for (i = 0; i < sackData->nPairs; i++) {
        dataAcked |= handle_ack(stream, sackData->pairs[i].start, sackData->pairs[i].end);
    }
    return dataAcked;
}

/**
 * Return true if we have changed the window such that we can
 * potentially send new data.
 */
static void handle_data(struct nabto_stream_s* stream,
                        struct nabto_win_info* win,
                        const uint8_t *        start,
                        int                    dlen,
                        struct nabto_stream_sack_data* sackData) {
    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    r_buffer*        rbuf;
    switch (accept_xmit_seq(tcb, win->seq, dlen)) {
        case SEQ_RETRANS:
            /* Be sure to send/resend an ACK, earlier ACK may be lost */
            if (dlen == 0) {
                // this is just an ack retransmission
            } else {
                tcb->ackSent = tcb->recvNext-1;
            }
            break;

        case SEQ_EXPECTED:
            /* use ack's from received packet to roll transmit window */
            NABTO_LOG_TRACE(("xmitLastSent %" PRIu32 " xmitFirst %" PRIu32, tcb->xmitLastSent, tcb->xmitFirst));


            NABTO_LOG_TRACE(("current advertisedWindow %" PRIu32 " advertised window from packet %" PRIu32, tcb->maxAdvertisedWindow, win->ack+win->advertisedWindow));


            if (tcb->xmitLastSent != tcb->xmitFirst) {
                uint32_t ackStart = tcb->xmitFirst; // first seq awaiting ack
                uint32_t ackEnd = win->ack;


                if (ackStart == ackEnd &&
                    (sackData == NULL || (sackData != NULL && sackData->nPairs == 0))) {
                    // update ack status to detect triple acks. based
                    // on the win->ack. If the packet contains sack
                    // data use that instead.

                    // this should only be called if the packet contains data.
                    if (dlen == 0) {
                        update_triple_ack(stream, ackStart);
                    }
                }

                if (handle_ack(stream, ackStart, ackEnd)) {
                    stream->applicationEvents.dataWritten = true;
                }
            }



            /* if possible insert data in receive window */
            if (dlen > 0) {
                int ix = (int)(win->seq % tcb->cfg.recvWinSize);
                rbuf = &tcb->recv[ix];
                if (tcb->recvNext > win->seq) {
                    NABTO_LOG_DEBUG(("retransmission of %u which is consumed.", win->seq));
                    stream->stats.receivedResentPackets++;
                    if (memcmp((const void*) start, (const void*) rbuf->buf, dlen) != 0) {
                        NABTO_LOG_DEBUG(("retransmitted data was wrong"));
                        NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_BUFFERS, ("data in window"),rbuf->buf, rbuf->size);
                        NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_BUFFERS, ("data in retransmission"), start, dlen);
                    }
                } else if (rbuf->size != 0) {
                    NABTO_LOG_DEBUG(("Retransmission seq: %i", win->seq));
                    if (dlen != (int)rbuf->size) {
                        NABTO_LOG_WARN(("Retransmission seq %u size mismatch, got: %u expect: %u", win->seq, dlen, rbuf->size));
                    }
                    else if (memcmp((const void*) start, (const void*) rbuf->buf, dlen) != 0) {
                        NABTO_LOG_WARN(("Retransmission seq %u data mismatch, size: %u expect %u", win->seq, dlen, rbuf->seq));
                    }
                } else {
                    NABTO_LOG_DEBUG(("%" PRIu16 "     Data=%i bytes, seq=%i inserted into slot=%i",stream->streamTag, dlen, win->seq, ix));
                    stream->stats.receivedBytes += dlen;
                    if (stream->stats.timeFirstMBReceived == 0 && stream->stats.receivedBytes >= 1048576) {
                        stream->stats.timeFirstMBReceived = unabto_stream_get_duration(stream);
                    }
                    memcpy(rbuf->buf, (const void*) start, dlen);
                    rbuf->size = dlen;
                    rbuf->used = 0;
                    rbuf->seq = win->seq;
                    LOG_STATE(stream);
                    // update the highest received data sequence number, used in sack calculation.
                    tcb->recvMax = MAX(tcb->recvMax, win->seq);
                }

                // force sending an ack, if this data is in line,
                // fine, if it's out of order, then the other end
                // should be notified by tripple acks. If this is a
                // retransmission maybe the ack has been lost and the
                // other end should have a new ack. So whatever the
                // cause, answer data with an ack.
                tcb->ackSent--;



                // update recvTop which is the max available sequence number for receiving data from.
                while (true) {
                    r_buffer* b = &tcb->recv[tcb->recvTop % tcb->cfg.recvWinSize];
                    if (b->seq != tcb->recvTop) {
                        break;
                    }
                    tcb->recvTop++;
                    stream->applicationEvents.dataReady = true;
                }
            }

            // Handle eventual sack data..
            if (handle_sack(stream, sackData)) {
                stream->applicationEvents.dataWritten = true;
            }
            break;

        case SEQ_INVALID:
            break;
    }

    nabto_stream_tcb_check_xmit(stream, false, false);
}

static void handle_ack_packet(struct nabto_stream_s* stream,
                              struct nabto_win_info* win,
                              struct nabto_stream_sack_data* sackData) {
    handle_data(stream, win, 0, 0, sackData);
}


static void nabto_stream_tcb_event_error(struct nabto_stream_s* stream, struct nabto_win_info* win) {
    NABTO_LOG_ERROR(("Stream in state %i could not handle window of type %i, ack %i, seq %i", stream->u.tcb.streamState, win->type, win->ack, win->seq));
}

void nabto_stream_tcb_event(struct nabto_stream_s* stream,
                            struct nabto_win_info* win,
                            const uint8_t*         start,
                            int                    dlen,
                            struct nabto_stream_sack_data* sackData)
{
    struct nabto_stream_tcb* tcb = &stream->u.tcb;

    NABTO_LOG_DEBUG(("%" PRIu16 " --> [%" PRIu32 ",%" PRIu32 "] %" PRItext ", advertisedWindow %" PRIu16 ", %d bytes", stream->streamTag, win->seq, win->ack, unabto_stream_type_to_string(win->type), win->advertisedWindow, dlen));

    if (win->type == ACK ) {
        uint32_t oldAdvWindow = tcb->maxAdvertisedWindow;
        // all data has been acked and there is no more room for more data
        bool windowWasFull = (tcb->maxAdvertisedWindow == tcb->xmitFirst);

        tcb->maxAdvertisedWindow = MAX(tcb->maxAdvertisedWindow, win->ack+win->advertisedWindow);
        if (oldAdvWindow != tcb->maxAdvertisedWindow) {
            // trigger sending more data since the window has opened
            stream->applicationEvents.dataWritten = true;
        }
        if (windowWasFull) {
            tcb->cCtrl.cwnd += (tcb->maxAdvertisedWindow-oldAdvWindow);
        }
    }

    if (win->type & RST) {
        if (tcb->streamState != ST_CLOSED || tcb->streamState != ST_CLOSED_ABORTED) {
            SET_STATE(stream, ST_CLOSED_ABORTED);
        }
        return;
    }

    if (tcb->streamState == ST_CLOSED || tcb->streamState == ST_CLOSED_ABORTED) {
        send_rst(stream);
        return;
    }

    switch (tcb->streamState) {
        case ST_IDLE:
            if (win->type == SYN) {
                nabto_init_stream_state(stream, win); // calls nabto_init_stream_tcb_state
                SET_STATE(stream, ST_SYN_RCVD);
            } else {
                nabto_stream_tcb_event_error(stream, win);
            }
            break;

        case ST_SYN_SENT:
            // This state is only possible for an initiator.
            if (win->type == (SYN|ACK)) {
                nabto_limit_stream_config_syn_ack(stream, win);
                unabto_stream_init_buffers(stream);
                SET_STATE(stream, ST_ESTABLISHED);
                // Due to earlier design desisions an ack needs to be
                // sent without data such that the receiver will
                // change state to ST_ESTABLISHED
                send_data_packet(stream, tcb->xmitSeq, 0, 0, 0, 0);
            }
            break;

        case ST_SYN_RCVD:
            // This state is only possible for an !initiator
            if (win->type == ACK || win->type == (SYN|ACK)) {
                if (win->seq == tcb->recvNext && win->ack == tcb->xmitSeq) {
                    unabto_stream_init_buffers(stream);
                    SET_STATE(stream, ST_ESTABLISHED);
                }
            } else if (win->type == (FIN | ACK)) {
                if (win->seq == tcb->recvNext && win->ack == tcb->xmitSeq) {
                    SET_STATE(stream, ST_CLOSED);
                }
            } else if (win->type == SYN) {
                //  retransmission of the syn packet.
                NABTO_LOG_TRACE(("Syn packet retransmission"));
            } else {
                nabto_stream_tcb_event_error(stream, win);
            }
            break;

        case ST_ESTABLISHED:
            // A syn ack would indicate that an ack is missing.
            // handling it here will trigger an ack retransmission.
            if ((win->type == ACK) || (win->type == (FIN | ACK)) || (win->type == (SYN|ACK))) {
                handle_data(stream, win, start, dlen, sackData); /* sequence numbers are checked by caller */

                if (win->type == (FIN | ACK)) {
                    if(nabto_stream_tcb_handle_fin(tcb, win)) {
                        SET_STATE(stream, ST_CLOSE_WAIT);
                    }
                }
            } else {
                nabto_stream_tcb_event_error(stream, win);
            }

            break;

        case ST_LAST_ACK:
            // The peer has closed for sending anymore data
            // but acks still come through.

            if (win->type == ACK || win->type == (FIN | ACK)) {
                handle_ack_packet(stream, win, sackData);

                if (tcb->finSequence != 0) {
                    if (tcb->xmitSeq == tcb->xmitFirst && win->ack == tcb->xmitSeq) {
                        SET_STATE(stream, ST_CLOSED);
                    }
                } else {
                    if (win->ack == tcb->xmitSeq) {
                        // We have not sent an fin but now all data has been acked, send a fin
                        nabtoSetFutureStamp(&stream->u.tcb.timeoutStamp, 0);
                    }
                }

                if (win->type == (FIN | ACK)) {
                    NABTO_LOG_TRACE(("Retransmission of FIN|ACK"));
                }
            } else {
                nabto_stream_tcb_event_error(stream, win);
            }

            break;

            /**
             * If the ACK is on data stay in the state.
             * If the ACK is on our fin then move to FIN WAIT 2
             * If there's a FIN, and the ACK is on data then move to state
             * CLOSING
             * If there's a FIn and the ACK is on the FIN we have sent then
             * move to TIME_WAIT
             */
        case ST_FIN_WAIT_1:
            NABTO_LOG_TRACE(("Got packet in ST_FIN_WAIT_1 ack: %i, seq: %i, type: %i", win->ack, win->seq, win->type));
            if (win->type == ACK || win->type == (FIN | ACK)) {
                handle_data(stream, win, start, dlen, sackData); /* sequence numbers are checked by caller */

                if (win->type == ACK) {
                    if (nabto_stream_tcb_is_ack_on_fin(tcb, win)) {
                        SET_STATE(stream, ST_FIN_WAIT_2);
                    } else if (tcb->xmitSeq == tcb->xmitFirst && tcb->finSequence == 0) {
                        // This was ack on the last data send a fin
                        // which was not sent when we did the
                        // transition to ST_FIN_WAIT_1 since there was
                        // unacked data
                        nabtoSetFutureStamp(&stream->u.tcb.timeoutStamp, 0);
                    }
                } else if(win->type == (FIN | ACK)) {
                    // If we have sent a fin and the acknowledge is on this fin
                    // then we should goto state TIME_WAIT.
                   // Else if our FIN has not been acked we should go to the
                    // state CLOSING.
                    if (nabto_stream_tcb_is_ack_on_fin(tcb, win) && nabto_stream_tcb_handle_fin(tcb, win)) {
                        SET_STATE(stream, ST_TIME_WAIT);
                    } else if (nabto_stream_tcb_handle_fin(tcb, win)) {
                        SET_STATE(stream, ST_CLOSING);
                    } else {
                        nabto_stream_tcb_event_error(stream, win);
                    }
                }
            } else {
                nabto_stream_tcb_event_error(stream, win);
            }
            break;

            // In this case we can receive data or get a FIN, all our
            // outstanding data is acked and our FIN is acked.
        case ST_FIN_WAIT_2:
            if (win->type == ACK || win->type == (FIN | ACK)) {
                handle_data(stream, win, start, dlen, sackData);

                if (win->type == (FIN | ACK)) {
                    if (nabto_stream_tcb_handle_fin(tcb, win)) {
                        SET_STATE(stream, ST_TIME_WAIT);
                    }
                }
            } else {
                nabto_stream_tcb_event_error(stream, win);
            }
            break;

            // We have received a fin from the other end. But still needs an
            // ack or has outstanding nonacked data.
        case ST_CLOSING:
            if (win->type == ACK || win->type == (FIN | ACK) ) {
                handle_data(stream, win, start, dlen, sackData);
                if (nabto_stream_tcb_is_ack_on_fin(tcb, win)) {
                    SET_STATE(stream, ST_TIME_WAIT);
                }
            } else {
                nabto_stream_tcb_event_error(stream, win);
            }
            break;

        case ST_TIME_WAIT:
            if (win->type == ACK || win->type == (FIN | ACK) ) {
                handle_data(stream, win, start, dlen, sackData);
            } else {
                nabto_stream_tcb_event_error(stream, win);
            }
            break;

        case ST_CLOSE_WAIT:
            if (win->type == ACK || win->type == (FIN | ACK)) {
                handle_data(stream, win, start, dlen, sackData);
            } else {
                nabto_stream_tcb_event_error(stream, win);
            }
            break;

        default:
            break;
    }
}

bool nabto_stream_tcb_is_ack_on_fin(struct nabto_stream_tcb* tcb, struct nabto_win_info* win) {
    return (tcb->finSequence != 0 && win->ack == tcb->xmitSeq);
}

bool nabto_stream_tcb_handle_fin(struct nabto_stream_tcb* tcb, struct nabto_win_info* win) {
    tcb->recvFinSeq = win->seq;

    // if all outstandind data has been read increment recvNext such that the fin is acknowledged
    if(tcb->recvNext == tcb->recvFinSeq) {
        tcb->recvNext++;
        tcb->recvTop++;
    }

    return true;
}


/**
 * According to rfc2018:
 * The SACK option SHOULD be filled out by repeating the most
 * recently reported SACK blocks (based on first SACK blocks in
 * previous SACK options) that are not subsets of a SACK block
 * already included in the SACK option being constructed.  This
 * assures that in normal operation, any segment remaining part of a
 * non-contiguous block of data held by the data receiver is reported
 * in at least three successive SACK options, even for large-window
 * TCP implementations [RFC1323]).  After the first SACK block, the
 * following SACK blocks in the SACK option may be listed in
 * arbitrary order.
 *
 * @return true iff there are any sack pairs available.
 */
bool unabto_stream_create_sack_pairs(struct nabto_stream_s* stream, struct nabto_stream_sack_data* sackData)
{
    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    uint32_t end = 0;
    uint32_t i;
    

    memset(sackData, 0, sizeof(struct nabto_stream_sack_data));

    if (tcb->streamState < ST_ESTABLISHED) {
        return false;
    }

    // recvtop is one above max cumulative received sequence number.
    // recvMax is the maximum filled recv slot
    // if recvMax + 1 == recvTop there are no sack holes.
    for (i = tcb->recvMax; i >= tcb->recvTop; i--) {
        uint16_t ix = i % tcb->cfg.recvWinSize;
        r_buffer* rBuf = &tcb->recv[ix];

        bool slotused = (rBuf->size > 0 && rBuf->seq == i);

        if (end == 0 && slotused) {
            end = i+1;
        }

        if (end != 0 && !slotused) {
            unabto_stream_insert_sack_pair(i+1, end, sackData);
            end = 0;
        }

    }
    return (sackData->nPairs > 0);
}

/**
 * @param force if true insert the sack data even if there is no more
 *        room left for sack data in that case overwrite the top sack data
 * @return true if there was room for the sack
 *         pair, false otherwise.
 */
static bool unabto_stream_insert_sack_pair(uint32_t start, uint32_t end, struct nabto_stream_sack_data* sackData)
{
    NABTO_LOG_TRACE(("inserting sack pair (%" PRIu32 ",%" PRIu32 ")", start, end));

    if (sackData->nPairs < NP_PAYLOAD_SACK_MAX_PAIRS) {
        sackData->pairs[sackData->nPairs].start = start;
        sackData->pairs[sackData->nPairs].end   = end;
        sackData->nPairs++;
        return true;
    }
    return false;
}

/******************************************************************************/
/******************************************************************************/

bool nabto_stream_tcb_is_open(const struct nabto_stream_tcb* tcb)
{
    return (tcb->streamState >= ST_ESTABLISHED) && (tcb->streamState < ST_CLOSED);
}

/**
 * A stream is readable as long the other end has not closed the stream
 * and no more readable data is available.
 */
bool nabto_stream_tcb_is_readable(struct nabto_stream_s* stream) {
    const uint8_t* buf;
    unabto_stream_hint hint;
    nabto_stream_tcb_read(stream, &buf, &hint);
    return (hint == UNABTO_STREAM_HINT_OK);
}

/**
 * A stream is writeable ad long as the user has not closed the stream
 */
bool nabto_stream_tcb_is_writeable(const struct nabto_stream_tcb* tcb) {
    nabto_stream_tcb_state state = tcb->streamState;
    switch(state) {
        case ST_ESTABLISHED:
        case ST_CLOSE_WAIT:
            return true;
        default:
            return false;
    }
}

bool nabto_stream_tcb_close(struct nabto_stream_s* stream)
{
    nabto_stream_tcb_state state = stream->u.tcb.streamState;

    if (state == ST_CLOSE_WAIT || state == ST_ESTABLISHED) {
        if (state == ST_CLOSE_WAIT) {
            SET_STATE(stream, ST_LAST_ACK);
        }

        if (state == ST_ESTABLISHED) {
            SET_STATE(stream, ST_FIN_WAIT_1);
        }
    } else {
        if (stream->u.tcb.streamState < ST_ESTABLISHED) {
            // break opening protocol
            SET_STATE(stream, ST_CLOSED);
        }
    }

    return nabto_stream_tcb_is_closed(stream);
}

bool nabto_stream_tcb_force_close(struct nabto_stream_s* stream) {
    nabto_stream_tcb_state state = stream->u.tcb.streamState;
    if (state == ST_CLOSED || state == ST_CLOSED_ABORTED) {
        return true;
    }

    send_rst(stream);

    SET_STATE(stream, ST_CLOSED_ABORTED);
    return true;
}

void nabto_stream_tcb_on_connection_closed(struct nabto_stream_s * stream) {
    if (stream->u.tcb.streamState < ST_ESTABLISHED) {
        unabto_stream_release(stream);
    } else {
        nabto_stream_tcb_state state = stream->u.tcb.streamState;
        if (state == ST_CLOSED || state == ST_CLOSED_ABORTED) {
            return;
        }
        send_rst(stream);
        SET_STATE(stream, ST_CLOSED_ABORTED);
    }
}

void nabto_stream_tcb_on_connection_released(struct nabto_stream_s * stream) {
    if (stream->u.tcb.streamState < ST_ESTABLISHED) {
        unabto_stream_release(stream);
    } else {
        nabto_stream_tcb_state state = stream->u.tcb.streamState;
        if (state == ST_CLOSED || state == ST_CLOSED_ABORTED) {
            return;
        }
        SET_STATE(stream, ST_CLOSED_ABORTED);
    }
}

void nabto_stream_tcb_release(struct nabto_stream_s * stream) {
    size_t i;
    struct nabto_stream_tcb * tcb;
    if (!nabto_stream_tcb_is_closed(stream)) {
        NABTO_LOG_TRACE(("Releasing stream in state %d - %" PRItext, stream->u.tcb.streamState, nabto_stream_tcb_state_name(&stream->u.tcb)));
    }
    // run through the stream send window and free all unfreed send
    // segments.  there can be unfreed segments if the stream is
    // closed without all the data being acked.
    tcb = &stream->u.tcb;
    for (i = 0; i < tcb->cfg.xmitWinSize; i++) {
        if (tcb->xmit[i].xstate != B_IDLE) {
            unabto_stream_free_send_segment(tcb->xmit[i].buf);
            tcb->xmit[i].buf = NULL;
        }
    }
}

bool nabto_stream_tcb_is_closed(struct nabto_stream_s * stream) {
    return stream->u.tcb.streamState == ST_CLOSED || stream->u.tcb.streamState == ST_CLOSED_ABORTED;
}


#if NABTO_ENABLE_NEXT_EVENT
void nabto_stream_tcb_update_next_event(struct nabto_stream_s * stream, nabto_stamp_t* current_min_stamp) {
    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    if (tcb->streamState == ST_CLOSED || tcb->streamState == ST_CLOSED_ABORTED) {
        return;
    }

    if (tcb->xmitLastSent > tcb->xmitFirst && tcb->finSequence == 0 && tcb->streamState >= ST_ESTABLISHED) {
        nabto_update_min_stamp(current_min_stamp, &tcb->dataTimeoutStamp);
    }

    nabto_update_min_stamp(current_min_stamp, &tcb->timeoutStamp);
}
#endif

bool nabto_stream_read_window(const uint8_t* ptr, uint16_t len, struct nabto_win_info* info)
{
    if (len < NP_PAYLOAD_WINDOW_BYTELENGTH - NP_PAYLOAD_HDR_BYTELENGTH) return 0;
    READ_FORWARD_U8(info->type,    ptr);
    READ_FORWARD_U8(info->version, ptr);
    READ_FORWARD_U16(info->idCP,   ptr);
    READ_FORWARD_U16(info->idSP,   ptr);
    READ_FORWARD_U32(info->seq,    ptr);
    READ_FORWARD_U32(info->ack,    ptr);
    if (info->type == NP_PAYLOAD_WINDOW_FLAG_ACK) {
        READ_FORWARD_U16(info->advertisedWindow, ptr);
    }
    if (info->type == NP_PAYLOAD_WINDOW_FLAG_NON) {
        NABTO_LOG_ERROR(("failed to read window"));
        return false;
    }
    if (info->type & NP_PAYLOAD_WINDOW_FLAG_SYN) {
        uint16_t options;
        if (len < NP_PAYLOAD_WINDOW_SYN_BYTELENGTH - NP_PAYLOAD_HDR_BYTELENGTH) return 0;

        READ_FORWARD_U16(options,            ptr);
        if (options & NP_PAYLOAD_STREAM_FLAG_WSRF) {
            NABTO_LOG_TRACE(("WSRF: Enabled by peer"));
            info->u.syn.cfg.enableWSRF = true;
        } else {
            info->u.syn.cfg.enableWSRF = false;
        }
        if (options & NP_PAYLOAD_STREAM_FLAG_SACK) {
            NABTO_LOG_TRACE(("SACK: Enabled by peer"));
            info->u.syn.cfg.enableSACK = true;
        } else {
            info->u.syn.cfg.enableSACK = false;
        }
        /* we swap xmit and recv when reading because we are "the other end" */
        READ_FORWARD_U16(info->u.syn.cfg.xmitPacketSize, ptr);
        READ_FORWARD_U16(info->u.syn.cfg.xmitWinSize,    ptr);
        READ_FORWARD_U16(info->u.syn.cfg.recvPacketSize, ptr);
        READ_FORWARD_U16(info->u.syn.cfg.recvWinSize,    ptr);
        READ_FORWARD_U16(info->u.syn.cfg.maxRetrans,     ptr);
        READ_FORWARD_U16(info->u.syn.cfg.timeoutMsec,    ptr);
    }
    return true;
}

bool nabto_stream_encode_window(const struct nabto_win_info* win, uint8_t* ptr, uint16_t* length)
{
    uint8_t* start = ptr;
    WRITE_FORWARD_U8 (ptr, win->type);
    WRITE_FORWARD_U8 (ptr, win->version);
    WRITE_FORWARD_U16(ptr, win->idCP);
    WRITE_FORWARD_U16(ptr, win->idSP);
    WRITE_FORWARD_U32(ptr, win->seq);
    WRITE_FORWARD_U32(ptr, win->ack);
    if (win->type == NP_PAYLOAD_WINDOW_FLAG_ACK) {
        WRITE_FORWARD_U16(ptr, win->advertisedWindow);
    }

    if (win->type & NP_PAYLOAD_WINDOW_FLAG_SYN) {
        uint16_t options = 0;

        if (win->u.syn.cfg.enableWSRF) {
            options |= NP_PAYLOAD_STREAM_FLAG_WSRF;
        }
        if (win->u.syn.cfg.enableSACK) {
            options |= NP_PAYLOAD_STREAM_FLAG_SACK;
        }

        WRITE_FORWARD_U16(ptr, options);
        WRITE_FORWARD_U16(ptr, win->u.syn.cfg.recvPacketSize);
        WRITE_FORWARD_U16(ptr, win->u.syn.cfg.recvWinSize);
        WRITE_FORWARD_U16(ptr, win->u.syn.cfg.xmitPacketSize);
        WRITE_FORWARD_U16(ptr, win->u.syn.cfg.xmitWinSize);
        WRITE_FORWARD_U16(ptr, win->u.syn.cfg.maxRetrans);
        WRITE_FORWARD_U16(ptr, win->u.syn.cfg.timeoutMsec);
    }
    *length = (uint16_t)(ptr - start);
    return true;
}

uint16_t nabto_stream_window_payload_length(struct nabto_win_info* win)
{
    if (win->type == ACK) {
        return NP_PAYLOAD_WINDOW_ACK_BYTELENGTH;
    }

    if (win->type & SYN) {
        return NP_PAYLOAD_WINDOW_SYN_BYTELENGTH;
    }

    return NP_PAYLOAD_WINDOW_BYTELENGTH;
}

void nabto_stream_make_rst_response_window(const struct nabto_win_info* win, struct nabto_win_info* rstWin)
{
    rstWin->type = NP_PAYLOAD_WINDOW_FLAG_RST;
    rstWin->version = win->version;
    rstWin->idCP = win->idCP;
    rstWin->idSP = win->idSP;
    rstWin->seq = win->ack;
    rstWin->ack = win->seq + 1;
}


bool use_slow_start(struct nabto_stream_tcb* tcb);
void windowStatus(const char* str, struct nabto_stream_tcb* tcb);


void unabto_stream_secondary_data_structure_init(struct nabto_stream_s* stream)
{
    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    unabto_stream_congestion_control_init(tcb);

}

void unabto_stream_congestion_control_init(struct nabto_stream_tcb* tcb)
{
    tcb->cCtrl.isFirstAck = true;
    tcb->cCtrl.cwnd = CWND_INITIAL_VALUE;
    tcb->cCtrl.srtt = tcb->cfg.timeoutMsec;
    tcb->cCtrl.rto =  tcb->cfg.timeoutMsec;
    tcb->cCtrl.ssThreshold = tcb->cfg.xmitWinSize;
    unabto_stream_stats_observe(&tcb->ccStats.ssThreshold, tcb->cCtrl.ssThreshold);
}

void unabto_stream_congestion_control_adjust_ssthresh_after_triple_ack(struct nabto_stream_tcb* tcb) {
    if (!tcb->cCtrl.lostSegment) {
        tcb->cCtrl.ssThreshold = MAX(tcb->cCtrl.flightSize/2.0, SLOWSTART_MIN_VALUE);
        unabto_stream_stats_observe(&tcb->ccStats.ssThreshold, tcb->cCtrl.ssThreshold);
        tcb->cCtrl.lostSegment = true;
    }
}

/**
 * Called after a streaming data packet has been resent.
 */
void unabto_stream_congestion_control_timeout(struct nabto_stream_s * stream) {
    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    NABTO_LOG_TRACE(("unabto_stream_congestion_control_timeout"));
    // In the case of dual resending we cannot simply use cwnd as
    // a measure for the flight size since we could have reset it
    // in a previous resending.

    // After a timeout start with a fresh slow start
    tcb->cCtrl.cwnd = CWND_INITIAL_VALUE;
    tcb->cCtrl.ssThreshold = tcb->cfg.xmitWinSize;

    // A problem with karns algorithm is that a huge increase
    // of the delay can not be reflected in the
    // calculated srtt since all packets will timeout.
    // And hence no cleanly acked packets can be used for
    // the calculation of the srtt.

    tcb->cCtrl.rto += tcb->cCtrl.rto/2; // aka rto = rto*1.5 => exp backoff

    if (tcb->cCtrl.rto > stream->staticConfig.maxRetransmissionTime) {
        tcb->cCtrl.rto = stream->staticConfig.maxRetransmissionTime;
    }

    stream->stats.timeouts++;
    {
        int ix;
        x_buffer* xbuf;
        ix = tcb->xmitFirst % tcb->cfg.xmitWinSize;
        xbuf = &tcb->xmit[ix];
        NABTO_LOG_TRACE(("timeout, nretrans: %" PRIu16 ", seq: %" PRIu32, xbuf->nRetrans, xbuf->seq));
        if (xbuf->nRetrans > tcb->cfg.maxRetrans) {
            // timeout the stream
            NABTO_LOG_ERROR(("The stream has closed due to too many retransmissions of segment %" PRIu32, xbuf->seq));
            send_rst(stream);
            SET_STATE(stream, ST_CLOSED_ABORTED);
        }
    }

    windowStatus("Stream data timeout:", tcb);
}

void unabto_stream_congestion_control_handle_ack(struct nabto_stream_tcb* tcb, uint16_t ix, bool ackOnStartOfWindow) {
    // unabto_stream_window.c will set the buffer to IDLE when this function
    // returns.
    if (tcb->xmit[ix].xstate == B_SENT) {

        if (tcb->cCtrl.lostSegment && ackOnStartOfWindow) {
            tcb->cCtrl.lostSegment = false;
        }

        if (tcb->cCtrl.lostSegment) {
            tcb->cCtrl.cwnd += 1;
        } else if (use_slow_start(tcb)) {
            NABTO_LOG_TRACE(("slow starting! %f", tcb->cCtrl.cwnd));
            tcb->cCtrl.cwnd += 2;
        } else {
            // congestion avoidance
            tcb->cCtrl.cwnd += 1 + (1.0/tcb->cCtrl.flightSize);
        }
        {
            // cwnd should not grow above the maximum possible data on
            // the network. This will also limit cwnd in cases where
            // the client is slow to accept new data and hence end in
            // some slow start situation when the window opens again.
            uint32_t maxData = tcb->maxAdvertisedWindow - tcb->xmitFirst;

            if (tcb->cCtrl.cwnd > maxData) {
                tcb->cCtrl.cwnd = maxData;
            }
        }

        unabto_stream_stats_observe(&tcb->ccStats.cwnd, tcb->cCtrl.cwnd);
    }

    NABTO_LOG_TRACE(("adjusting cwnd: %f, ssThreshold: %f, flightSize %i", tcb->cCtrl.cwnd, tcb->cCtrl.ssThreshold, tcb->cCtrl.flightSize));
}

/**
 * This is called after tripple acks or after a timeout.
 */
void unabto_stream_mark_segment_for_retransmission(struct nabto_stream_tcb* tcb, x_buffer* xbuf)
{
    if (xbuf->xstate != B_SENT) {
        return;
    }
    if (xbuf->shouldRetransmit) {
        return;
    }
    
    xbuf->shouldRetransmit = true;
    tcb->retransmitSegmentCount++;
}


/**
 * Called before a data packet is sent to test if it's allowed to be sent.
 */
bool unabto_stream_congestion_control_can_send(struct nabto_stream_tcb* tcb)
{
    // is_in_cwnd(tcb);
    return (tcb->cCtrl.cwnd > 0);
}

bool use_slow_start(struct nabto_stream_tcb* tcb) {
    return tcb->cCtrl.flightSize < tcb->cCtrl.ssThreshold;
}

/**
 * Count not sent data
 */
uint32_t unabto_stream_not_sent_segments(struct nabto_stream_tcb* tcb) {
    // xmitSeq is the next sequence number for data to send.
    // xmitLastSent is the sequence number for the lastSegment sent +
    // 1. This means the following invariant holds xmitSeq >= xmitLastSent
    return (tcb->xmitSeq - tcb->xmitLastSent);
}

bool congestion_control_accept_more_data(struct nabto_stream_tcb* tcb) {
    bool status = (unabto_stream_not_sent_segments(tcb) <= tcb->cCtrl.cwnd);
    if (!status) {
        NABTO_LOG_TRACE(("Stream  does not accept more data notSent: %" PRIu32 ", cwnd %f", unabto_stream_not_sent_segments(tcb), tcb->cCtrl.cwnd));
    }
    return status;
}

void windowStatus(const char* str, struct nabto_stream_tcb* tcb) {
    NABTO_LOG_TRACE(("%s, ssthres: %f, cwnd: %f, srtt: %f, rttVar: %f", str, tcb->cCtrl.ssThreshold, tcb->cCtrl.cwnd, tcb->cCtrl.srtt, tcb->cCtrl.rttVar));
}

void unabto_stream_update_congestion_control_receive_stats(struct nabto_stream_s * stream, uint16_t ix) {
    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    // Update the rtt if the packet has been sent and but not resent
    if (tcb->xmit[ix].xstate == B_SENT && tcb->xmit[ix].nRetrans == 0) {
        nabto_stamp_t sentTime = tcb->xmit[ix].sentStamp;
        nabto_stamp_t now = nabtoGetStamp();
        nabto_stamp_diff_t diff = nabtoStampDiff(&now, &sentTime); // now-sentTime
        double time = nabtoStampDiff2ms(diff);

        if (tcb->cCtrl.isFirstAck) {
            /**
             * RFC 2988:
             * SRTT <- R
             * RTTVAR <- R/2
             * RTO <- SRTT + max(G, k*RTTVAR)
             * k = 4
             * let G = 0;
             */
            tcb->cCtrl.srtt = time;
            tcb->cCtrl.rttVar = time/2;
            tcb->cCtrl.isFirstAck = false;
        } else {
            /**
             * RFC 2988
             * RTTVAR <- (1-alpha) * RTTVAR + beta * | SRTT - R' |
             * SRTT <- (1-alpha) * SRTT + alpha * R'
             * alpha = 1/8, betal = 1/4
             */
            double alpha = 1.0/8;
            double beta = 1.0/4;
            tcb->cCtrl.rttVar = (1.0-beta) * tcb->cCtrl.rttVar + beta * fabs(tcb->cCtrl.srtt - time);
            tcb->cCtrl.srtt = (1.0-alpha) * tcb->cCtrl.srtt + alpha * time;
        }
        unabto_stream_stats_observe(&tcb->ccStats.rtt, time);
        tcb->cCtrl.rto = tcb->cCtrl.srtt + 4.0*tcb->cCtrl.rttVar;

        NABTO_LOG_TRACE(("packet time %f, tcb->srtt %f, tcb->rttVar %f, tcb->rto %" PRIu16, time, tcb->cCtrl.srtt, tcb->cCtrl.rttVar, tcb->cCtrl.rto));


        /**
         * This will circumvent retransmissions in the case where
         * the client only sends acks for every second packet
         */
        tcb->cCtrl.rto += 500;

        if (tcb->cCtrl.rto > stream->staticConfig.maxRetransmissionTime) {
            tcb->cCtrl.rto = stream->staticConfig.maxRetransmissionTime;
        }

        /**
         * RFC 2988 (2.4)
         * if rto < 1s, set rto = 1s
         *
         * This part of the RFC is omitted such that we are more robust
         * for bad networks.
         */
    }
}

void unabto_stream_dump_state(struct nabto_stream_s* stream) {
#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_TRACE)
    struct nabto_stream_tcb* tcb = &stream->u.tcb;

    NABTO_LOG_TRACE(("stream_s"));
    NABTO_LOG_TRACE((" streamTag %i", stream->streamTag));
    NABTO_LOG_TRACE((" state %i", stream->state));
    NABTO_LOG_TRACE((" idCP %i, idSP %i", stream->idCP, stream->idSP));

    NABTO_LOG_TRACE((" tcb "));
    NABTO_LOG_TRACE(("  streamState %i", tcb->streamState));
    NABTO_LOG_TRACE(("  retransCount %i", tcb->retransCount));
    NABTO_LOG_TRACE(("  xmitSeq %i", tcb->xmitSeq));
    NABTO_LOG_TRACE(("  finSent %i", tcb->finSequence));
    NABTO_LOG_TRACE(("  xmitLastSent %i", tcb->xmitLastSent));
    NABTO_LOG_TRACE(("  xmitFirst %i", tcb->xmitFirst));
    NABTO_LOG_TRACE(("  ackWSRFcount %i", tcb->ackWSRFcount));
    NABTO_LOG_TRACE(("  recvNext %i", tcb->recvNext));
    NABTO_LOG_TRACE(("  ackSent %i", tcb->ackSent));
    NABTO_LOG_TRACE((" congestion control"));
    NABTO_LOG_TRACE(("  srtt %f", tcb->cCtrl.srtt));
    NABTO_LOG_TRACE(("  rttVar %f", tcb->cCtrl.rttVar));
    NABTO_LOG_TRACE(("  rto %" PRIu16, tcb->cCtrl.rto));
    NABTO_LOG_TRACE(("  cwnd %f", tcb->cCtrl.cwnd));
    NABTO_LOG_TRACE(("  ssThreshold %f", tcb->cCtrl.ssThreshold));
    NABTO_LOG_TRACE(("  flightSize %i", tcb->cCtrl.flightSize));
#endif
}

/**
 * Initialise the stream configuration from received request.
 * @param stream  the stream
 * @param info    the request
 */
static void nabto_init_stream_state(struct nabto_stream_s* stream, const struct nabto_win_info* info)
{
    stream->state = STREAM_IN_USE;
    stream->idCP = info->idCP;
    stream->idSP = nabto_stream_next_sp_id();
    nabto_init_stream_tcb_state(&stream->u.tcb, info, stream);
}

static uint16_t nabto_stream_next_sp_id(void) {
   uint16_t idSP;
   do { idSP = ++idSP__; } while (idSP == 0);
   return idSP;
}

static uint16_t nabto_stream_next_cp_id(void) {
   uint16_t idCP;
   do { idCP = ++idCP__; } while (idCP == 0);
   return idCP;
}

void unabto_stat_init(struct unabto_stats* stat)
{
    stat->min = 0;
    stat->max = 0;
    stat->count = 0;
    stat->avg = 0;
}

void unabto_stream_stats_observe(struct unabto_stats* stat, double value)
{
    if (stat->min == 0 && stat->max == 0) {
        stat->min = stat->max = value;
    }

    if (value < stat->min) {
        stat->min = value;
    }
    if (value > stat->max) {
        stat->max = value;
    }
    stat->count += 1;
    stat->avg += (value - stat->avg) / stat->count;
}

uint32_t unabto_stream_get_duration(struct nabto_stream_s* stream)
{
    nabto_stamp_t now = nabtoGetStamp();
    nabto_stamp_diff_t duration = nabtoStampDiff(&now, &stream->stats.streamStart);
    return nabtoStampDiff2ms(duration);
}

#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_DEBUG)
const char* unabto_stream_type_to_string(uint8_t winType)
{
    text msg;
    switch (winType) {
        case NP_PAYLOAD_WINDOW_FLAG_SYN                             : msg = "SYN";     break;
        case NP_PAYLOAD_WINDOW_FLAG_SYN | NP_PAYLOAD_WINDOW_FLAG_ACK: msg = "SYN|ACK"; break;
        case NP_PAYLOAD_WINDOW_FLAG_FIN | NP_PAYLOAD_WINDOW_FLAG_ACK: msg = "FIN|ACK"; break;
        case NP_PAYLOAD_WINDOW_FLAG_RST                             : msg = "RST";     break;
        case NP_PAYLOAD_WINDOW_FLAG_ACK                             : msg = "ACK";    break;
        default: msg = "?"; NABTO_LOG_TRACE(("Type?: %" PRIu8, winType)); break;
    }
    return msg;
}
#endif

#endif /* NABTO_ENABLE_STREAM && NABTO_ENABLE_MICRO_STREAM */
