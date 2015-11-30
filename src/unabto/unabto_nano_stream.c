/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_STREAM

/**
 * @file
 * Nabto uServer stream - Implementation.
 */
#include "unabto_env_base.h"

#if NABTO_ENABLE_STREAM && NABTO_ENABLE_NANO_STREAM

#include "unabto_stream.h"
#include "unabto_packet.h"
#include "unabto_logging.h"
#include "unabto_util.h"
#include "unabto_memory.h"
#include "unabto_external_environment.h"
#include "unabto_next_event.h"

#include <string.h>


static NABTO_THREAD_LOCAL_STORAGE  uint16_t            idSP__ = 0;       /**< the one and only */



/**
 * Handle a Nabto Stream Packet (internal usage).
 * @param con    the connection
 * @param hdr    the packet header
 * @param info   the window information payload
 * @param start  the decrypted payload
 * @param dlen   the decrypted payload
 */
void nabto_stream_event(nabto_connect*       con,
                        nabto_packet_header* hdr,
                        uint8_t*             info,
                        uint8_t*             dataStart,
                        int                  dataLength);

/** Treat timeouts in all streams. */
void unabto_time_event_stream(void);

/******************************************************************************/

typedef enum {
    ST_IDLE,          ///< Before being used. Means it's a fresh resource.
    ST_SYN_RCVD,      ///< Opened as listener.
    ST_ESTABLISHED,   ///< Open waiting for, close or FIN.

    ST_FIN_WAIT_1,    ///< We have received a close from the user.
    ST_FIN_WAIT_2,    ///< Our FIN has been acked. Waiting for a FIN from the peer.
    ST_CLOSING,       ///< We have received a close from the user and afterwards we got a FIN from the peer.
    ST_TIME_WAIT,     ///< Waiting for retransmitted FINs, e.g. if the last sent ack to the peer was lost.

    ST_CLOSE_WAIT,    ///< We have received a FIN from the peer and waits for the user to close the stream.
    ST_LAST_ACK,      ///< We have received a close from the User but waits for our FIN to be acknowledged.

    ST_CLOSED,        ///< The stream is closed, the application can now safely release the stream resource.
    ST_CLOSED_ABORTED ///< The stream is uncleanly closed.
} unabto_nano_stream_state;

/** Receive buffer */
typedef struct {
    uint32_t seq;                   /**< sequence number                  */
    uint16_t size;                  /**< number of bytes in buffer        */
    uint16_t used;                  /**< number of bytes used by the user */
    uint8_t*  buf;                  /**< buffer                           */
} nano_r_buffer;


/** Buffer state */
enum {
    B_IDLE, /**< unused        */
    B_DATA, /**< data not sent */
    B_SENT  /**< data sent     */
};

/** Transmit buffer */
typedef struct {
    uint32_t      seq;                   /**< sequence number       */
    uint16_t      size;                  /**< number of bytes       */
    uint8_t*      buf;                   /**< buffer                */
    uint8_t       xstate;                /**< state                 */
    nabto_stamp_t expireStamp;           /**< Timestamp when the
                                          * transmission expires. This
                                          * stamp is set when the
                                          * packet is sent the first
                                          * time. */
    bool          fin;                   /**< FIN flag              */
    nabto_stamp_t sentStamp;             /**< When was the data sent. */
    nabto_stamp_t sentStampTimeout;      /**< Precalculated timeout stamp. */
} nano_x_buffer;


/** Stream Transfer Control Block */
struct nano_stream_state {
    uint8_t                        retransCount;           /**< Retransmission count,
                                                                 First used for retransmission of SYN,
                                                                 Then retransmission of FIN, and lastly used for 
                                                                 waiting in ST_TIME_WAIT. */
    uint16_t                        timeoutData;            ///< Timeout for retransmission of data packets in ms.
    nabto_stamp_t                   timeoutStamp;           /**< Timeout stamp           */
    uint8_t                         xmitCount;              /**< number of outstanding
                                                                * = not_sent+awaiting_ack   */
    uint32_t                        xmitSeq;                /**< next seq to send        */
    bool                            finSent;                 /**< The sequence number of the fin. */
    uint8_t                         xmitLastSent;           /**< index into xmit[] of first buffer not sent yet. */
    uint8_t                         xmitFirst;              /**< index into xmit[] of first not acked */
    nano_x_buffer                        xmit[NABTO_STREAM_SEND_WINDOW_SIZE]; /**< transmit window                      */
 
    uint32_t                        recvNext;               /**< next seq to receive     */
    uint32_t                        ackSent;                /**< If we have sent an ack on the value recvNext points to */
    nano_r_buffer                        recv[NABTO_STREAM_RECEIVE_WINDOW_SIZE]; /**< receive window          */
 


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
    // and if the slot 'recvNext' isn't empty, the peer is known to have sent kacket in range
    // ]recvNext-cfg.recvWinSize .. recvNext]
    // Receiving these packets may be caused by the ack's to the peer being lost,
    // thus a new ack should be sent.
};

typedef struct 
{
    uint16_t recvPacketSize;  ///< receiver packet size
    uint8_t  recvWinSize;     ///< receiver window size
    uint16_t xmitPacketSize;  ///< transmitter packet size
    uint8_t  xmitWinSize;     ///< transmitter window size
    uint8_t  maxRetrans;      ///< max number of retransmissions
    uint16_t timeoutMsec;     ///< retransmission timeout
} unabto_nano_stream_config;

/** Stream state */
struct nabto_stream_s {
    uint16_t                          streamTag;       /**< the tag                     */
    nabto_connect*                    connection;      /**< the connection              */
    uint8_t                           state;           /**< the state of the entry      */
    uint16_t                          idCP;            /**< ID, client part             */
    uint16_t                          idSP;            /**< ID, serveer part            */
    struct {
        bool                          dataReady : 1;
        bool                          dataWritten : 1;
        bool                          readClosed : 1;
        bool                          writeClosed : 1;
        bool                          closed : 1;
    } applicationEvents;

    unabto_stream_stats               stats;           /**< Stats for the stream        */
    unabto_nano_stream_config         cfg;
    struct nano_stream_state          tcb;             /**< state for unreliable con     */
};

/** Window information */
struct nabto_win_info {
    /* common fields */
    uint8_t  type;    /**< type        */
    uint8_t  version; /**< version     */
    uint16_t idCP;    /**< ID, CP part */
    uint16_t idSP;    /**< ID, SP part */
    uint32_t seq;     /**< sequence number             */
    uint32_t ack;     /**< acknowledge sequence number */

