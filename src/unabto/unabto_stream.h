/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_STREAM_H_
#define _UNABTO_STREAM_H_
/**
 * @file
 * Nabto uServer stream  - Interface.
 *
 * Handling streams.
 */

#if NABTO_ENABLE_STREAM_STANDALONE
#include "unabto_streaming_types.h"
#else
#include <unabto/unabto_env_base.h>
#endif

#if NABTO_ENABLE_STREAM

#if !NABTO_ENABLE_STREAM_STANDALONE
#include <unabto/unabto_connection.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct nabto_stream_s;

typedef struct nabto_stream_s unabto_stream;

/** Stream statisitics */
typedef struct unabto_stream_stats_s {
    unsigned int sentPackets;
    unsigned int sentBytes;
    unsigned int sentResentPackets;
    unsigned int receivedPackets;
    unsigned int receivedBytes;
    unsigned int receivedResentPackets;
    unsigned int reorderedOrLostPackets;
    unsigned int userWrite;
    unsigned int userRead;
    nabto_stamp_t streamStart;
    unsigned int timeouts;
    unsigned int timeFirstMBReceived; // ms, 0 if not set,
    unsigned int timeFirstMBSent; // ms, 0 if not set
    unsigned int rttMin;
    unsigned int rttMax;
    unsigned int rttAvg;
    unsigned int cwndMin;
    unsigned int cwndMax;
    unsigned int cwndAvg;
    unsigned int ssThresholdMin;
    unsigned int ssThresholdMax;
    unsigned int ssThresholdAvg;
    unsigned int flightSizeMin;
    unsigned int flightSizeMax;
    unsigned int flightSizeAvg;
    unsigned int sendSegmentAllocFailures;
    unsigned int recvWindowSize;
    unsigned int recvSegmentSize;
    unsigned int sendWindowSize;
    unsigned int sendSegmentSize;

} unabto_stream_stats;

#ifndef UNABTO_STREAM_STATS_MAKE_PRINTABLE
#define UNABTO_STREAM_STATS_MAKE_PRINTABLE(stats) (stats.sentPackets), (stats.sentBytes), (stats.sentResentPackets),(stats.receivedPackets), (stats.receivedBytes), (stats.receivedResentPackets), (stats.reorderedOrLostPackets), (stats.timeouts), (stats.rttAvg), (stats.cwndAvg), (stats.ssThresholdAvg), (stats.flightSizeAvg), (stats.sendSegmentAllocFailures), (stats.recvWindowSize), (stats.recvSegmentSize), (stats.sendWindowSize), (stats.sendSegmentSize)
#endif

#ifndef UNABTO_STREAM_STATS_PRI
#define UNABTO_STREAM_STATS_PRI "sentPackets: %u, sentBytes %u, sentResentPackets %u, receivedPackets %u, receivedBytes %u, receivedResentPackets %u, reorderedOrLostPackets %u, timeouts %u, rtt avg %u, cwnd avg %u, ssthreshold avg %u, flightSize avg %u, sendSegmentAllocFailures %u, recvWindowSize %u, recvSegmentSize %u, sendWindowSize %u, sendSegmentSize %u"
#endif

typedef enum
{
    UNABTO_STREAM_HINT_OK = 0,
    UNABTO_STREAM_HINT_STREAM_CLOSED = 1,
    UNABTO_STREAM_HINT_INVALID_STREAM = 2,
    UNABTO_STREAM_HINT_READ_OR_WRITE_ERROR = 3,
    UNABTO_STREAM_HINT_ARGUMENT_ERROR = 4,
    UNABTO_STREAM_HINT_UNABLE_TO_ACKNOWLEDGE = 5,
    UNABTO_STREAM_HINT_ACKNOWLEDGING_TOO_MUCH_DATA = 6
} unabto_stream_hint;

/**************** OPEN/CLOSE *******************************/

/**
 * Close a stream.
 * @param stream  the stream
 * @return
 * - true: if all data has been exchanged
 * - false: all data not yet exchanged, call again until returning true
 */
bool unabto_stream_close(unabto_stream* stream);

/**
 * Forcibly close a stream.
 * No more data will be sent or received.
 */
bool unabto_stream_force_close(unabto_stream* stream);

