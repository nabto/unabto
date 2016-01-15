#include <unabto/unabto_env_base.h>

#if NABTO_ENABLE_STREAM && NABTO_ENABLE_MICRO_STREAM

#include <unabto/unabto_stream_environment.h>
#include <unabto/unabto_memory.h>
#include <unabto/unabto_util.h>
#include <unabto/unabto_packet.h>

#if NABTO_ENABLE_DYNAMIC_MEMORY

#else
NABTO_THREAD_LOCAL_STORAGE unabto_stream stream__[NABTO_MEMORY_STREAM_MAX_STREAMS];  /**< a pool of streams */
NABTO_THREAD_LOCAL_STORAGE uint8_t r_buffer_data[NABTO_MEMORY_STREAM_MAX_STREAMS * NABTO_MEMORY_STREAM_RECEIVE_SEGMENT_SIZE * NABTO_MEMORY_STREAM_RECEIVE_WINDOW_SIZE];
NABTO_THREAD_LOCAL_STORAGE uint8_t x_buffer_data[NABTO_MEMORY_STREAM_MAX_STREAMS * NABTO_MEMORY_STREAM_SEND_SEGMENT_SIZE * NABTO_MEMORY_STREAM_SEND_WINDOW_SIZE];

NABTO_THREAD_LOCAL_STORAGE x_buffer x_buffers[NABTO_MEMORY_STREAM_MAX_STREAMS * NABTO_MEMORY_STREAM_SEND_WINDOW_SIZE];
NABTO_THREAD_LOCAL_STORAGE r_buffer r_buffers[NABTO_MEMORY_STREAM_MAX_STREAMS * NABTO_MEMORY_STREAM_RECEIVE_WINDOW_SIZE];
#endif

bool build_and_send_packet(struct nabto_stream_s* stream, uint8_t type, uint32_t seq, const uint8_t* winInfoData, size_t winInfoSize, uint8_t* data, uint16_t size, struct nabto_stream_sack_data* sackData)
{
    enum { l_win = NP_PAYLOAD_WINDOW_SYN_BYTELENGTH - NP_PAYLOAD_WINDOW_BYTELENGTH };
    uint8_t*       ptr;
    nabto_connect* con           = stream->connection;
    uint8_t*       buf           = nabtoCommunicationBuffer;
    struct nabto_stream_tcb* tcb = &stream->u.tcb;
    uint32_t       ackToSend     = unabto_stream_ack_number_to_send(tcb);
    uint16_t       recvWinSize   = unabto_stream_advertised_window_size(tcb);

    if (con == NULL) {
        return false;
    }
    
    nabtoSetFutureStamp(&tcb->ackStamp, 2*tcb->cfg.timeoutMsec);

    ptr = insert_data_header(buf, con->spnsi, con->nsico, stream->streamTag);
    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_WINDOW, 0, l_win + winInfoSize + (type == NP_PAYLOAD_WINDOW_FLAG_ACK ? 2 : 0));
    WRITE_FORWARD_U8 (ptr, type);
    WRITE_FORWARD_U8 (ptr, NP_STREAM_VERSION);
    WRITE_FORWARD_U16(ptr, stream->idCP);
    WRITE_FORWARD_U16(ptr, stream->idSP);
    WRITE_FORWARD_U32(ptr, seq);
    WRITE_FORWARD_U32(ptr, ackToSend);
    if (type == NP_PAYLOAD_WINDOW_FLAG_ACK) {
        WRITE_FORWARD_U16(ptr, recvWinSize);
    }

    if (winInfoSize) {
        memcpy(ptr, (const void*) winInfoData, winInfoSize); ptr += winInfoSize;
    }
    
    if (sackData && sackData->nPairs > 0) {
        uint8_t i;
        ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_SACK, 0, 8 * sackData->nPairs);
        for (i = 0; i < sackData->nPairs; i++) {
            WRITE_FORWARD_U32(ptr, sackData->pairs[i].start);
            WRITE_FORWARD_U32(ptr, sackData->pairs[i].end);
        }
    }


    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_CRYPTO, 0, 0);

    if (send_and_encrypt_packet_con(con, data, size, ptr)) {
        tcb->ackSent = ackToSend;
        tcb->lastSentAdvertisedWindow = recvWinSize;
        stream->stats.sentPackets++;
        return true;
    } else {
        return false;
    }
}

void unabto_stream_init_buffers(struct nabto_stream_s* stream)
{
    int i;
    struct nabto_stream_tcb* tcb = &stream->u.tcb;

    uint8_t* xmitBase = x_buffer_data + (unabto_stream_index(stream) * NABTO_MEMORY_STREAM_SEND_SEGMENT_SIZE * NABTO_MEMORY_STREAM_SEND_WINDOW_SIZE);
    uint8_t* recvBase = r_buffer_data + (unabto_stream_index(stream) * NABTO_MEMORY_STREAM_RECEIVE_SEGMENT_SIZE * NABTO_MEMORY_STREAM_RECEIVE_WINDOW_SIZE);

    tcb->xmit = &x_buffers[unabto_stream_index(stream) * NABTO_MEMORY_STREAM_SEND_WINDOW_SIZE];
    tcb->recv = &r_buffers[unabto_stream_index(stream) * NABTO_MEMORY_STREAM_RECEIVE_WINDOW_SIZE];

    for (i=0;i< NABTO_MEMORY_STREAM_SEND_WINDOW_SIZE; i++)
    {
        memset(&tcb->xmit[i], 0, sizeof(x_buffer));
        tcb->xmit[i].buf = xmitBase + (i * NABTO_MEMORY_STREAM_SEND_SEGMENT_SIZE);
    }

    for (i=0;i< NABTO_MEMORY_STREAM_RECEIVE_WINDOW_SIZE; i++)
    {
        memset(&tcb->recv[i], 0, sizeof(r_buffer));
        tcb->recv[i].buf = recvBase + (i * NABTO_MEMORY_STREAM_RECEIVE_SEGMENT_SIZE);
    }
}

bool unabto_stream_is_connection_reliable(struct nabto_stream_s* stream)
{
    if (stream->connection == NULL) {
        return false;
    }
    return nabto_connection_is_reliable(stream->connection);
}

#endif