    uint16_t                   options; /**< options       */
    unabto_nano_stream_config  cfg;     /**< configuration */
};

/**
 * Initialize state
 */
void nano_stream_init_state(unabto_stream * stream, const struct nabto_win_info* info);

#define NABTO_STREAM_TIMEOUT           1000  /**< The default stream timeout (msec) */
#define NABTO_STREAM_MAX_RETRANS         12  /**< Max number of retransmission of stream messages */


/******************************************************************************/
 

#if UNABTO_PLATFORM_PIC18
#pragma udata big_mem
#endif

static NABTO_THREAD_LOCAL_STORAGE struct nabto_stream_s nano_stream__[NABTO_STREAM_MAX_STREAMS];  /**< a pool of streams */
static NABTO_THREAD_LOCAL_STORAGE uint8_t r_buffer_data[NABTO_STREAM_MAX_STREAMS * NABTO_STREAM_RECEIVE_SEGMENT_SIZE * NABTO_STREAM_RECEIVE_WINDOW_SIZE];
static NABTO_THREAD_LOCAL_STORAGE uint8_t x_buffer_data[NABTO_STREAM_MAX_STREAMS * NABTO_STREAM_SEND_SEGMENT_SIZE * NABTO_STREAM_SEND_WINDOW_SIZE];

#if UNABTO_PLATFORM_PIC18
#pragma udata
#endif

static bool nano_stream_is_ack_on_fin(struct nano_stream_state* tcb, struct nabto_win_info* win);
static bool nano_stream_handle_fin(struct nano_stream_state* tcb, struct nabto_win_info* win);

static void nano_stream_check_new_data_xmit(struct nabto_stream_s* stream);
static void nano_stream_check_retransmit_data(struct nabto_stream_s* stream);

/******************************************************************************/

 /**
 * Verify if stream has pending xmit data
 */
void nano_stream_check_xmit(struct nabto_stream_s* stream, bool isTimedEvent);

void nano_stream_state_transition(struct nabto_stream_s* stream, uint8_t new_state);

/******************************************************************************/

#define LIMITED(value, min, max)  ((value) >= (max) ? (max) : (value) <= (min) ? (min) : (value))




static void handle_nano_stream_packet(nabto_connect* con, nabto_packet_header* hdr,
                                      uint8_t* start, uint16_t dlen,
                                      uint8_t* payloadsStart, uint8_t* payloadsEnd,
                                      void* userData);

/**
 * Find an already active stream
 */
static unabto_stream* find_nano_stream(uint16_t tag, nabto_connect* con);

static uint16_t nano_stream_next_sp_id(void);

static uint8_t nano_stream_read_win(const uint8_t* payload,
                                    uint16_t len,
                                    struct nabto_win_info* info,
                                    unabto_stream* stream, 
                                    int dlen);

/**
 * Return true if data was acked.
 */
static void nano_stream_handle_data(struct nabto_stream_s* stream,
                                    struct nabto_win_info* win,
                                    uint8_t *              start,
                                    int                    dlen);

enum {
    //SEQ_EXT = 0x01,
    //ACK_EXT = 0x02,

    ACK     = NP_PAYLOAD_WINDOW_FLAG_ACK,
  //FAC     = NP_PAYLOAD_WINDOW_FLAG_FAC,
    FIN     = NP_PAYLOAD_WINDOW_FLAG_FIN,
    SYN     = NP_PAYLOAD_WINDOW_FLAG_SYN
};

#define nano_stream_event_error(stream, win) NABTO_LOG_ERROR(("Stream in state %i could not handle window of type %i, ack %i, seq %i", stream->state, (win)->type, (win)->ack, (win)->seq))


/******************************************************************************/

