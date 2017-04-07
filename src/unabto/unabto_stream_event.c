/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_STREAM

/**
 * @file
 * Nabto uServer stream packet event - Implementation.
 */
#include "unabto_env_base.h"

#if NABTO_ENABLE_STREAM && NABTO_ENABLE_MICRO_STREAM

#include "unabto_stream.h"
#include "unabto_stream_event.h"
#include "unabto_protocol_defines.h"
#include "unabto_packet.h"
#include "unabto_logging.h"
#include "unabto_util.h"
#include "unabto_main_contexts.h"
#include "unabto_memory.h"
#include "unabto_external_environment.h"
#include "unabto_stream_environment.h"
#if NABTO_ENABLE_NEXT_EVENT
#include "unabto_next_event.h"
#endif

#include <string.h>



static void handle_stream_packet(nabto_connect* con, nabto_packet_header* hdr,
                                 uint8_t* start, uint16_t dlen,
                                 uint8_t* payloadsStart, uint8_t* payloadsEnd,
                                 void* userData);

static struct nabto_stream_s* find_stream(uint16_t tag, nabto_connect* con);

static struct nabto_stream_s* find_free_stream(uint16_t tag, nabto_connect* con);

static bool nabto_stream_validate_win(struct nabto_win_info* info, struct nabto_stream_s* stream);

/******************************************************************************/

void handle_stream_packet(nabto_connect* con, nabto_packet_header* hdr,
                          uint8_t* start, uint16_t dlen,
                          uint8_t* payloadsStart, uint8_t* payloadsEnd,
                          void* userData) {
    
    // read the window and sack payloads
    struct unabto_payload_packet window;

    uint8_t* sackStart = 0;
    uint16_t sackLength = 0;
         
    NABTO_NOT_USED(userData);

    if(!unabto_find_payload(payloadsStart, payloadsEnd, NP_PAYLOAD_TYPE_WINDOW, &window)) {
        NABTO_LOG_ERROR(("Stream %i, Packet has no WINDOW payload!", hdr->tag));
        return;
    }

    {
        struct unabto_payload_packet sack;
        if (unabto_find_payload(payloadsStart, payloadsEnd, NP_PAYLOAD_TYPE_SACK, &sack)) {
            sackStart = (uint8_t*)sack.dataBegin;
            sackLength = sack.dataLength;
        }
    }

    NABTO_LOG_DEBUG(("(.%i.) STREAM EVENT, dlen: %i", hdr->nsi_sp, dlen));
    nabtoSetFutureStamp(&con->stamp, con->timeOut);
    nabto_stream_event(con, hdr, (uint8_t*)(window.dataBegin - SIZE_PAYLOAD_HEADER), start, dlen, sackStart, sackLength);
}

