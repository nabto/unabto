/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_STREAM

/**
 * @file
 * Nabto uServer stream - Implementation.
 */

#if NABTO_ENABLE_STREAM_STANDALONE
#include "unabto_streaming_types.h"
#else
#include "unabto_env_base.h"
#endif

#if NABTO_ENABLE_STREAM && NABTO_ENABLE_MICRO_STREAM

#if NABTO_ENABLE_STREAM_STANDALONE
#else
#include "unabto_stream.h"
#include "unabto_stream_event.h"
#include "unabto_packet.h"

#include "unabto_util.h"
#endif

#include <unabto/unabto_stream_window.h>
#include <unabto/unabto_stream_environment.h>
#include <unabto/unabto_logging.h>
#include <unabto/unabto_stream_types.h>


#include <string.h>

void unabto_stream_init_data_structure(struct nabto_stream_s* stream) {
    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    memset(stream, 0, sizeof(struct nabto_stream_s));
    
    nabtoSetFutureStamp(&tcb->timeoutStamp, 0);
    nabtoSetFutureStamp(&tcb->dataTimeoutStamp, 0);
    nabtoSetFutureStamp(&tcb->dataExpireStamp, 0);
    nabtoSetFutureStamp(&tcb->ackStamp, 0);
    nabtoSetFutureStamp(&tcb->retransmitTimer, 0);

    stream->stats.streamStart = nabtoGetStamp();
}

bool unabto_stream_is_open(unabto_stream* stream)
{
    return nabto_stream_tcb_is_open(&stream->u.tcb);
}

/******************************************************************************/

bool unabto_stream_is_readable(unabto_stream* stream) {
    return nabto_stream_tcb_is_readable(stream);
}

size_t unabto_stream_read(unabto_stream* stream, const uint8_t** buf, unabto_stream_hint* hint)
{
    size_t avail = 0;
    stream->stats.userRead++;
    avail = nabto_stream_tcb_read(stream, buf, hint);
    return avail;
}

bool unabto_stream_ack(unabto_stream* stream, const uint8_t* buf, size_t used, unabto_stream_hint* hint)
{
    *hint = UNABTO_STREAM_HINT_OK;
    return nabto_stream_tcb_ack(stream, buf, used);
}

size_t unabto_stream_read_buf(unabto_stream* stream, uint8_t* buf, size_t size, unabto_stream_hint* hint)
{
    size_t avail = 0;
    size_t readen = 0;
    const uint8_t * mem;
    if (buf == 0 || size == 0) {
        *hint = UNABTO_STREAM_HINT_ARGUMENT_ERROR;
        return 0;
    }
    do {
        avail = nabto_stream_tcb_read(stream, &mem, hint);
        if (*hint != UNABTO_STREAM_HINT_OK) {
            return 0;
        }
        if (avail > 0) {
            if (avail > size) avail = size;
            memcpy(buf, mem, avail);
            buf += avail;
            size -= avail;
            readen += avail;
            if (!nabto_stream_tcb_ack(stream, mem, avail)) {
                *hint = UNABTO_STREAM_HINT_UNABLE_TO_ACKNOWLEDGE;
                return 0;
            }
        }
    } while (avail > 0 && size > 0);
    *hint = UNABTO_STREAM_HINT_OK;
    return readen;
}

/******************************************************************************/

bool unabto_stream_is_writeable(unabto_stream* stream) {
    return nabto_stream_tcb_is_writeable(&stream->u.tcb);
}

size_t unabto_stream_write(unabto_stream* stream, const uint8_t* buf, size_t size, unabto_stream_hint* hint)
{
    size_t queued = 0;

    stream->stats.userWrite++;

    NABTO_LOG_TRACE(("write size=%" PRIsize, size));
    if (!nabto_stream_tcb_is_writeable(&stream->u.tcb)) {
        *hint = UNABTO_STREAM_HINT_READ_OR_WRITE_ERROR;
        return 0;
    }
    queued = nabto_stream_tcb_write(stream, buf, size);

    NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_BUFFERS, ("Wrote on stream size %" PRIsize, queued), buf, queued);
    *hint = UNABTO_STREAM_HINT_OK;
    return queued;
}

size_t unabto_stream_can_write(unabto_stream* stream, unabto_stream_hint* hint)
{
    *hint = UNABTO_STREAM_HINT_OK;

    if (!nabto_stream_tcb_is_writeable(&stream->u.tcb)) {
        *hint = UNABTO_STREAM_HINT_READ_OR_WRITE_ERROR;
        return 0;
    }
    return  nabto_stream_tcb_can_write(stream);
}

/******************************************************************************/

bool unabto_stream_is_closed(unabto_stream* stream) {
    return nabto_stream_tcb_is_closed(stream);
}

bool unabto_stream_close(unabto_stream* stream)
{
    //NABTO_LOG_INFO(("timed out %d times", stream->stats.timeouts));
    return nabto_stream_tcb_close(stream);
}

bool unabto_stream_force_close(unabto_stream* stream) 
{
    return nabto_stream_tcb_force_close(stream);
}

void unabto_stream_release(unabto_stream* stream)
{
    nabto_stream_tcb_release(stream);
    stream_reset(stream);
}

/******************************************************************************/
#if !NABTO_ENABLE_STREAM_STANDALONE
nabto_connect* unabto_stream_connection(unabto_stream* stream)
{
    return stream->connection;
}
#endif

#if !NABTO_ENABLE_STREAM_STANDALONE
int unabto_stream_get_connection_type(unabto_stream* stream, nabto_connection_type* connection_type) {
    *connection_type = get_connection_type(stream->connection);
    return 0;
}
#endif

int unabto_stream_get_stats(unabto_stream* stream, unabto_stream_stats* stats) {

    stream->stats.rttMin = stream->u.tcb.ccStats.rtt.min;
    stream->stats.rttMax = stream->u.tcb.ccStats.rtt.max;
    stream->stats.rttAvg = stream->u.tcb.ccStats.rtt.avg;
    stream->stats.cwndMin = stream->u.tcb.ccStats.cwnd.min;
    stream->stats.cwndMax = stream->u.tcb.ccStats.cwnd.max;
    stream->stats.cwndAvg = stream->u.tcb.ccStats.cwnd.avg;
    stream->stats.ssThresholdMin = stream->u.tcb.ccStats.ssThreshold.min;
    stream->stats.ssThresholdMax = stream->u.tcb.ccStats.ssThreshold.max;
    stream->stats.ssThresholdAvg = stream->u.tcb.ccStats.ssThreshold.avg;
    stream->stats.sentNotAckedMin = stream->u.tcb.ccStats.sentNotAcked.min;
    stream->stats.sentNotAckedMax = stream->u.tcb.ccStats.sentNotAcked.max;
    stream->stats.sentNotAckedAvg = stream->u.tcb.ccStats.sentNotAcked.avg;
    *stats = stream->stats;
    return 0;
}

text nabto_stream_state_name(unabto_stream* stream) {
    return nabto_stream_tcb_state_name(&stream->u.tcb);
}

#endif /* NABTO_ENABLE_STREAM && NABTO_ENABLE_MICRO_STREAM */
