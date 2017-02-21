/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_MEMORY_H_
#define _UNABTO_MEMORY_H_

#include <unabto/unabto_env_base.h>
#include <unabto/unabto_main_contexts.h>
#include <unabto/unabto_connection.h>
#include <unabto/unabto_stream_window.h>

#ifdef __cplusplus
extern "C" {
#endif


#if NABTO_ENABLE_DYNAMIC_MEMORY

/**************************/
/* DYNAMIC MEMORY SECTION */
/**************************/

#if NABTO_ENABLE_CONNECTIONS
extern NABTO_THREAD_LOCAL_STORAGE nabto_connect* connections;

#define NABTO_MEMORY_CONNECTIONS_SIZE nmc.nabtoMainSetup.connectionsSize

#endif

#if NABTO_ENABLE_STREAM && NABTO_ENABLE_MICRO_STREAM
extern NABTO_THREAD_LOCAL_STORAGE unabto_stream* stream__;
extern NABTO_THREAD_LOCAL_STORAGE uint8_t* r_buffer_data;
extern NABTO_THREAD_LOCAL_STORAGE uint8_t* x_buffer_data;
extern NABTO_THREAD_LOCAL_STORAGE x_buffer* x_buffers;
extern NABTO_THREAD_LOCAL_STORAGE r_buffer* r_buffers;
#endif

#define NABTO_MEMORY_STREAM_MAX_STREAMS nmc.nabtoMainSetup.streamMaxStreams
#define NABTO_MEMORY_STREAM_RECEIVE_SEGMENT_SIZE NABTO_STREAM_RECEIVE_SEGMENT_SIZE
#define NABTO_MEMORY_STREAM_RECEIVE_WINDOW_SIZE nmc.nabtoMainSetup.streamSendWindowSize
#define NABTO_MEMORY_STREAM_SEND_SEGMENT_SIZE NABTO_STREAM_SEND_SEGMENT_SIZE
#define NABTO_MEMORY_STREAM_SEND_WINDOW_SIZE nmc.nabtoMainSetup.streamSendWindowSize


bool unabto_allocate_memory(nabto_main_setup* nms);

void unabto_free_memory();

#else

/*************************/
/* STATIC MEMORY SECTION */
/*************************/

#if NABTO_ENABLE_CONNECTIONS
#define NABTO_MEMORY_CONNECTIONS_SIZE NABTO_CONNECTIONS_SIZE
extern NABTO_THREAD_LOCAL_STORAGE nabto_connect connections[NABTO_MEMORY_CONNECTIONS_SIZE];
#endif

#if NABTO_ENABLE_STREAM && ( NABTO_ENABLE_MICRO_STREAM )

#define NABTO_MEMORY_STREAM_MAX_STREAMS NABTO_STREAM_MAX_STREAMS
#define NABTO_MEMORY_STREAM_RECEIVE_SEGMENT_SIZE NABTO_STREAM_RECEIVE_SEGMENT_SIZE
#define NABTO_MEMORY_STREAM_RECEIVE_WINDOW_SIZE NABTO_STREAM_RECEIVE_WINDOW_SIZE
#define NABTO_MEMORY_STREAM_SEND_SEGMENT_SIZE NABTO_STREAM_SEND_SEGMENT_SIZE
#define NABTO_MEMORY_STREAM_SEND_WINDOW_SIZE NABTO_STREAM_SEND_WINDOW_SIZE

#endif



#endif // NABTO_MEMORY_MODEL_DYNAMIC

extern NABTO_THREAD_LOCAL_STORAGE uint8_t nabtoCommunicationBuffer[NABTO_COMMUNICATION_BUFFER_SIZE];

extern NABTO_THREAD_LOCAL_STORAGE uint16_t nabtoCommunicationBufferSize;

extern nabto_context* unabto_context;

extern NABTO_THREAD_LOCAL_STORAGE nabto_main_context nmc;

#if NABTO_ENABLE_CONNECTIONS
extern NABTO_THREAD_LOCAL_STORAGE nabto_stamp_t connection_timeout_cache_stamp;
extern NABTO_THREAD_LOCAL_STORAGE bool connection_timeout_cache_cached;
#endif




#ifdef __cplusplus
} //extern "C"
#endif

#endif