void nabto_stream_event(nabto_connect*       con,
                        nabto_packet_header* hdr,
                        uint8_t*             info, //WINDOW payload with payload header
                        uint8_t*             start,
                        int                  dlen,
                        uint8_t*             sackStart,
                        uint16_t             sackLength)
{
    struct nabto_win_info  win;
    struct nabto_stream_s* stream;
    struct nabto_stream_sack_data sackData;
    uint16_t len;

    // We must have a WINDOW payload to continue.
    if (!info) {
        NABTO_LOG_ERROR(("Stream %i, Packet has no WINDOW payload!", hdr->tag));
        return;
    }
    
    READ_U16(len, info + 2);
    if (!nabto_stream_read_window(info + SIZE_PAYLOAD_HEADER, len - SIZE_PAYLOAD_HEADER, &win)) {
        NABTO_LOG_DEBUG(("ReadWin failure"));
        return;
    }

    {
        text msg;
        switch (win.type) {
        case NP_PAYLOAD_WINDOW_FLAG_SYN                             : msg = "SYN";     break;
        case NP_PAYLOAD_WINDOW_FLAG_SYN | NP_PAYLOAD_WINDOW_FLAG_ACK: msg = "SYN|ACK"; break;
        case NP_PAYLOAD_WINDOW_FLAG_FIN | NP_PAYLOAD_WINDOW_FLAG_ACK: msg = "FIN|ACK"; break;
        case NP_PAYLOAD_WINDOW_FLAG_RST                             : msg = "RST";     break;
        case NP_PAYLOAD_WINDOW_FLAG_ACK                             : msg = "DATA";    break;
        default       : msg = "?"; NABTO_LOG_TRACE(("Type?: %" PRIu8, win.type)); break;

        }
        NABTO_NOT_USED(msg);
        NABTO_LOG_DEBUG(("%" PRIu16 " --> [%" PRIu32 ",%" PRIu32 "] %" PRItext ", %d bytes", hdr->tag, win.seq, win.ack, msg, dlen));
    }

    stream = find_stream(hdr->tag, con);
    if (stream == NULL) {
        if (win.type == NP_PAYLOAD_WINDOW_FLAG_SYN) {
            stream = find_free_stream(hdr->tag, con);
            if (stream == NULL) {
                NABTO_LOG_DEBUG(("Stream with tag %i not accepted", hdr->tag));
            }
        } else {
            NABTO_LOG_DEBUG(("Received non syn packet for stream which is not available tag %i", hdr->tag));
        }
    }

    if (stream == NULL) {
        if (! (win.type & NP_PAYLOAD_WINDOW_FLAG_RST)) {
            build_and_send_rst_packet(con, hdr->tag, &win);
        }
        return;
    }
    
    if (!nabto_stream_validate_win(&win, stream)) {
        NABTO_LOG_ERROR(("Cannot validate received stream window."));
        return;
    }

    NABTO_LOG_TRACE(("(.%i.) Stream with tag %i accepted, slot=%i", con->spnsi, hdr->tag, unabto_stream_index(stream)));

    stream->stats.receivedPackets++;
 
    memset(&sackData, 0, sizeof(sackData));
    {
        uint8_t* ptr = sackStart;
        while(sackLength >= 8 && sackData.nPairs < NP_PAYLOAD_SACK_MAX_PAIRS) {
            uint32_t sackSeqStart; // start of sack 
            uint32_t sackSeqEnd; // end of sack one larger than actual acked window.
            READ_FORWARD_U32(sackSeqStart, ptr);
            READ_FORWARD_U32(sackSeqEnd, ptr);
            sackLength -= 8;
            
            sackData.pairs[sackData.nPairs].start = sackSeqStart;
            sackData.pairs[sackData.nPairs].end = sackSeqEnd;
            sackData.nPairs++;
        }
    }

    nabto_stream_tcb_event(stream, &win, start, dlen, &sackData);
}

bool build_and_send_rst_packet(nabto_connect* con, uint16_t tag, struct nabto_win_info* win)
{
    struct nabto_win_info rst;
    uint16_t winLength;
    uint16_t encodeLength;
    uint8_t*       ptr;
    uint8_t*       buf   = nabtoCommunicationBuffer;
    uint8_t*       end   = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;
    memset(&rst, 0, sizeof( struct nabto_win_info));
    
    nabto_stream_make_rst_response_window(win, &rst);
    winLength = nabto_stream_window_payload_length(&rst);

    ptr = insert_data_header(buf, con->spnsi, con->nsico, tag);
    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_WINDOW, 0, winLength);

    if (nabto_stream_encode_window(&rst, ptr, &encodeLength)) {
        ptr += encodeLength;
    } else {
        return false;
    }

    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_CRYPTO, 0, 0);
    
    return send_and_encrypt_packet_con(con, 0, 0, ptr);
}

void unabto_time_event_stream(void)
{
    int i;
    for (i = 0; i < NABTO_MEMORY_STREAM_MAX_STREAMS; ++i) {
        if (stream__[i].state != STREAM_IDLE) {
            struct nabto_stream_s* stream = &stream__[i];
            nabto_stream_tcb_check_xmit(&stream__[i], true, false);
            if (stream->statisticsEvents.streamEnded) {
                unabto_stream_send_stats(stream, NP_PAYLOAD_STATS_TYPE_UNABTO_STREAM_ENDED);
                stream->statisticsEvents.streamEnded = false;
            }
            if (stream->applicationEvents.dataReady) {
                unabto_stream_event(stream, UNABTO_STREAM_EVENT_TYPE_DATA_READY);
                stream->applicationEvents.dataReady = false;
            }

            if (stream->applicationEvents.dataWritten) {
                unabto_stream_event(stream, UNABTO_STREAM_EVENT_TYPE_DATA_WRITTEN);
                stream->applicationEvents.dataWritten = false;
            }

            if (stream->applicationEvents.readClosed) {
                unabto_stream_event(stream, UNABTO_STREAM_EVENT_TYPE_READ_CLOSED);
                stream->applicationEvents.readClosed = false;
            }

            if (stream->applicationEvents.writeClosed) {
                unabto_stream_event(stream, UNABTO_STREAM_EVENT_TYPE_WRITE_CLOSED);
                stream->applicationEvents.writeClosed = false;
            }

            if (stream->applicationEvents.closed) {
                unabto_stream_event(stream, UNABTO_STREAM_EVENT_TYPE_CLOSED);
                stream->applicationEvents.closed = false;
            }
        }
    }
}

