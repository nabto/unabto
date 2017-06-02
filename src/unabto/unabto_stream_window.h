/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
/**
 * @file
 * Nabto uServer stream packet events from an unreliable con - Prototypes.
 *
 * Handling data packets from an unreliable  stream.
 */

#ifndef _UNABTO_STREAM_WINDOW_H_
#define _UNABTO_STREAM_WINDOW_H_

#if NABTO_ENABLE_STREAM_STANDALONE
#include "unabto_streaming_types.h"
#else
#include <unabto/unabto_env_base.h>
#endif

#if (NABTO_ENABLE_STREAM && NABTO_ENABLE_MICRO_STREAM)

#include <unabto/unabto_protocol_defines.h>
#include <unabto/unabto_stream.h>

/** Stream Transfer Control Block Configuration */
typedef struct nabto_stream_tcb_config
{
    uint16_t recvPacketSize;  ///< receiver packet size
    uint16_t recvWinSize;     ///< receiver window size
    uint16_t xmitPacketSize;  ///< transmitter packet size
    uint16_t xmitWinSize;     ///< transmitter window size
    uint16_t maxRetrans;      ///< max number of retransmissions
    uint16_t timeoutMsec;     ///< retransmission timeout
    bool     enableWSRF;      ///< enable Window Size Reduction Functionality
    bool     enableSACK;      ///< Enable selective Acknowledge
} nabto_stream_tcb_config;

/** Window information */
struct nabto_win_info {
    /* common fields */
    uint8_t  type;    /**< type        */
    uint8_t  version; /**< version     */
    uint16_t idCP;    /**< ID, CP part */
    uint16_t idSP;    /**< ID, SP part */
    uint32_t seq;     /**< sequence number             */
    uint32_t ack;     /**< acknowledge sequence number */
    uint16_t advertisedWindow; /**< Advertised window size */
    /** The resulting maximum send sequence number is ack+advertisedWindow. */

    /** fields depending on the type */
    union {
        /** SYN fields */
        struct {
            nabto_stream_tcb_config    cfg;     /**< configuration */
        } syn;
    } u;                                        /**< the union */
};

/** Stream Transfer Control Block states */
typedef enum {
    ST_IDLE,          ///< Before being used. Means it's a fresh resource.
    ST_SYN_SENT,      ///< Opened by this end.
    ST_SYN_RCVD,      ///< Opened as listener.
    ST_ESTABLISHED,   ///< Open waiting for, close or FIN.

    ST_FIN_WAIT_1,    ///< We have received a close from the user.
    ST_FIN_WAIT_2,    ///< Our FIN has been acked. Waiting for a FIN from the peer.
    ST_CLOSING,       ///< We have received a close from the user and afterwards we got a FIN from the peer.
    ST_TIME_WAIT,     ///< Waiting for retransmitted FINs, e.g. if the last sent ack to the peer was lost.

    ST_CLOSE_WAIT,    ///< We have received a FIN from the peer and waits for the user to close the stream.
    ST_LAST_ACK,      ///< We have received a close from the User but waits for our FIN to be acknowledged.

    ST_CLOSED,        ///< The stream is closed, the application can now safely release the stream resource.
    ST_CLOSED_ABORTED ///< The stream has been uncleanly closed, it's possible that data loss has happened.
} nabto_stream_tcb_state;


/** Receive buffer */
typedef struct {
    uint32_t seq;                   /**< sequence number                  */
    uint16_t size;                  /**< number of bytes in buffer        */
    uint16_t used;                  /**< number of bytes used by the user */
    uint8_t* buf;                   /**< buffer                           */
} r_buffer;


/** Buffer state */
typedef enum {
    B_IDLE, /**< unused        */
    B_DATA, /**< data not sent */
    B_SENT  /**< data sent     */
} b_state_t;


/** Transmit buffer */
typedef struct {
    uint32_t      seq;                   /**< sequence number       */
    uint16_t      size;                  /**< number of bytes       */
    uint8_t*      buf;                   /**< buffer                */
    b_state_t     xstate;                /**< state                 */
    uint16_t      nRetrans;              /**< Retansmission counter. */
    nabto_stamp_t sentStamp;             /**< When was the data sent. Used to measure rtt */
    uint16_t      ackedAfter;            /**< Number of buffers which
                                          * has been acked which is
                                          * after this buffer in the
                                          * window. */
    bool          shouldRetransmit;      /** true if this buffer should be retransmitted */
} x_buffer;


// structure which can tell some statistics about a double.

struct unabto_stats {
    double min;
    double max;
    double avg;
    double count;
};

typedef struct {
    struct unabto_stats rtt; // rount trip time
    struct unabto_stats cwnd; // congestion window size.
    struct unabto_stats ssThreshold; // slow start threshold
    struct unabto_stats flightSize; // data on the line
} nabto_stream_congestion_control_stats;