void handle_nano_stream_packet(nabto_connect* con, nabto_packet_header* hdr,
                               uint8_t* start, uint16_t dlen,
                               uint8_t* payloadsStart, uint8_t* payloadsEnd,
                               void* userData) {
    
    // read the window payload
    
    uint8_t* payloadWindowStart;
    uint16_t payloadWindowLength;
    struct nabto_win_info win;
    unabto_stream*         stream;
    struct nano_stream_state* tcb;
    bool ackEqXmitSeq, seqEqRecvNext;

    NABTO_NOT_USED(userData);

    if(!find_payload(payloadsStart, payloadsEnd, NP_PAYLOAD_TYPE_WINDOW, &payloadWindowStart, &payloadWindowLength)) {
        NABTO_LOG_ERROR(("Stream packet without window information"));
        return;
    }
    
    NABTO_LOG_DEBUG(("(.%i.) STREAM EVENT, dlen: %i", hdr->nsi_sp, dlen));
    nabtoSetFutureStamp(&con->stamp, con->timeOut);

    stream = find_nano_stream(hdr->tag, con);
    
    if (stream == NULL) {
        NABTO_LOG_DEBUG(("Stream with tag %i not accepted", hdr->tag));
        return;
    }
   
    NABTO_LOG_TRACE(("(.%" PRIu32 ".) Stream with tag %" PRIu16 " accepted, slot=%" PRIsize, con->spnsi, hdr->tag, unabto_stream_index(stream)));

    stream->stats.receivedPackets++;
 
    if (!nano_stream_read_win(payloadWindowStart + SIZE_PAYLOAD_HEADER, payloadWindowLength, &win, stream, dlen)) {
        NABTO_LOG_DEBUG(("ReadWin failure %" PRIu16, payloadWindowLength));
        return;
    }

    tcb = &stream->tcb;

    ackEqXmitSeq = (win.ack == tcb->xmitSeq);
    seqEqRecvNext = (win.seq == tcb->recvNext);


    switch (stream->state) {
        case ST_IDLE:
            if (win.type == SYN) {
                nano_stream_init_state(stream, &win); // calls nabto_init_stream_tcb_state
                nano_stream_state_transition(stream, ST_SYN_RCVD);
            } else {
                nano_stream_event_error(stream, &win);
            }
            break;

        case ST_SYN_RCVD:
            if (win.type == ACK || win.type == (SYN|ACK)) {
                if (seqEqRecvNext && ackEqXmitSeq) {
                    nano_stream_state_transition(stream, ST_ESTABLISHED);
                }
            } else if (win.type == (FIN | ACK)) {
                if (seqEqRecvNext && ackEqXmitSeq) {
                    nano_stream_state_transition(stream, ST_CLOSED);
                }
            } else {
                nano_stream_event_error(stream, &win);
            }
            break;

        case ST_ESTABLISHED:
            if ((win.type == ACK) || (win.type == (FIN | ACK))) {
                nano_stream_handle_data(stream, &win, start, dlen); /* sequence numbers are checked by caller */
                
                if (win.type == (FIN | ACK)) {
                    if(nano_stream_handle_fin(tcb, &win)) {
                        nano_stream_state_transition(stream, ST_CLOSE_WAIT);
                    }
                }
            } else {
                nano_stream_event_error(stream, &win);
            }

            break;

        case ST_LAST_ACK:
            // The peer has closed for sending anymore data
            // but acks still come through.

            if (win.type == ACK || win.type == (FIN | ACK)) {
                nano_stream_handle_data(stream, &win, 0, 0);

                if (tcb->finSent) {
                    if (tcb->xmitCount == 0 && ackEqXmitSeq) {
                        nano_stream_state_transition(stream, ST_CLOSED);
                    }
                }
                
                if (win.type == (FIN | ACK)) {
                    NABTO_LOG_TRACE(("Retransmission of FIN|ACK"));
                }
            } else {
                nano_stream_event_error(stream, &win);
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
            NABTO_LOG_TRACE(("Got packet in ST_FIN_WAIT_1 ack: %i, seq: %i, type: %i", win.ack, win.seq, win.type));
            if (win.type == ACK || win.type == (FIN | ACK)) {
                nano_stream_handle_data(stream, &win, start, dlen); /* sequence numbers are checked by caller */

                if (win.type == ACK) {
                    

                    if (nano_stream_is_ack_on_fin(tcb, &win)) {
                        nano_stream_state_transition(stream, ST_FIN_WAIT_2);
                    }
                } else if(win.type == (FIN | ACK)) {
                    // If we have sent a fin and the acknowledge is on this fin
                    // then we should goto state TIME_WAIT.
                    // Else if our FIN has not been acked we should go to the
                    // state CLOSING.
                    if (nano_stream_is_ack_on_fin(tcb, &win) && nano_stream_handle_fin(tcb, &win)) {
                        nano_stream_state_transition(stream, ST_TIME_WAIT);
                    } else if (nano_stream_handle_fin(tcb, &win)) {
                        nano_stream_state_transition(stream, ST_CLOSING);
                    } else {
                        nano_stream_event_error(stream, &win);
                    }
                }
            } else {
                nano_stream_event_error(stream, &win);
            }
            break;

            // In this case we can receive data or get a FIN, all our
            // outstanding data is acked and our FIN is acked.
        case ST_FIN_WAIT_2:
            if (win.type == ACK || win.type == (FIN | ACK)) {
                nano_stream_handle_data(stream, &win, start, dlen);

                if (win.type == (FIN | ACK)) {
                    if (nano_stream_handle_fin(tcb, &win)) {
                        nano_stream_state_transition(stream, ST_TIME_WAIT);
                    }
                }
            } else {
                nano_stream_event_error(stream, &win);
            }
            break;

            // We have received a fin from the other end. But still needs an
            // ack or has outstanding nonacked data.
        case ST_CLOSING:
            if (win.type == ACK || win.type == (FIN | ACK) ) {
                nano_stream_handle_data(stream, &win, start, dlen);
                if (nano_stream_is_ack_on_fin(tcb, &win)) {
                    nano_stream_state_transition(stream, ST_TIME_WAIT);
                }
            } else {
                nano_stream_event_error(stream, &win);
            }
            break;
            
        case ST_TIME_WAIT:
            if (win.type == ACK || win.type == (FIN | ACK) ) {
                nano_stream_handle_data(stream, &win, start, dlen);
            } else {
                nano_stream_event_error(stream, &win);
            }
            break;

        default:
            break;
    }
}

void unabto_time_event_stream(void)
{
    uint8_t i;
    for (i = 0; i < NABTO_STREAM_MAX_STREAMS; ++i) {
        struct nabto_stream_s* stream = &nano_stream__[i];
        if (stream->state != ST_IDLE && stream->state != ST_CLOSED && stream->state != ST_CLOSED_ABORTED) {
            
            nano_stream_check_xmit(stream, true);

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
    memset(nano_stream__, 0, sizeof(nano_stream__));

    unabto_packet_set_handler(NP_PACKET_HDR_TAG_STREAM_MIN, NP_PACKET_HDR_TAG_STREAM_MAX, handle_nano_stream_packet, 0);
}

/**
 * Initialise the stream data. @param stream the stream data
 */
void nano_stream_reset(unabto_stream* stream)
{
    memset(stream, 0, sizeof(unabto_stream));
}

/**
 * Find a stream
 * @param tag  the stream tag
 * @param con  the connection
 * @return     the stream (0 if not found)
 */
unabto_stream* find_nano_stream(uint16_t tag, nabto_connect* con)
{
    uint8_t i;
    for (i = 0; i < NABTO_STREAM_MAX_STREAMS; ++i) {
        
        if (nano_stream__[i].streamTag == tag && 
            nano_stream__[i].connection == con) {
            return nano_stream__ + i;
        }
    }
    
    for (i = 0; i < NABTO_STREAM_MAX_STREAMS; ++i) {
        if (nano_stream__[i].state == ST_IDLE) {
            unabto_stream* stream = nano_stream__ + i;
            nano_stream_reset(stream);
            stream->streamTag  = tag;
            stream->connection = con;
            return stream;
        }
    }

    return NULL;
}

/**
 * Initialise the stream configuration from received request.
 * @param stream  the stream
 * @param info    the request
 */
void nano_stream_init_state(unabto_stream* stream, const struct nabto_win_info* info)
{
  int i;

    stream->idCP = info->idCP;
    stream->idSP = nano_stream_next_sp_id();

    stream->cfg.recvPacketSize = LIMITED(info->cfg.recvPacketSize, 0, NABTO_STREAM_RECEIVE_SEGMENT_SIZE);
    stream->cfg.recvWinSize    = LIMITED(info->cfg.recvWinSize,    1, NABTO_STREAM_RECEIVE_WINDOW_SIZE);
    stream->cfg.xmitPacketSize = LIMITED(info->cfg.xmitPacketSize, 0, NABTO_STREAM_SEND_SEGMENT_SIZE);
    stream->cfg.xmitWinSize    = LIMITED(info->cfg.xmitWinSize,    1, NABTO_STREAM_SEND_WINDOW_SIZE);
    stream->cfg.maxRetrans     = LIMITED(info->cfg.maxRetrans,     0, NABTO_STREAM_MAX_RETRANS);
    stream->cfg.timeoutMsec    = LIMITED(info->cfg.timeoutMsec,    NABTO_STREAM_TIMEOUT, NABTO_STREAM_TIMEOUT);

    stream->tcb.xmitSeq = 30;         /* some arbitrary value    */
    stream->tcb.xmitFirst = stream->tcb.xmitSeq % stream->cfg.xmitWinSize;
    stream->tcb.xmitLastSent = stream->tcb.xmitSeq % stream->cfg.xmitWinSize;
    stream->tcb.recvNext = info->seq + 1; /* initial value in tcb */
    stream->tcb.retransCount = 0;
    memset(stream->tcb.xmit, 0, sizeof(stream->tcb.xmit));
    for (i=0;i< NABTO_STREAM_SEND_WINDOW_SIZE;i++)
    {
      stream->tcb.xmit[i].buf = &x_buffer_data[(unabto_stream_index(stream) * NABTO_STREAM_SEND_SEGMENT_SIZE * NABTO_STREAM_SEND_WINDOW_SIZE) + (i * NABTO_STREAM_SEND_SEGMENT_SIZE)];
    }

    memset(stream->tcb.recv, 0, sizeof(stream->tcb.recv));
    for (i=0;i< NABTO_STREAM_RECEIVE_WINDOW_SIZE;i++)
    {
      stream->tcb.recv[i].buf = &r_buffer_data[(unabto_stream_index(stream) * NABTO_STREAM_RECEIVE_SEGMENT_SIZE * NABTO_STREAM_RECEIVE_WINDOW_SIZE) + (i * NABTO_STREAM_RECEIVE_SEGMENT_SIZE)];
    }

    stream->tcb.timeoutData = stream->cfg.timeoutMsec;
}

/******************************************************************************/

uint16_t nano_stream_next_sp_id(void) {
   uint16_t idSP;
   do { idSP = ++idSP__; } while (idSP == 0);
   return idSP;
}

void nabto_stream_connection_closed(nabto_connect* con)
{
    unabto_stream* stream;

    for (stream = nano_stream__; stream < nano_stream__ + NABTO_STREAM_MAX_STREAMS; ++stream) {
        if (stream->connection == con && stream->state != ST_IDLE) {
            NABTO_LOG_TRACE(("Releasing stream, slot=%" PRIsize ", (connection closed) in state %i", unabto_stream_index(stream), stream->state));

            if (stream->state < ST_ESTABLISHED) {
                // The application does not know this stream
                nano_stream_state_transition(stream, ST_IDLE);
            } else {
                if (stream->state == ST_CLOSED || stream->state == ST_CLOSED_ABORTED) {
                } else {
                    // the application should release the stream
                    nano_stream_state_transition(stream, ST_CLOSED_ABORTED);
                }
            }
        }
    }
}

void nabto_stream_connection_released(nabto_connect* con)
{
    unabto_stream* stream;

    for (stream = nano_stream__; stream < nano_stream__ + NABTO_STREAM_MAX_STREAMS; ++stream) {
        if (stream->connection == con && stream->state != ST_IDLE) {
            NABTO_LOG_TRACE(("Releasing stream, slot=%" PRIsize ", (connection closed) in state %i", unabto_stream_index(stream), stream->state));

            stream->connection = NULL;
        }
    }
}

/******************************************************************************/
/******************************************************************************/

/**
 * Read the WINDOW payload
 * @param payload  the start of the WINDOW payload data
 * @param len      the length of the WINDOW payload data
 * @param info     to write the data
 * @param stream   the stream
 * @return         1 if valid (and info filled) - 0 on error
 *
 * Parameter
 * - dlen          the length of data (for logging)
 */
uint8_t nano_stream_read_win(const uint8_t* payload, //WINDOW payload without payload header
                              uint16_t len,
                              struct nabto_win_info* info,
                              unabto_stream* stream, 
                              int dlen)
{
//    NABTO_DECLARE_LOCAL_MODULE(nabto::Log::STREAM);

    if (len < NP_PAYLOAD_WINDOW_BYTELENGTH - NP_PAYLOAD_HDR_BYTELENGTH) return 0;
    READ_U8(info->type,    payload     );
    READ_U8(info->version, payload +  1);
    READ_U16(info->idCP,   payload +  2);
    READ_U16(info->idSP,   payload +  4);
    READ_U32(info->seq,    payload +  6);
    READ_U32(info->ack,    payload + 10);
    if (info->type == NP_PAYLOAD_WINDOW_FLAG_NON) {
        NABTO_LOG_ERROR(("failed to read window"));
        return 0;
    }
    if (info->type == NP_PAYLOAD_WINDOW_FLAG_SYN) {
        if (len < NP_PAYLOAD_WINDOW_SYN_BYTELENGTH - NP_PAYLOAD_HDR_BYTELENGTH) return 0;
        READ_U16(info->options,            payload + 14);

        /* we swap xmit and recv when reading because we are "the other end" */
        READ_U16(info->cfg.xmitPacketSize, payload + 16);
        READ_U16(info->cfg.xmitWinSize,    payload + 18);
        READ_U16(info->cfg.recvPacketSize, payload + 20);
        READ_U16(info->cfg.recvWinSize,    payload + 22);
        READ_U16(info->cfg.maxRetrans,     payload + 24);
        READ_U16(info->cfg.timeoutMsec,    payload + 26);
    } else {
        if (stream->idCP != info->idCP || stream->idSP != info->idSP) {
            NABTO_LOG_TRACE(("type: %i, illegal stream id (%i,%i), expect (%i,%i)", (int)info->type, info->idCP, info->idSP, stream->idCP, stream->idSP));
            return 0;
        }
    }

    return 1;
}

/******************************************************************************/
/******************************************************************************/

#if NABTO_ENABLE_NEXT_EVENT
/**
 * return next timestamp which should be considered.
 */
void nabto_stream_update_next_event(nabto_stamp_t* current_min_stamp)
{
    uint8_t i;
    for (i = 0; i < NABTO_STREAM_MAX_STREAMS; i++) {
        unabto_stream* stream = &nano_stream__[i];
        if (stream->state != ST_IDLE && stream->state != ST_CLOSED && stream->state != ST_CLOSED_ABORTED) {
            struct nano_stream_state* tcb = &stream->tcb;
            uint8_t i;
            nabto_stamp_t now = nabtoGetStamp();
            for (i = tcb->xmitFirst; i < tcb->xmitFirst+tcb->xmitCount; i++) {
                uint8_t ix = i % stream->cfg.xmitWinSize;
                nano_x_buffer* xbuf = &tcb->xmit[ix];
                if (xbuf->xstate == B_SENT) {
                    nabto_update_min_stamp(current_min_stamp, &xbuf->sentStampTimeout);
                }
            }
            nabto_update_min_stamp(current_min_stamp, &tcb->timeoutStamp);

            if (stream->applicationEvents.dataReady || 
                stream->applicationEvents.dataWritten ||
                stream->applicationEvents.readClosed ||
                stream->applicationEvents.writeClosed ||
                stream->applicationEvents.closed)
            {
                nabto_update_min_stamp(current_min_stamp, &now);
            }
        }
    }
}
#endif





bool unabto_stream_is_readable(struct nabto_stream_s* stream) {
    const uint8_t* buf;
    unabto_stream_hint hint;
    unabto_stream_read(stream, &buf, &hint);
    if (hint == UNABTO_STREAM_HINT_OK) {
        return true;
    }

    return false;
}

size_t unabto_stream_read(struct nabto_stream_s* stream, const uint8_t** buf, unabto_stream_hint* hint)
{
    uint8_t ix = stream->tcb.recvNext % stream->cfg.recvWinSize;
    nano_r_buffer* rbuf = &stream->tcb.recv[ix];
    uint16_t avail = rbuf->size - rbuf->used;
    *hint = UNABTO_STREAM_HINT_OK;

    if (avail > 0 && rbuf->seq == stream->tcb.recvNext) {
        NABTO_LOG_TRACE(("Retrieving data from slot=%i size=%i", ix, (uint16_t)avail));
        *buf = (const uint8_t*) rbuf->buf + rbuf->used;
        NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_TRACE, ("Read from stream"), *buf, avail);
        *hint = UNABTO_STREAM_HINT_OK;
        return avail;
    }
     
    if (stream->state > ST_FIN_WAIT_2) {
        // there's no data and there will be no more data in the future since we have received an FIN from the other end.
        *hint = UNABTO_STREAM_HINT_READ_OR_WRITE_ERROR;
    }
    return 0;
}

bool unabto_stream_ack(struct nabto_stream_s* stream, const uint8_t* buf, size_t used, unabto_stream_hint* hint)
{
    uint8_t ix = stream->tcb.recvNext % stream->cfg.recvWinSize;
    nano_r_buffer* rbuf = &stream->tcb.recv[ix];
    uint16_t avail = rbuf->size - rbuf->used;
    
    *hint = UNABTO_STREAM_HINT_OK;
    
    NABTO_LOG_TRACE(("Acking data from slot=%i size=%i avail=%i", ix, used, avail));
    NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_TRACE, ("Ack on stream"), buf, used);
    if (used > avail) {
        NABTO_LOG_TRACE(("used > avail, acking nothing"));
        *hint = UNABTO_STREAM_HINT_ACKNOWLEDGING_TOO_MUCH_DATA;
        return false;
    } else if (used < avail) {
        rbuf->used += (uint16_t)used;
    } else {
        rbuf->size = 0; // Maintain invariant the unused slots have size 0
        ++stream->tcb.recvNext;  /* rolling the receive window because the slot has become idle */
        NABTO_LOG_TRACE(("slot %i is now empty, next slot %i, tcb->recvNext %u", ix, stream->tcb.recvNext % stream->cfg.recvWinSize, stream->tcb.recvNext));
        nano_stream_check_xmit(stream, false);
    }
    return true;
}

/******************************************************************************/

bool unabto_stream_is_writeable(struct nabto_stream_s* stream) {
    switch(stream->state) {
        case ST_ESTABLISHED:
        case ST_CLOSE_WAIT:
            return true;
        default:
            return false;
    }
}

size_t unabto_stream_write(struct nabto_stream_s* stream, const uint8_t* buf, size_t size, unabto_stream_hint* hint)
{
    size_t queued = 0;
    NABTO_LOG_TRACE(("write size=%" PRIsize, size));

    if (!unabto_stream_is_writeable(stream)) {
        *hint = UNABTO_STREAM_HINT_READ_OR_WRITE_ERROR;
        NABTO_LOG_TRACE(("Stream is not writeable"));
        return 0;
    }
    
    NABTO_LOG_TRACE(("Writing to stream, size %i, xmitCount %i, xmitWinSize %i", size, stream->tcb.xmitCount, stream->cfg.xmitWinSize));

    while (size && stream->tcb.xmitCount < stream->cfg.xmitWinSize) {
        uint16_t sz;
        uint8_t ix = stream->tcb.xmitSeq % stream->cfg.xmitWinSize;
        nano_x_buffer* xbuf = &stream->tcb.xmit[ix];

        sz = size > stream->cfg.xmitPacketSize ? stream->cfg.xmitPacketSize : (uint16_t)size;
        NABTO_LOG_TRACE(("-------- nabto_stream_write %i bytes, seq=%i into ix=%i", sz, stream->tcb.xmitSeq, ix));
        
        memcpy(xbuf->buf, (const void*) &buf[queued], sz);
        xbuf->size = sz;
        xbuf->seq = stream->tcb.xmitSeq;
        xbuf->xstate = B_DATA;
        queued += sz;
        size -= sz;
        ++stream->tcb.xmitSeq;   // done here or after sending?
        ++stream->tcb.xmitCount;
    }
    if (queued) {
        NABTO_LOG_TRACE(("-------- nabto_stream_write calls nabto_stream_tcb_check_xmit"));
        nano_stream_check_new_data_xmit(stream);
    }

    stream->stats.sentBytes += queued;
    *hint = UNABTO_STREAM_HINT_OK;
    return queued;
}

size_t unabto_stream_can_write(struct nabto_stream_s* stream, unabto_stream_hint* hint)
{
    *hint = UNABTO_STREAM_HINT_OK;
    if (!unabto_stream_is_writeable(stream)) {
        *hint = UNABTO_STREAM_HINT_READ_OR_WRITE_ERROR;
        return 0;
    }
    return  (stream->cfg.xmitWinSize - stream->tcb.xmitCount) * stream->cfg.xmitPacketSize;
}

/******************************************************************************/

bool unabto_stream_is_closed(struct nabto_stream_s* stream) {
    return stream->state == ST_CLOSED || stream->state == ST_CLOSED_ABORTED;
}

bool unabto_stream_close(struct nabto_stream_s* stream)
{
    if (stream->state == ST_CLOSE_WAIT || stream->state == ST_ESTABLISHED) {
        if (stream->state == ST_CLOSE_WAIT) {
            nano_stream_state_transition(stream, ST_LAST_ACK);
        }

        if (stream->state == ST_ESTABLISHED) {
            nano_stream_state_transition(stream, ST_FIN_WAIT_1);
        }
    } else {
        if (stream->state < ST_ESTABLISHED) {
            // break opening protocol
            nano_stream_state_transition(stream, ST_CLOSED);
        }
    }

    return unabto_stream_is_closed(stream);
}

void unabto_stream_release(struct nabto_stream_s* stream)
{
    if (!unabto_stream_is_closed(stream)) {
        NABTO_LOG_TRACE(("Releasing stream in state %d - %i", stream->state, stream->state));
    }
    nano_stream_state_transition(stream, ST_IDLE);
    nano_stream_reset(stream);

}


int unabto_stream_get_stats(struct nabto_stream_s* stream, unabto_stream_stats* stats) {
    *stats = stream->stats;
    return 0;
}

nabto_connect* unabto_stream_connection(struct nabto_stream_s* stream)
{
    return stream->connection;
}

/***
 * WINDOW
 */
/// Difference (modulo 2**16) between sequence numbers.
/// @param s1  the largest sequence number
/// @param s2  the smallest sequence number
/// @return    the ddifference
#define SEQ_DIFF(s1, s2) ((uint16_t)(s1 - s2))

void nano_stream_state_transition(struct nabto_stream_s* stream, uint8_t new_state) {
    if (stream->state == new_state) return;
    NABTO_LOG_TRACE(("State transition from %i to %i", stream->state, new_state));
    stream->state = new_state;
    switch(new_state) {
        case ST_IDLE:
            break;
        case ST_SYN_RCVD:
            nabtoSetFutureStamp(&stream->tcb.timeoutStamp, 0);
            break;
        case ST_ESTABLISHED:
            unabto_stream_accept(stream);
            break;

        // We move to the following states when user close is received.
        case ST_FIN_WAIT_1:
            stream->tcb.retransCount = 0;
            nabtoSetFutureStamp(&stream->tcb.timeoutStamp, 0);
            stream->applicationEvents.writeClosed = true;
            break;

        case ST_FIN_WAIT_2:
            break;

        case ST_CLOSING:
            stream->applicationEvents.readClosed = true;
            break;

        case ST_TIME_WAIT:
            stream->tcb.retransCount = 0;
            stream->applicationEvents.readClosed = true;
            break;

        case ST_CLOSE_WAIT:
            stream->applicationEvents.readClosed = true;
            break;

        case ST_LAST_ACK:
            stream->tcb.retransCount = 0;
            nabtoSetFutureStamp(&stream->tcb.timeoutStamp, 0);
            stream->applicationEvents.writeClosed = true;
            break;

        case ST_CLOSED:
            stream->applicationEvents.closed = true;
            break;
        case ST_CLOSED_ABORTED:
            /**
             * Since we have aborted the stream in a unknown way so we
             * kills the connection. Such that the other end of the
             * stream knows that something not expected happened on
             * this stream.
             */
            //nabto_release_connection_req(stream->connection);
            stream->applicationEvents.closed = true;
        default:
            break;
    }
    
    
}

/******************************************************************************/

/**
 * Build and send a packet.
 * @param stream       the stream
 * @param type         the stream window type
 * @param seq          the xmit sequence number
 * @param winInfoData  the window payload additional data (in network order)
 * @param winInfoSize  size of the window payload additional data
 * @param data         the data to be encrypted
 * @param size         size of the data to be encrypted
 * @return             true if packet is sent (and then stream->ackSent is updated)
 */
static void build_and_send_packet(struct nabto_stream_s* stream, uint8_t type, uint32_t seq, const uint8_t* winInfoData, size_t winInfoSize, uint8_t* data, uint16_t size)
{
    enum { l_win = NP_PAYLOAD_WINDOW_SYN_BYTELENGTH - NP_PAYLOAD_WINDOW_BYTELENGTH };
    uint8_t*       ptr;
    nabto_connect* con   = stream->connection;

    if(con == NULL) {
        return;
    }

    ptr = insert_data_header(nabtoCommunicationBuffer, con->spnsi, con->nsico, stream->streamTag);
    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_WINDOW, 0, l_win + winInfoSize + (type == ACK ? 2 : 0));
    WRITE_U8 (ptr, type);                      ptr += 1;
    WRITE_U8 (ptr, NP_STREAM_VERSION);         ptr += 1;
    WRITE_U16(ptr, stream->idCP);              ptr += 2;
    WRITE_U16(ptr, stream->idSP);              ptr += 2;
    WRITE_U32(ptr, seq);                       ptr += 4;
    WRITE_U32(ptr, stream->tcb.recvNext);                  ptr += 4;
    if (type == ACK) {
        WRITE_U16(ptr, stream->cfg.recvWinSize);           ptr += 2;
    }

    if (winInfoSize) {
        memcpy(ptr, (const void*) winInfoData, winInfoSize); ptr += winInfoSize;
    }
    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_CRYPTO, 0, 0);

    send_and_encrypt_packet_con(con, data, size, ptr);
    stream->tcb.ackSent = stream->tcb.recvNext;
    stream->stats.sentPackets++;
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
static void send_data_packet(struct nabto_stream_s* stream, uint32_t seq, uint8_t* data, uint16_t size)
{
    build_and_send_packet(stream, ACK, seq, 0, 0, data, size);
}