/******************************************************************************/
/******************************************************************************/

void unabto_stream_init(void)
{
    memset(stream__, 0, sizeof(struct nabto_stream_s) * NABTO_MEMORY_STREAM_MAX_STREAMS);
    
    NABTO_LOG_INFO(("sizeof(stream__)=%" PRIsize, sizeof(struct nabto_stream_s) * NABTO_MEMORY_STREAM_MAX_STREAMS));
    
    unabto_packet_set_handler(NP_PACKET_HDR_TAG_STREAM_MIN, NP_PACKET_HDR_TAG_STREAM_MAX,
                              handle_stream_packet, 0);
}

void stream_initial_config(struct nabto_stream_s* stream)
{
    nabto_stream_tcb_config* cfg = &stream->u.tcb.initialConfig;
    cfg->recvPacketSize = NABTO_MEMORY_STREAM_RECEIVE_SEGMENT_SIZE;
    cfg->recvWinSize = NABTO_MEMORY_STREAM_RECEIVE_WINDOW_SIZE;
    cfg->xmitPacketSize = NABTO_MEMORY_STREAM_SEND_SEGMENT_SIZE;
    cfg->xmitWinSize = NABTO_MEMORY_STREAM_SEND_WINDOW_SIZE;
    cfg->maxRetrans = NABTO_STREAM_MAX_RETRANS;
    cfg->timeoutMsec = NABTO_STREAM_TIMEOUT;
    cfg->enableWSRF = true;
    cfg->enableSACK = true;
}

void stream_init_static_config(struct nabto_stream_s* stream)
{
    stream->staticConfig.defaultStreamTimeout = NABTO_STREAM_TIMEOUT;
    stream->staticConfig.minRetrans = NABTO_STREAM_MIN_RETRANS;
    stream->staticConfig.maxRetransmissionTime = NABTO_STREAM_MAX_RETRANSMISSION_TIME;
}

/**
 * Initialise the stream data. @param stream the stream data
 */
void stream_reset(struct nabto_stream_s* stream)
{
    unabto_stream_init_data_structure(stream);
    stream_initial_config(stream);
    stream_init_static_config(stream);
    unabto_stream_init_buffers(stream);
}

/**
 * Find a stream
 * @param tag  the stream tag
 * @param con  the connection
 * @return     the stream (0 if not found)
 */
struct nabto_stream_s* find_stream(uint16_t tag, nabto_connect* con)
{
    int i;
    for (i = 0; i < NABTO_MEMORY_STREAM_MAX_STREAMS; ++i) {
        if (stream__[i].state != STREAM_IDLE) {
            if (stream__[i].streamTag == tag && 
                stream__[i].connection == con) {
                return stream__ + i; // return stream in use
            }
        }
    }
    return NULL;
}

struct nabto_stream_s* find_free_stream(uint16_t tag, nabto_connect* con) 
{
    int i;
    for (i = 0; i < NABTO_MEMORY_STREAM_MAX_STREAMS; ++i) {
        if (stream__[i].state == STREAM_IDLE) {
            struct nabto_stream_s* stream = &stream__[i];
            stream_reset(stream);
            stream->streamTag = tag;
            stream->connection = con;
            return stream;
        }
    }
    return NULL;
}

void nabto_stream_connection_closed(nabto_connect* con)
{
    struct nabto_stream_s* stream;

    for (stream = stream__; stream < stream__ + NABTO_MEMORY_STREAM_MAX_STREAMS; ++stream) {
        if (stream->connection == con && stream->state != STREAM_IDLE) {
            NABTO_LOG_TRACE(("Releasing stream, slot=%i, (connection closed) in state %" PRItext, unabto_stream_index(stream), nabto_stream_state_name(stream)));

            nabto_stream_tcb_on_connection_closed(stream);
        }
    }
}