typedef struct {
    double        srtt;          ///< Smoothed round trip time.
    double        rttVar;        ///< Round trip time variance.
    uint16_t      rto;           ///< Retransmission timeout.
    bool          isFirstAck;    ///< True when the first ack has been received.
    double        cwnd;          ///< Tokens available for sending data
    double        ssThreshold;   ///< Slow start threshold
    int           flightSize;    ///< Gauge of sent but not acked buffers. Aka flight size.
    bool          lostSegment;   ///< True if a segment has been lost
                                 ///and we are running the fast
                                 ///retransmit / fast recovery
                                 ///algorithm
} nabto_stream_congestion_control;


/** Stream Transfer Control Block */
struct nabto_stream_tcb {
    nabto_stream_tcb_state          streamState;            /**< state                   */
    nabto_stream_tcb_config         initialConfig;          /**< Initial config sent as the first syn or used as a limit in the syn | ack */
    nabto_stream_tcb_config         cfg;                    /**< configuration (from SYN)    */
    uint16_t                        retransCount;           /**< Retransmission count,
                                                                 First used for retransmission of SYN,
                                                                 Then retransmission of FIN, and lastly used for
                                                                 waiting in ST_TIME_WAIT. */
    /**
     * The timeoutStamp is used to signal if the stream window should
     * do something. It's both used as regular timeout and a timeout
     * telling something to happen imidiately. This timeout is not
     * used for retransmission of regular data.
     */
    nabto_stamp_t                   timeoutStamp;
    /**
     * timeout stamp for data, this is regularly updated.
     */
    nabto_stamp_t                   dataTimeoutStamp;       /**< Timeout stamp for data  */
    nabto_stamp_t                   ackStamp;               /**< time to send unsolicited ACK         */
    uint32_t                        maxAdvertisedWindow;

    uint32_t                        finSequence;            /**< The sequence number of the fin. */

    /**
     * data between xmitFirst and xmitLastSent has not been acked yet
     * invariant xmitSeq >= xmitLastSent >= xmitFirst
     *
     * ready data = xmitFist .. xmitSeq
     *
     * data ready for first transmission = xmitLastSent .. xmitSeq
     *
     * not acked data = xmitFirst .. xmitLastSent
     *
     * old xmitCount = xmitSeq - xmitFirst
     */
    uint32_t                        xmitFirst;              /**< first sequence number not acked */
    uint32_t                        xmitLastSent;           /**< first sequence number not sent yet. */
    uint32_t                        xmitSeq;                /**< next seq to use when adding data to the xmit buffers */
    /**
     * Data between xmitAcked and xmitHighSend has not been acknowledgeg
     */
    x_buffer*                       xmit;                   /**< transmit window                      */

    int                             retransmitSegmentCount; /**< Exactly the number of segments with (shouldRetransmit == true) aka waiting for retransmission */
    
    uint16_t                        ackWSRFcount;           /**< count unsolicited acks               */

    uint32_t                        recvNext;               /**< next seq to receive data from */
    uint32_t                        recvTop;                /**< highest cumulative filled recv slots +1. aka the next ack number to send. */
    uint32_t                        recvMax;                /**< max sequence number of used recv slot. */
    uint32_t                        ackSent;                /**< last ack sent           */
    uint32_t                        recvFinSeq;             /**< The received sequence number the fin has, if this is
                                                               set the other end has sent a fin. */
    r_buffer*                       recv;                   /**< receive window          */

    nabto_stream_congestion_control cCtrl;
    nabto_stream_congestion_control_stats ccStats;

    // Receiving packets designated by a sequence number 'seq'
    // -------------------------------------------------------
    // The window is the 'seq' range [recvNext .. recvNext+cfg.recvWinSize[
    //
    // seq in range 'window' are valid received packets
    //
    // The user is reading data from slot 'seq', thus this slot isn't acked to the peer yet.
    // This means that packets in 'window' may be received more than once, if retransmitted by the peer
    //
    // If the slot 'recvNext' is empty, the peer is known to have sent packets in range
    // [recvNext-cfg.recvWinSize .. recvNext[
    // and if the slot 'recvNext' isn't empty, the peer is known to have sent packet in range
    // ]recvNext-cfg.recvWinSize .. recvNext]
    // Receiving these packets may be caused by the ack's to the peer being lost,
    // thus a new ack should be sent.
};

struct nabto_stream_sack_pair {
    uint32_t start;
    uint32_t end;
};

struct nabto_stream_sack_data {
    uint8_t nPairs;
    struct nabto_stream_sack_pair pairs[NP_PAYLOAD_SACK_MAX_PAIRS];
};




/******************************************************************************/

struct nabto_stream_s;
struct nabto_win_info;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * open a stream
 *
 * Precondition: The stream structure has been initialized and
 * prepared for opening by giving it a tag and a connection.
 */
void nabto_stream_tcb_open(struct nabto_stream_s* stream);

/**
 * Read data
 */
size_t nabto_stream_tcb_read(struct nabto_stream_s * stream, const uint8_t** buf, unabto_stream_hint* hint);

/**
 * Acknowledge data read
 */
bool nabto_stream_tcb_ack(struct nabto_stream_s * stream, const uint8_t* buf, size_t used);

/**
 * Write data
 */
size_t nabto_stream_tcb_write(struct nabto_stream_s * stream, const uint8_t* buf, size_t size);