/******************************************************************************/

/**
 * Send the response to a SYN message.
 * @param stream  the stream
 * @return true if syn-ack send
 */
static void send_SYN_ACK(struct nabto_stream_s* stream)
{
    uint16_t options = 0;
    uint8_t tmp[14];

    WRITE_U16(tmp +  0, options);
    WRITE_U16(tmp +  2, stream->cfg.recvPacketSize);
    WRITE_U16(tmp +  4, stream->cfg.recvWinSize);
    WRITE_U16(tmp +  6, stream->cfg.xmitPacketSize);
    WRITE_U16(tmp +  8, stream->cfg.xmitWinSize);
    WRITE_U16(tmp + 10, stream->cfg.maxRetrans);
    WRITE_U16(tmp + 12, stream->cfg.timeoutMsec);

    build_and_send_packet(stream, SYN|ACK, stream->tcb.xmitSeq, tmp, sizeof(tmp), 0, 0);

    NABTO_LOG_DEBUG(("%" PRIu16 " <-- [%" PRIu32 ",%" PRIu32 "] SYN|ACK (RETRANS: %" PRIu16 ")",
                     stream->streamTag, stream->tcb.xmitSeq, stream->tcb.recvNext, stream->tcb.retransCount));
    // Not using tcb->cfg.timeoutMsec here since we are negotiating
    // it with the other end.
    nabtoSetFutureStamp(&stream->tcb.timeoutStamp, NABTO_STREAM_TIMEOUT);
}