void nabto_stream_connection_released(nabto_connect* con)
{
    struct nabto_stream_s* stream;

    for (stream = stream__; stream < stream__ + NABTO_MEMORY_STREAM_MAX_STREAMS; ++stream) {
        if (stream->connection == con && stream->state != STREAM_IDLE) {
            NABTO_LOG_TRACE(("Releasing stream, slot=%i, (connection closed) in state %" PRItext, unabto_stream_index(stream), nabto_stream_state_name(stream)));

            nabto_stream_tcb_on_connection_released(stream);
            stream->connection = NULL;
        }
    }
}

/******************************************************************************/
/******************************************************************************/

bool nabto_stream_validate_win(struct nabto_win_info* info, struct nabto_stream_s* stream)
{
    if (!(info->type & NP_PAYLOAD_WINDOW_FLAG_SYN)) {
        if (stream->idCP != info->idCP || stream->idSP != info->idSP) {
            NABTO_LOG_TRACE(("type: %i, illegal stream id (%i,%i), expect (%i,%i)", (int)info->type, info->idCP, info->idSP, stream->idCP, stream->idSP));
            return false;
        }
    }
    return true;
}


/******************************************************************************/
/******************************************************************************/

#if NABTO_ENABLE_NEXT_EVENT
/**
 * return next timestamp which should be considered.
 */
void nabto_stream_update_next_event(nabto_stamp_t* current_min_stamp)
{
    int i;
    for (i = 0; i < NABTO_MEMORY_STREAM_MAX_STREAMS; i++) {
        struct nabto_stream_s* stream = &stream__[i];
        if (stream->state != STREAM_IDLE) {
            if (stream->applicationEvents.dataReady || 
                stream->applicationEvents.dataWritten ||
                stream->applicationEvents.readClosed ||
                stream->applicationEvents.writeClosed ||
                stream->applicationEvents.closed ||
                stream->statisticsEvents.streamEnded)
            {
                nabto_stamp_t now = nabtoGetStamp();
                nabto_update_min_stamp(current_min_stamp, &now);
            }

            // If we have unacked packets then check then use the best timeout we know of
            nabto_stream_tcb_update_next_event(stream, current_min_stamp);
        }
    }
}
#endif

int unabto_stream_index(unabto_stream* stream) {
    return stream - stream__;
}


uint8_t* unabto_stream_stats_write_u32(uint8_t* ptr, uint8_t* end, uint8_t type, uint32_t value)
{
    if (ptr == NULL) {
        return NULL;
    }
    if (end - ptr < sizeof(uint32_t) + 2) {
        return NULL;
    }
    WRITE_FORWARD_U8(ptr, type);
    WRITE_FORWARD_U8(ptr, 6);
    WRITE_FORWARD_U32(ptr, value);
    return ptr;
}

uint8_t* unabto_stream_stats_write_u16(uint8_t* ptr, uint8_t* end, uint8_t type, uint16_t value)
{
    if (ptr == NULL) {
        return ptr;
    }
    if (end - ptr < sizeof(uint16_t) + 2) {
        return NULL;
    }
    WRITE_FORWARD_U8(ptr, type);
    WRITE_FORWARD_U8(ptr, 4);
    WRITE_FORWARD_U16(ptr, value);
    return ptr;
}

uint8_t* unabto_stream_stats_write_u8(uint8_t* ptr, uint8_t* end, uint8_t type, uint8_t value)
{
    if (ptr == NULL) {
        return NULL;
    }
    if (end - ptr < sizeof(uint8_t) + 2) {
        return NULL;
    }
    WRITE_FORWARD_U8(ptr, type);
    WRITE_FORWARD_U8(ptr, 3);
    WRITE_FORWARD_U8(ptr, value);
    return ptr;
}