/**
 * Release the stream.
 * @param stream  the stream
 *
 * If called before nabto_stream_close() has returned true, data may be lost
 */
void unabto_stream_release(unabto_stream* stream);

/**************** Events ************************************
 *
 * These functions has to be implemented by the application.
 */

/**
 * When a new stream is accepted and ready for data transmission, the
 * framework calls this function.
 *
 * @param stream a pointer to the internal stream structure. This
 *               should be seen as an opaque pointer.
 */
void unabto_stream_accept(unabto_stream* stream);

#if NABTO_ENABLE_STREAM_EVENTS
/**
 * Get stream events when events happens. This function has to be
 * implemented by the application.
 */

typedef enum {
    /**
     * Data is ready on the stream for the application to consume.
     */   
    UNABTO_STREAM_EVENT_TYPE_DATA_READY,
    /**
     * Data has been acknowledged by the other peer, such that there
     * should be room for sending new data.
     */
    UNABTO_STREAM_EVENT_TYPE_DATA_WRITTEN,
    /**
     * The stream has previously been blocked in a send operation
     * because there was no more send buffers available on the
     * system. That condition has probably changed and the application
     * should try to send the data again.
     */
    UNABTO_STREAM_EVENT_TYPE_SEND_SEGMENT_AVAILABLE,
    /**
     * No more data can be written to the stream since the other peer
     * has closed for the transmission.
     */
    UNABTO_STREAM_EVENT_TYPE_WRITE_CLOSED,
    /**
     * No more data can be written to the stream, since it has been
     * closed by this end.
     */
    UNABTO_STREAM_EVENT_TYPE_READ_CLOSED,
    /**
     * The stream is closed, no more data can be written or readen
     */
    UNABTO_STREAM_EVENT_TYPE_CLOSED,
} unabto_stream_event_type;

void unabto_stream_event(unabto_stream* stream, unabto_stream_event_type event);
#else
#define unabto_stream_event(stream, event)
#endif

/**************** READ/WRITE *******************************/

/**
 * Read received data and acknowledge.
 * @param stream  the stream
 * @param buf     pointer to a buffer to receive the data
 * @param count   number of bytes in the buffer
 * @param hint    additional result information
 * -  UNABTO_STREAM_HINT_OK
 * -  UNABTO_STREAM_HINT_STREAM_CLOSED
 * -  UNABTO_STREAM_HINT_INVALID_STREAM
 * -  UNABTO_STREAM_HINT_READ_OR_WRITE_ERROR
 * -  UNABTO_STREAM_HINT_ARGUMENT_ERROR
 * -  UNABTO_STREAM_HINT_UNABLE_TO_ACKNOWLEDGE
 * @return        the number of bytes copied to the buffer
 */
 size_t unabto_stream_read_buf(unabto_stream* stream, uint8_t* buf, size_t size, unabto_stream_hint* hint);


/**
 * Retrieve access to received data.
 * @param stream  the stream
 * @param buf     to return a pointer to the received data
 * @param hint    additional result information
 * -  UNABTO_STREAM_HINT_READ_OR_WRITE_ERROR
 * -  UNABTO_STREAM_HINT_INVALID_STREAM
 * -  UNABTO_STREAM_HINT_STREAM_CLOSED
 * -  UNABTO_STREAM_HINT_OK
 * @return        the number of bytes in the buffer
 *
 * When returning data (positive result), the next call must be a call of nabto_stream_ack(),
 * else the same data will be delivered again.
 *
 * See nabto_stream_ack for an example.
 */
size_t unabto_stream_read(unabto_stream* stream, const uint8_t** buf, unabto_stream_hint* hint);