/******************************************************************************/

/**
 * Send a FIN ACK message.
 * @param stream  the stream
 * @return        true iff sent
 */
static void send_FIN_ACK(struct nabto_stream_s* stream)
{
    /**
     * We send a FIN | ACK after all outstanding data has been acked.
     * This means xmitFirst points to the window location we should send next.
     */
    


    if (!stream->tcb.finSent) {
        stream->tcb.xmit[stream->tcb.xmitFirst].seq = stream->tcb.xmitSeq;
        stream->tcb.xmitSeq++;
        stream->tcb.xmitCount++;
        stream->tcb.finSent = true;
    }

    {
        uint32_t fin_seq = stream->tcb.xmit[stream->tcb.xmitFirst].seq;
        
        // The fin has the sequence number xmitSeq-1
        build_and_send_packet(stream, FIN|ACK, fin_seq, 0, 0, 0, 0);
        NABTO_LOG_DEBUG(("%i <-- [%i,%i] FIN|ACK, RETRANS: %i", stream->streamTag, fin_seq, stream->tcb.recvNext, stream->tcb.retransCount));

    }
}

/******************************************************************************/

void nano_stream_check_retransmit_data(struct nabto_stream_s* stream) {
    uint8_t nSlots    = stream->cfg.xmitWinSize;
    uint8_t ix        = stream->tcb.xmitFirst % nSlots;

    nano_x_buffer* xbuf = &stream->tcb.xmit[ix];
    
    // Check if we have any sent but unacked data.
    if (xbuf->xstate != B_SENT) {
        return;
    }
    

    // Check if the oldest unacked but sent data is expired.
    if (nabtoIsStampPassed(&xbuf->expireStamp)) {
        nano_stream_state_transition(stream, ST_CLOSED_ABORTED);
        return;
    }
    
    if (stream->connection == NULL) {
        return;
    }

    if (nabto_connection_is_reliable(stream->connection)) {
        // We should not retransmit data on a reliable connection.
        return;
    }
   
    for ( ; nSlots > 0; --nSlots) {
        bool sendNow = false;
        xbuf = &stream->tcb.xmit[ix];

        // Check if we are above the most recent sent buffer
        // if so there can be no retransmits since the data has not been sent.

        // Todo: make it more general:
        // In a special case xmitFirst can be the same as xmitLast but the 
        // window is full of sent data which has not been acknowledged yet.
        // so check that ix != tcb->xmitFirst
        if (ix == stream->tcb.xmitLastSent && ix != stream->tcb.xmitFirst) {
            break;
        }
        
        if (xbuf->xstate != B_SENT) {
            continue;
        }

        if (nabtoIsStampPassed(&xbuf->sentStampTimeout)) {
            if (nabtoStampLess(&xbuf->expireStamp, &xbuf->sentStamp)) {
                nano_stream_state_transition(stream, ST_CLOSED_ABORTED);
                break;
            } else {
                sendNow = true;
            }
        }
        
        if (sendNow) {
            send_data_packet(stream, xbuf->seq, xbuf->buf, xbuf->size);
            stream->stats.sentResentPackets++;
            
            xbuf->sentStamp = nabtoGetStamp();
            nabtoSetFutureStamp(&xbuf->sentStampTimeout, stream->tcb.timeoutData);
        }
        ix = (ix + 1) % stream->cfg.xmitWinSize;
    }
}