uint8_t* unabto_stream_insert_stream_stats(uint8_t* ptr, uint8_t* end, struct nabto_stream_s* stream)
{
    uint8_t* payloadBegin = ptr;
    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_STREAM_STATS, 0, 0);

    // stream info
    ptr = unabto_stream_stats_write_u16(ptr, end, NP_PAYLOAD_STREAM_STATS_TAG,   stream->streamTag);
    ptr = unabto_stream_stats_write_u16(ptr, end, NP_PAYLOAD_STREAM_STATS_CP_ID, stream->idCP);
    ptr = unabto_stream_stats_write_u16(ptr, end, NP_PAYLOAD_STREAM_STATS_SP_ID, stream->idSP);
    // NP_PAYLOAD_STREAM_STATS_DURATION
    // NP_PAYLOAD_STREAM_STATS_STATUS

    // stream stats
    ptr = unabto_stream_stats_write_u32(ptr, end, NP_PAYLOAD_STREAM_STATS_SENT_PACKETS,              stream->stats.sentPackets);
    ptr = unabto_stream_stats_write_u32(ptr, end, NP_PAYLOAD_STREAM_STATS_SENT_BYTES,                stream->stats.sentBytes);
    ptr = unabto_stream_stats_write_u32(ptr, end, NP_PAYLOAD_STREAM_STATS_SENT_RESENT_PACKETS,       stream->stats.sentResentPackets);
    ptr = unabto_stream_stats_write_u32(ptr, end, NP_PAYLOAD_STREAM_STATS_RECEIVED_PACKETS,          stream->stats.receivedPackets);
    ptr = unabto_stream_stats_write_u32(ptr, end, NP_PAYLOAD_STREAM_STATS_RECEIVED_BYTES,            stream->stats.receivedBytes);
    ptr = unabto_stream_stats_write_u32(ptr, end, NP_PAYLOAD_STREAM_STATS_RECEIVED_RESENT_PACKETS,   stream->stats.receivedResentPackets);
    ptr = unabto_stream_stats_write_u32(ptr, end, NP_PAYLOAD_STREAM_STATS_REORDERED_OR_LOST_PACKETS, stream->stats.reorderedOrLostPackets);
    ptr = unabto_stream_stats_write_u32(ptr, end, NP_PAYLOAD_STREAM_STATS_USER_WRITE,                stream->stats.userWrite);
    ptr = unabto_stream_stats_write_u32(ptr, end, NP_PAYLOAD_STREAM_STATS_USER_READ,                 stream->stats.userRead);

    /* NP_PAYLOAD_STREAM_STATS_CONGESTION_CONTROL_RTT_MIN             */
    /* NP_PAYLOAD_STREAM_STATS_CONGESTION_CONTROL_RTT_MAX             */
    /* NP_PAYLOAD_STREAM_STATS_CONGESTION_CONTROL_RTT_AVG             */
    /* NP_PAYLOAD_STREAM_STATS_CONGESTION_CONTROL_CWND_MIN            */
    /* NP_PAYLOAD_STREAM_STATS_CONGESTION_CONTROL_CWND_MAX            */
    /* NP_PAYLOAD_STREAM_STATS_CONGESTION_CONTROL_CWND_AVG            */
    /* NP_PAYLOAD_STREAM_STATS_CONGESTION_CONTROL_SS_THRESHOLD_MIN    */
    /* NP_PAYLOAD_STREAM_STATS_CONGESTION_CONTROL_SS_THRESHOLD_MAX    */
    /* NP_PAYLOAD_STREAM_STATS_CONGESTION_CONTROL_SS_THRESHOLD_AVG    */
    /* NP_PAYLOAD_STREAM_STATS_CONGESTION_CONTROL_SENT_NOT_ACKED_MIN */
    /* NP_PAYLOAD_STREAM_STATS_CONGESTION_CONTROL_SENT_NOT_ACKED_MAX  */
    /* NP_PAYLOAD_STREAM_STATS_CONGESTION_CONTROL_SENT_NOT_ACKED_AVG */

    // insert payload length
    if (ptr != NULL) {
        WRITE_U16(payloadBegin + 2, ptr - payloadBegin);
    }
    return ptr;
}

void unabto_stream_send_stats(struct nabto_stream_s* stream, uint8_t event)
{
    size_t length;
    uint8_t* ptr = insert_header(nabtoCommunicationBuffer, 0, stream->connection->spnsi, NP_PACKET_HDR_TYPE_STATS, false, 0, 0, 0);
    uint8_t* end = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;

    ptr = insert_stats_payload(ptr, end, event);
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

    ptr = unabto_stream_insert_stream_stats(ptr, end, stream);
    if ( ptr == NULL) {
        return;
    }

    ptr = insert_connection_stats_payload(ptr, end, stream->connection);
    if (ptr == NULL) {
        return;
    }

    length = ptr - nabtoCommunicationBuffer;
    insert_length(nabtoCommunicationBuffer, length);
    send_to_basestation(nabtoCommunicationBuffer, length, &nmc.context.gsp);
}


#endif /* NABTO_ENABLE_STREAM && NABTO_ENABLE_MICRO_STREAM */