/**
 * Determine number of bytes that can be written
 */
size_t nabto_stream_tcb_can_write(struct nabto_stream_s * stream);

/******************************************************************************/

 /**
 * Given a stream, a window, modify the stream state
 */
void nabto_stream_tcb_event(struct nabto_stream_s* stream,
                            struct nabto_win_info* win,
                            uint8_t*               start,
                            int                    dlen,
                            struct nabto_stream_sack_data* sack);
 /**
 * Verify if stream has pending xmit data
 * @param windowHasOpened is set if advertisedWindow goes from 0 -> != 0
 */
void nabto_stream_tcb_check_xmit(struct nabto_stream_s* stream, bool isTimedEvent, bool windowHasOpened);

/**
 * Query whether a valid stream is open.
 * @param stream  the stream
 * @return        true if open
 */
bool nabto_stream_tcb_is_open(const struct nabto_stream_tcb* tcb);

/**
 * Query whether a valid stream is open for reading.
 */
bool nabto_stream_tcb_is_readable(struct nabto_stream_s* stream);

/**
 * Query whether a valid stream is open for writing.
 */
bool nabto_stream_tcb_is_writeable(const struct nabto_stream_tcb* tcb);

/**
 * Query whether the stream is closed orderly - initiate closing if required.
 * @param stream  the stream
 * @return        true iff closed without data loss
 */
bool nabto_stream_tcb_close(struct nabto_stream_s* stream);

bool nabto_stream_tcb_force_close(struct nabto_stream_s* stream);

/**
 * Notification that connection has been closed
 */
void nabto_stream_tcb_on_connection_closed(struct nabto_stream_s * stream);

/**
 * Notify stream that it's connection has been released.
 */
void nabto_stream_tcb_on_connection_released(struct nabto_stream_s * stream);

/**
 * Return printable state name
 */
text nabto_stream_tcb_state_name(const struct nabto_stream_tcb* tcb);

/**
 * Initialize state
 */
void nabto_init_stream_tcb_state(struct nabto_stream_tcb* tcb, const struct nabto_win_info* info, struct nabto_stream_s* stream);


void nabto_stream_tcb_release(struct nabto_stream_s * stream);

bool nabto_stream_tcb_is_closed(struct nabto_stream_s * stream);

#if NABTO_ENABLE_NEXT_EVENT
/**
 * Update time stamp with the time until next event
 */
void nabto_stream_tcb_update_next_event(struct nabto_stream_s * stream, nabto_stamp_t* current_min_stamp);
#endif

void nabto_stream_make_rst_response_window(const struct nabto_win_info* win, struct nabto_win_info* rstWin);
bool nabto_stream_read_window(const uint8_t* start, uint16_t length, struct nabto_win_info* win);
uint16_t nabto_stream_window_payload_length(struct nabto_win_info* win);
bool nabto_stream_encode_window(const struct nabto_win_info* win, uint8_t* start, uint16_t* length);

// Calculate the advertised window in
uint16_t unabto_stream_advertised_window_size(struct nabto_stream_tcb* tcb);
uint32_t unabto_stream_ack_number_to_send(struct nabto_stream_tcb* tcb);


bool unabto_stream_create_sack_pairs(struct nabto_stream_s* stream, struct nabto_stream_sack_data* sackData);
/******************************************************************************/
/******************************************************************************/

/**
 * A nabto streaming congestion control algorithm should implement the following
 * functions. Further it should define the type nabto_stream_congestion_control_t
 * which it can use for store congestion control data
 */
void unabto_stream_update_congestion_control_receive_stats(struct nabto_stream_s * stream, uint16_t ix);

void unabto_stream_congestion_control_adjust_ssthresh_after_triple_ack(struct nabto_stream_tcb* tcb);

void unabto_stream_congestion_control_timeout(struct nabto_stream_s * stream);

void unabto_stream_congestion_control_init(struct nabto_stream_tcb* tcb);

/**
 * Called after a streaming data packet has been resent.
 */
void unabto_stream_congestion_control_resent(struct nabto_stream_tcb* tcb, uint16_t ix);


/**
 * Called after a streaming data packet has been sent.
 */
void unabto_stream_congestion_control_sent(struct nabto_stream_tcb* tcb, uint16_t ix);

/**
 * Called when an ack is handled.
 */
void unabto_stream_congestion_control_handle_ack(struct nabto_stream_tcb* tcb, uint16_t ix, bool ackStartOfWindow);

/**
 * Called before a data packet is sent to test if it's allowed to be sent.
 */
bool unabto_stream_congestion_control_can_send(struct nabto_stream_tcb* tcb);

/**
 * Send stream statistics packet
 */
void unabto_stream_send_stats(struct nabto_stream_s* stream, uint8_t event);


/**
 * observe a value.
 */
void unabto_stream_stats_observe(struct unabto_stats* stat, double value);

/**
 * get stream duration in ms
 */
uint32_t unabto_stream_get_duration(struct nabto_stream_s * stream);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* NABTO_ENABLE_STREAM && NABTO_ENABLE_MICRO_STREAM */

#endif