void nano_stream_check_new_data_xmit(struct nabto_stream_s* stream) {
    // Check for sending data which has not been sent the first time yet.
    // This should be called when new data is queued, data is acked or
    // other events which could change the amount of available data
    // or the flight size.
    
    // When a packet is sent and changes status from B_DATA to B_SENT
    // remember to increment xmitLastSent.

    uint8_t nSlots    = stream->cfg.xmitWinSize;
    uint8_t ix    =  stream->tcb.xmitLastSent % nSlots;


    for ( ; nSlots > 0; --nSlots) {
        
        nano_x_buffer* xbuf = & stream->tcb.xmit[ix];
        if (xbuf->xstate != B_DATA) {
            // No more fresh data to send.
            break;
        }

        send_data_packet(stream, xbuf->seq, xbuf->buf, xbuf->size);

        // data was sent increment xmitLastSent etc.
        xbuf->xstate = B_SENT;
        stream->tcb.xmitLastSent = ( stream->tcb.xmitLastSent+1) % stream->cfg.xmitWinSize;
        nabtoSetFutureStamp(&xbuf->expireStamp, stream->cfg.maxRetrans*stream->cfg.timeoutMsec);
        xbuf->sentStamp = nabtoGetStamp();
        nabtoSetFutureStamp(&xbuf->sentStampTimeout,  stream->tcb.timeoutData);

        ix = (ix + 1) % stream->cfg.xmitWinSize;
    }
}

