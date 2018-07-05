#include "unabto_memory.h"
#include "unabto_logging.h"
#include "unabto_stream_window.h"
#include "unabto_stream_types.h"

#if NABTO_ENABLE_DYNAMIC_MEMORY

#if NABTO_ENABLE_CONNECTIONS

NABTO_THREAD_LOCAL_STORAGE nabto_connect* connections = 0;

#endif

#if NABTO_ENABLE_STREAM && NABTO_ENABLE_MICRO_STREAM
NABTO_THREAD_LOCAL_STORAGE unabto_stream* stream__ = 0;
NABTO_THREAD_LOCAL_STORAGE uint8_t* r_buffer_data = 0;
NABTO_THREAD_LOCAL_STORAGE bool* send_segment_pool = 0;
NABTO_THREAD_LOCAL_STORAGE uint8_t* x_buffer_data = 0;
NABTO_THREAD_LOCAL_STORAGE r_buffer* r_buffers = 0;
NABTO_THREAD_LOCAL_STORAGE x_buffer* x_buffers = 0;

#endif

bool unabto_allocate_memory(nabto_main_setup* nms)
{
#if NABTO_ENABLE_CONNECTIONS
    connections = (nabto_connect*)malloc(sizeof(struct nabto_connect_s) * NABTO_MEMORY_CONNECTIONS_SIZE);
    if (connections == NULL) {
        NABTO_LOG_FATAL(("Cannot initialize connections. Number of connections: %" PRIu16, nms->connectionsSize));
        return false;
    }
#endif

#if NABTO_ENABLE_STREAM && NABTO_ENABLE_MICRO_STREAM
    stream__ = (unabto_stream*)malloc(sizeof(struct nabto_stream_s) * NABTO_MEMORY_STREAM_MAX_STREAMS);
    if (stream__ == NULL) {
        NABTO_LOG_FATAL(("Cannot initialize stream structure. Number of streams: %" PRIu16, NABTO_MEMORY_STREAM_MAX_STREAMS));
        return false;
    }

    {
        size_t receiveBuffersSize = (size_t)NABTO_MEMORY_STREAM_MAX_STREAMS * NABTO_MEMORY_STREAM_RECEIVE_SEGMENT_SIZE * NABTO_MEMORY_STREAM_RECEIVE_WINDOW_SIZE;
        r_buffer_data = (uint8_t*)malloc(receiveBuffersSize);
        if (r_buffer_data == NULL) {
            NABTO_LOG_FATAL(("Cannot initialize stream receive buffers. Receive buffers total size: %" PRIsize, receiveBuffersSize));
            return false;
        }
    }


    {
        size_t sendSegmentPoolSize = sizeof(bool) * NABTO_MEMORY_STREAM_SEND_SEGMENT_POOL_SIZE;
        send_segment_pool = (bool*)malloc(sendSegmentPoolSize);
        if (send_segment_pool == NULL) {
            NABTO_LOG_FATAL(("Cannot initialize send segment pool. Send segment pool size %" PRIsize, sendSegmentPoolSize));
            return false;
        }
        memset(send_segment_pool, 0, sendSegmentPoolSize);
    }

    {
        size_t sendBuffersSize = (size_t)NABTO_MEMORY_STREAM_SEND_SEGMENT_POOL_SIZE * NABTO_MEMORY_STREAM_SEND_SEGMENT_SIZE;
        x_buffer_data = (uint8_t*)malloc(sendBuffersSize);
        if (x_buffer_data == NULL) {
            NABTO_LOG_FATAL(("Cannot initialize stream send buffers. Send buffers total size: %" PRIsize, sendBuffersSize));
            return false;
        }
    }

    {
        size_t nReceiveBuffers = (size_t)NABTO_MEMORY_STREAM_MAX_STREAMS * NABTO_MEMORY_STREAM_RECEIVE_WINDOW_SIZE;
        r_buffers = (r_buffer*)malloc(sizeof(r_buffer) * nReceiveBuffers);
        if (r_buffers == NULL) {
            NABTO_LOG_FATAL(("Cannot initialize receive buffers structures"));
            return false;
        }
    }

    {
        size_t nXmitBuffers = (size_t)NABTO_MEMORY_STREAM_MAX_STREAMS * NABTO_MEMORY_STREAM_SEND_WINDOW_SIZE;
        x_buffers = (x_buffer*)malloc(sizeof(x_buffer) * nXmitBuffers);
        if (x_buffers == NULL) {
            NABTO_LOG_FATAL(("Cannot initialize xmit buffers structures"));
            return false;
        }
    }
#endif
    return true;
}

void unabto_free_memory()
{
    free(connections); connections = 0;
    free(stream__); stream__ = 0;
    free(r_buffer_data); r_buffer_data = 0;
    free(send_segment_pool); send_segment_pool = 0;
    free(x_buffer_data); x_buffer_data = 0;
    free(r_buffers); r_buffers = 0;
    free(x_buffers); x_buffers = 0;
}

#else

#endif