/**
 * Acknowledge data received.
 * @param stream  the stream
 * @param buf     the pointer returned by the preceeding nabto_stream_read())
 * @param hint    additional result information
 * -  UNABTO_STREAM_HINT_READ_OR_WRITE_ERROR
 * -  UNABTO_STREAM_HINT_INVALID_STREAM
 * -  UNABTO_STREAM_HINT_STREAM_CLOSED
 * -  UNABTO_STREAM_HINT_OK
 * @param size    the number of bytes acknowledged (must be less equal to the result of preceeding nabto_stream_read()),
 *                the function does not have to acknowledge all bytes received, remaining bytes will be delivered in next call of nabto_stream_read()
 * @return        true if data was acknowledged (parameters were valid)
 *
 * Usage:
 * @verbatim
 * unabto_stream *stream;
 * ...
 * while (1) {
 *   const uint8_t* buf;
 *   size_t size = nabto_stream_read(stream, &buf);
 *   if (size > 0) {
 *     size_t used = use_bytes_from_beginning_of_buffer(buf, size);
 *     
 *     if (!nabto_stream_ack(stream, buf, used)) {
 *       // the parameters were illegal, or the stream is no longer open, call ignored
 *     }
 *   }
 * } @endverbatim
 */
bool unabto_stream_ack(unabto_stream* stream, const uint8_t* buf, size_t used, unabto_stream_hint* hint);


/**
 * Write data to an open stream. Does not block, returns number of
 * bytes immediately accepted (sent or queued).
 * @param stream  the stream
 * @param buf     the buffer
 * @param size    the size of the buffer
 * @param hint    additional result information
 * -  UNABTO_STREAM_HINT_READ_OR_WRITE_ERROR
 * -  UNABTO_STREAM_HINT_INVALID_STREAM
 * -  UNABTO_STREAM_HINT_STREAM_CLOSED
 * -  UNABTO_STREAM_HINT_OK
 * @return        the number of bytes from buf being sent (or queued for sending)
 */
size_t unabto_stream_write(unabto_stream* stream, const uint8_t* buf, size_t size, unabto_stream_hint* hint);


/**
 * Returns the number of bytes that can safely be written to the stream.
 * @param stream  the stream
 * @param hint    additional result information
 * -  UNABTO_STREAM_HINT_READ_OR_WRITE_ERROR
 * -  UNABTO_STREAM_HINT_INVALID_STREAM
 * -  UNABTO_STREAM_HINT_STREAM_CLOSED
 * -  UNABTO_STREAM_HINT_OK
 * @return        the number of bytes which can currently be written to the stream.
 */
size_t unabto_stream_can_write(unabto_stream* stream, unabto_stream_hint* hint);


/**************** QUERY ************************************/


/**
 * Query whether a valid stream is open.
 * @param stream  the stream
 * @return        true if open
 */
bool unabto_stream_is_open(unabto_stream* stream);


/**
 * Query whether a valid stream is open for reading.
 */
bool unabto_stream_is_readable(unabto_stream* stream);


/**
 * Query whether a valid stream is open for writing.
 */
bool unabto_stream_is_writeable(unabto_stream* stream);


/**
 * Query whether a valid stream is closed.
 */
bool unabto_stream_is_closed(unabto_stream* stream);

#if !NABTO_ENABLE_STREAM_STANDALONE
/**
 * Retrieve the connection in which the stream is embedded.
 * @param stream  the stream
 * @return        the connection (0 if stream isn't valid)
 */
nabto_connect* unabto_stream_connection(unabto_stream* stream);


/**
 * Query connection info from stream
 */
int unabto_stream_get_connection_type(unabto_stream* stream, nabto_connection_type* connection_type);
#endif

/**
 * Query statistics from stream
 */
int unabto_stream_get_stats(unabto_stream* stream, unabto_stream_stats* stats);

/**
 * Get index of stream in the stream array.  This function can be
 * subject to removal in the future.
 */
int unabto_stream_index(unabto_stream* stream);


/**************** API which is used from the uNabto core ***************************/

/** Initialise the stream data structure */
void unabto_stream_init(void);

#if !NABTO_ENABLE_STREAM_STANDALONE
/**
 * Close all streams on a connection when the connection closes.
 * @param con  the connection
 */
void nabto_stream_connection_closed(nabto_connect* con);

void nabto_stream_connection_released(nabto_connect* con);
#endif


/** Treat timeouts in all streams. */
void unabto_time_event_stream(void);

#if NABTO_ENABLE_NEXT_EVENT
/** Find time stamp of next event occuring on any stream. */
void nabto_stream_update_next_event(nabto_stamp_t* current_min_stamp);
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* NABTO_ENABLE_STREAM */

#endif