void nano_stream_check_xmit(struct nabto_stream_s* stream, bool isTimedEvent)
{
    switch (stream->state) {
        case ST_SYN_RCVD:
            if (nabtoIsStampPassed(&stream->tcb.timeoutStamp)) {
                if (stream->tcb.retransCount > stream->cfg.maxRetrans) {
                    NABTO_LOG_INFO(("retransCount > tcb->cfg.maxRetrans, releasing the stream"));
                    // The stream is first accepted and hence released by an application after
                    // the state has passed ST_SYN_RCVD.
                    // Instead of switching to ST_CLOSED just release the resource.
                    stream->state = ST_IDLE;
                } else {
                    send_SYN_ACK(stream);
                    stream->tcb.retransCount++;
                }
            }
            break;

        case ST_ESTABLISHED:
        case ST_FIN_WAIT_1:
        case ST_CLOSING:
        case ST_CLOSE_WAIT:
        case ST_LAST_ACK:
            // Send new data (Cheap operation.)
            nano_stream_check_new_data_xmit(stream);            
            /* Check for sending retransmission */
            // retransmissions should only occur on timed events

            if (isTimedEvent) {
                nano_stream_check_retransmit_data(stream);
            }
            break;
        case ST_TIME_WAIT:
            if (nabtoIsStampPassed(&stream->tcb.timeoutStamp)) {
                stream->tcb.retransCount++;
                if (stream->tcb.retransCount > stream->cfg.maxRetrans) {
                    nano_stream_state_transition(stream, ST_CLOSED);
                }
            }
            break;
                
        default:
            break;
    }

    /* Check for rensend of FIN */
    if (stream->tcb.xmitCount == 0) {
        switch (stream->state) {
            case ST_FIN_WAIT_1:
            case ST_CLOSING:
            case ST_LAST_ACK:
                if (nabtoIsStampPassed(&stream->tcb.timeoutStamp)) {
                    if (stream->tcb.retransCount > stream->cfg.maxRetrans) {
                        NABTO_LOG_INFO(("retransCount > tcb->cfg.maxRetrans, releasing the stream"));
                        nano_stream_state_transition(stream, ST_CLOSED);
                    } else {
                        send_FIN_ACK(stream);
                        nabtoSetFutureStamp(&stream->tcb.timeoutStamp, stream->cfg.timeoutMsec);
                        stream->tcb.retransCount++;
                    }
                }
                break;
            default: // Compiler warning removal.
                break;
        }
    }

    /* Check for sending an ACK without data */
    if (stream->tcb.ackSent != stream->tcb.recvNext) {
        send_data_packet(stream, stream->tcb.xmitSeq, 0, 0);
    }

    // Set timestamps in the future if they are irrelevant
    // s.t. an event based approach is possible.
    if (nabtoIsStampPassed(&stream->tcb.timeoutStamp)) {
        nabtoSetFutureStamp(&stream->tcb.timeoutStamp, stream->cfg.timeoutMsec);
    }
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
static uint8_t accept_xmit_seq(struct nabto_stream_s* stream, struct nabto_win_info* win, uint16_t dlen)
{
    NABTO_LOG_TRACE(("Accept_xmit_seq, seq: %i", win->seq));
    /* NOTE: The code below only works because we are using unsigned
     * arithmetics. The code will break for signed integers.
     */
    /* EQUIV: if (tcb->recvNext <= seq && seq < tcb->recvNext + limit) */


    if (win->seq - stream->tcb.recvNext < stream->cfg.recvWinSize) {
        return SEQ_EXPECTED;
    }

    // This case can happen if we have not yet acknowledged incoming
    // data and the incoming data window is full.
    if (dlen == 0) {
        /* If the packet has no data then return SEQ_EXPECTED
         */
        return SEQ_EXPECTED;
    }

    /* EQUIV: if (tcb->recvNext - limit <= seq && seq <= tcb->recvNext) */
    if ((uint8_t)stream->tcb.recvNext - (uint8_t)win->seq <= stream->cfg.recvWinSize) {
        NABTO_LOG_DEBUG(("Data retransmission, seq: %i, expect: [%i..%i]", win->seq, stream->tcb.recvNext, stream->tcb.recvNext + stream->cfg.recvWinSize - 1));
        return SEQ_RETRANS;
    }
    NABTO_LOG_DEBUG(("Bad seq: %" PRIu32 ", expect: [%" PRIu32 "..%" PRIu32 "]" , win->seq, stream->tcb.recvNext, stream->tcb.recvNext + stream->cfg.recvWinSize));
    return SEQ_INVALID;
}

void nano_stream_handle_ack(struct nabto_stream_s* stream, struct nabto_win_info* win) {

    uint32_t ack_start;
    uint8_t ackCount;
    
    if (stream->tcb.xmitCount == 0) {
        return;
    }

    ack_start = stream->tcb.xmit[stream->tcb.xmitFirst].seq; // first seq awaiting ack
    
    // Number of consequtive sequence numbers we should acknowledge.
    ackCount = win->ack - ack_start;

    if (ack_start > win->ack) return;

    NABTO_LOG_TRACE(("%" PRIu16 " acking: [%" PRIu32 "..%" PRIu32 "]", stream->streamTag, ack_start, win->ack));
    
    for (; ackCount > 0; ackCount--) {
        // Mark segment as acked.
        if (stream->tcb.xmit[stream->tcb.xmitFirst].xstate != B_IDLE) {
            NABTO_LOG_TRACE(("setting buffer %i to IDLE", stream->tcb.xmitFirst));
            stream->tcb.xmit[stream->tcb.xmitFirst].xstate = B_IDLE;
        }
        
        stream->tcb.xmitFirst = (stream->tcb.xmitFirst+1) % stream->cfg.xmitWinSize;
        --stream->tcb.xmitCount;
        stream->applicationEvents.dataWritten = true;
    }
}

static void nano_stream_handle_data(struct nabto_stream_s* stream,
                                    struct nabto_win_info* win,
                                    uint8_t *              start,
                                    int                    dlen) {
    switch (accept_xmit_seq(stream, win, dlen)) {
        case SEQ_RETRANS:
            /* Be sure to send/resend an ACK, earlier ACK may be lost */
            stream->tcb.ackSent--;
            
            nano_stream_handle_ack(stream, win);

            stream->stats.receivedResentPackets++;

            break;
        
        case SEQ_EXPECTED:
            /* use ack's from received packet to roll transmit window */
            nano_stream_handle_ack(stream, win);

            /* if possible insert data in receive window */
            if (dlen > 0) {
                uint8_t ix = (uint8_t)(win->seq % stream->cfg.recvWinSize);
                nano_r_buffer* rbuf = &stream->tcb.recv[ix];
                if (rbuf->size == 0) {
                    NABTO_LOG_DEBUG(("%" PRIu16 "     Data=%i bytes, seq=%i inserted into slot=%i",stream->streamTag, dlen, win->seq, ix));
                    //NABTO_LOG_BUFFER(("data"), start, dlen);
                    stream->stats.receivedBytes += dlen;
                    memcpy(rbuf->buf, (const void*) start, dlen);
                    rbuf->size = dlen;
                    rbuf->used = 0;
                    rbuf->seq = win->seq;
                    stream->applicationEvents.dataReady = true;
                }
            }
            break;

        case SEQ_INVALID:
            break;
    }
}



bool nano_stream_is_ack_on_fin(struct nano_stream_state* tcb, struct nabto_win_info* win) {
    return (tcb->finSent && win->ack == tcb->xmitSeq);
}

bool nano_stream_handle_fin(struct nano_stream_state* tcb, struct nabto_win_info* win) {
    if (win->seq == tcb->recvNext) {
        tcb->recvNext++;
        return true;
    } else {
        NABTO_LOG_ERROR(("Can't accept FIN since we have unacked data."));
    }
    return false;
}


int unabto_stream_index(unabto_stream* stream) {
    return stream - nano_stream__;
}

#endif /* NABTO_ENABLE_STREAM && NABTO_ENABLE_NANO_STREAM */
