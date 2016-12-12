/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */

#include <unabto/unabto_env_base.h>
#include <unabto/unabto_logging.h>
#include <unabto/unabto_memory.h>
#include <unabto/unabto_app.h>
#include <unabto/unabto_stream.h>
#include <unabto/unabto_stream_window.h>
#include <unabto/unabto_stream_event.h>

#include "unabto_diag.h"

#include <stdio.h>
#include <string.h>

#ifdef WIN32
#define STR(s) "??"
#else
#define STR_H(s) #s
#define STR(s) STR_H(s)
#endif


#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
// Until we have restructures the include files, we can not include the
// internal types, since they are in unabto_app_adapter.c.
// We define some fake structures here with the same sizes
enum fake_queue_state { dummy = 0 };
struct fake_queue_event {
    /* struct naf_handle_s */
    application_request  applicationRequest;           ///< the request part seen by the application
    nabto_packet_header  header;                       ///< the request packet header
    nabto_connect*       connection;                   ///< the connection
    uint32_t             spnsi;                        ///< the connection consistency id
    unabto_buffer        applicationRequestBuffer;     ///< pointer to application request
    unabto_query_request readBuffer;                   ///< read access to r_buf
    /* END of struct naf_handle_s */
    enum fake_queue_state state;
};
#endif


void unabto_printf_unabto_config(FILE * f, const char *progname)
{
    if (!progname) progname = "";
    fprintf(f, "%s: unabto_config.h\n", progname);
    fprintf(f, "/*=========================================================*/\n");
    fprintf(f, "#define NABTO_ENABLE_REMOTE_CONNECTION            %d\n", NABTO_ENABLE_REMOTE_CONNECTION);
    fprintf(f, "#define NABTO_ENABLE_LOCAL_CONNECTION             %d\n", NABTO_ENABLE_LOCAL_CONNECTION);
    fprintf(f, "#define NABTO_ENABLE_LOCAL_ACCESS_LEGACY_PROTOCOL %d\n", NABTO_ENABLE_LOCAL_ACCESS_LEGACY_PROTOCOL);
    fprintf(f, "/*---------------------------------------------------------*/\n");
    fprintf(f, "#define NABTO_CONNECTIONS_SIZE %d\n", NABTO_CONNECTIONS_SIZE);
    fprintf(f, "#define UNABTO_COMMUNICATION_BUFFER_SIZE %d\n", UNABTO_COMMUNICATION_BUFFER_SIZE);
    fprintf(f, "/*---------------------------------------------------------*/\n");
    fprintf(f, "#define NABTO_APPLICATION_EVENT_MODEL_ASYNC %d\n", NABTO_APPLICATION_EVENT_MODEL_ASYNC);
    fprintf(f, "#define NABTO_APPREQ_QUEUE_SIZE %d\n", NABTO_APPREQ_QUEUE_SIZE);
    fprintf(f, "/*---------------------------------------------------------*/\n");
    fprintf(f, "#define NABTO_ENABLE_LOGGING %d\n", NABTO_ENABLE_LOGGING);
    fprintf(f, "/*---------------------------------------------------------*/\n");
    fprintf(f, "#define NABTO_ENABLE_STREAM %d\n", NABTO_ENABLE_STREAM);
    fprintf(f, "#define NABTO_ENABLE_MICRO_STREAM %d\n", NABTO_ENABLE_MICRO_STREAM);
    fprintf(f, "#define NABTO_ENABLE_NANO_STREAM %d\n", NABTO_ENABLE_NANO_STREAM);

    fprintf(f, "/*---------------------------------------------------------*/\n");
    fprintf(f, "#define NABTO_ENABLE_UCRYPTO %d\n", NABTO_ENABLE_UCRYPTO);
    fprintf(f, "/*---------------------------------------------------------*/\n");
    fprintf(f, "#define NABTO_ENABLE_CLIENT_ID %d\n", NABTO_ENABLE_CLIENT_ID);
    fprintf(f, "#define NABTO_ENABLE_CONNECTION_ESTABLISHMENT_ACL_CHECK %d\n", NABTO_ENABLE_CONNECTION_ESTABLISHMENT_ACL_CHECK);
    fprintf(f, "#define NABTO_ENABLE_TCP_FALLBACK %d\n", NABTO_ENABLE_TCP_FALLBACK);
    fprintf(f, "/*---------------------------------------------------------*/\n");
    fprintf(f, "#define NABTO_ENABLE_EVENTCHANNEL %d\n", NABTO_ENABLE_EVENTCHANNEL);
    fprintf(f, "#define NABTO_SET_TIME_FROM_ALIVE %d\n", NABTO_SET_TIME_FROM_ALIVE);
    fprintf(f, "#define NABTO_ENABLE_STATUS_CALLBACKS %d\n", NABTO_ENABLE_STATUS_CALLBACKS);
    fprintf(f, "/*---------------------------------------------------------*/\n");
    fprintf(f, "#define NABTO_THREAD_LOCAL_STORAGE %s\n", STR(NABTO_THREAD_LOCAL_STORAGE));

    fprintf(f, "\n");
    fprintf(f, "%s: Derived values\n", progname);
    fprintf(f, "/*=========================================================*/\n");
    fprintf(f, "#define NABTO_ENABLE_REMOTE_ACCESS %d\n", NABTO_ENABLE_REMOTE_ACCESS);
    fprintf(f, "#define NABTO_ENABLE_LOCAL_ACCESS  %d\n", NABTO_ENABLE_LOCAL_ACCESS);
    fprintf(f, "#define NABTO_ENABLE_CONNECTIONS   %d\n", NABTO_ENABLE_CONNECTIONS);
}

void unabto_printf_memory_sizes(FILE * f, const char *progname)
{
    if (!progname) progname = "";
    fprintf(f, "%s: Sizes\n", progname);
    fprintf(f, "#define NABTO_COMMUNICATION_BUFFER_SIZE %u\n", NABTO_COMMUNICATION_BUFFER_SIZE);
#if NABTO_ENABLE_CONNECTIONS
    fprintf(f, "#define NABTO_CONNECTIONS_SIZE %u\n", NABTO_CONNECTIONS_SIZE);
#endif
#if NABTO_ENABLE_STREAM
    fprintf(f, "#define NABTO_STREAM_MAX_STREAMS %u\n", NABTO_STREAM_MAX_STREAMS);
#endif
    fprintf(f, "\n");
    fprintf(f, "%s: unabto_memory.h\n", progname);
    fprintf(f, "NABTO_THREAD_LOCAL_STORAGE uint8_t nabtoCommunicationBuffer[%u];\n", NABTO_COMMUNICATION_BUFFER_SIZE);
    fprintf(f, "NABTO_THREAD_LOCAL_STORAGE uint16_t nabtoCommunicationBufferSize;\n");
    fprintf(f, "NABTO_THREAD_LOCAL_STORAGE nabto_main_context nmc;  /* %u bytes */\n", (unsigned int) sizeof(nmc));
#if NABTO_ENABLE_CONNECTIONS
    fprintf(f, "NABTO_THREAD_LOCAL_STORAGE nabto_connect connections[%u];  /* %u * %u bytes = %u bytes */\n", NABTO_CONNECTIONS_SIZE, NABTO_CONNECTIONS_SIZE, (unsigned int) sizeof(nabto_connect), (unsigned int) sizeof(connections));
#endif
#if NABTO_ENABLE_STREAM && NABTO_ENABLE_MICRO_STREAM
    fprintf(f, "NABTO_THREAD_LOCAL_STORAGE nabto_stream stream__[%u];  /* %u * %u bytes = %u bytes */\n", NABTO_STREAM_MAX_STREAMS, NABTO_STREAM_MAX_STREAMS, (unsigned int) sizeof(unabto_stream), (unsigned int) sizeof(unabto_stream) * NABTO_STREAM_MAX_STREAMS);
#endif
#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
    fprintf(f, "\n");
    fprintf(f, "%s: unabto_app_adapter.c\n", progname);
    fprintf(f, "NABTO_THREAD_LOCAL_STORAGE queue_event queue[%u];  /* %u * %u bytes = %u bytes */\n", NABTO_APPREQ_QUEUE_SIZE, NABTO_APPREQ_QUEUE_SIZE, (unsigned int) sizeof(struct fake_queue_event), (unsigned int) (NABTO_APPREQ_QUEUE_SIZE * sizeof(struct fake_queue_event)));
    fprintf(f, "     /* sizeof(queue_event) = %u bytes */\n", (unsigned int) sizeof(struct fake_queue_event));
    fprintf(f, "     /* sizeof(queue_event) = %u + NABTO_REQUEST_MAX_SIZE bytes */\n", (unsigned int) (sizeof(struct fake_queue_event) - NABTO_REQUEST_MAX_SIZE));
#endif
    fprintf(f, "\n");
    fprintf(f, "%s: unabto_config.h\n", progname);
    fprintf(f, "#define NABTO_DEVICE_NAME_MAX_SIZE          %d\n", NABTO_DEVICE_NAME_MAX_SIZE);
    fprintf(f, "#define NABTO_DEVICE_VERSION_MAX_SIZE       %d\n", NABTO_DEVICE_VERSION_MAX_SIZE);
    fprintf(f, "#define NABTO_URL_OVERRIDE_MAX_SIZE         %d\n", NABTO_URL_OVERRIDE_MAX_SIZE);
    fprintf(f, "#define NABTO_CLIENT_ID_MAX_SIZE            %d\n", NABTO_CLIENT_ID_MAX_SIZE);
    fprintf(f, "#define NABTO_EVENT_CHANNEL_MAX_SIZE        %d\n", NABTO_EVENT_CHANNEL_MAX_SIZE);
    fprintf(f, "#define NABTO_RESPONSE_MAX_SIZE             %d\n", NABTO_RESPONSE_MAX_SIZE);
    fprintf(f, "#define NABTO_REQUEST_MAX_SIZE              %d\n", NABTO_REQUEST_MAX_SIZE);
#if NABTO_ENABLE_STREAM
    fprintf(f, "/*---------------------------------------------------------*/\n");
    fprintf(f, "#define NABTO_STREAM_RECEIVE_SEGMENT_SIZE   %d\n", NABTO_STREAM_RECEIVE_SEGMENT_SIZE);
    fprintf(f, "#define NABTO_STREAM_RECEIVE_WINDOW_SIZE    %d\n", NABTO_STREAM_RECEIVE_WINDOW_SIZE);
    fprintf(f, "#define NABTO_STREAM_SEND_SEGMENT_SIZE      %d\n", NABTO_STREAM_SEND_SEGMENT_SIZE);
    fprintf(f, "#define NABTO_STREAM_SEND_WINDOW_SIZE       %d\n", NABTO_STREAM_SEND_WINDOW_SIZE);
    fprintf(f, "     /* Streaming buffer storage: %u * %u bytes = %u bytes (included in stream__)*/\n",
             NABTO_STREAM_MAX_STREAMS,
             NABTO_STREAM_RECEIVE_WINDOW_SIZE * NABTO_STREAM_RECEIVE_SEGMENT_SIZE + NABTO_STREAM_SEND_WINDOW_SIZE * NABTO_STREAM_SEND_SEGMENT_SIZE,
             (NABTO_STREAM_RECEIVE_WINDOW_SIZE * NABTO_STREAM_RECEIVE_SEGMENT_SIZE + NABTO_STREAM_SEND_WINDOW_SIZE * NABTO_STREAM_SEND_SEGMENT_SIZE) * NABTO_STREAM_MAX_STREAMS);
#endif

#if NABTO_ENABLE_STREAM && NABTO_ENABLE_MICRO_STREAM
    fprintf(f, "     /* Micro streaming union  storage: %u * %u bytes = %u bytes (includes buffer storage)*/\n",
             NABTO_STREAM_MAX_STREAMS,
             (unsigned int) sizeof(struct nabto_stream_tcb),
             NABTO_STREAM_MAX_STREAMS * (unsigned int) sizeof(struct nabto_stream_tcb));
#endif
    fprintf(f, "\n");
    fprintf(f, "%s: unabto_config_derived.h\n", progname);
    fprintf(f, "#define UNABTO_HEADER_MAX_SIZE             %d\n", UNABTO_HEADER_MAX_SIZE);
    fprintf(f, "#define UNABTO_WINDOW_PAYLOAD_DATA_SIZE    %d\n", UNABTO_WINDOW_PAYLOAD_DATA_SIZE);
    fprintf(f, "#define UNABTO_WINDOW_PAYLOAD_MAX_SIZE     %d\n", UNABTO_WINDOW_PAYLOAD_MAX_SIZE);
    fprintf(f, "#define UNABTO_CRYPTO_PAYLOAD_SIZE(0)      %d\n", UNABTO_CRYPTO_PAYLOAD_SIZE(0));
    fprintf(f, "#define UNABTO_DIALOGUE_PAYLOAD_MAX_SIZE   %d\n", UNABTO_DIALOGUE_PAYLOAD_MAX_SIZE);
    fprintf(f, "/*---------------------------------------------------------*/\n");
    fprintf(f, "#define UNABTO_INVITE_BS_PACKET_MAX_SIZE   %d\n", UNABTO_INVITE_BS_PACKET_MAX_SIZE);
    fprintf(f, "#define UNABTO_INVITE_GSP_PACKET_MAX_SIZE  %d\n", UNABTO_INVITE_GSP_PACKET_MAX_SIZE);
    fprintf(f, "#define UNABTO_ATTACH_REQ_GSP_PACKET_SIZE  %d\n", UNABTO_ATTACH_REQ_GSP_PACKET_SIZE);
    fprintf(f, "#define UNABTO_ATTACH_RSP_GSP_PACKET_SIZE  %d\n", UNABTO_ATTACH_RSP_GSP_PACKET_SIZE);
    fprintf(f, "#define UNABTO_CONNECT_REQ_PACKET_MAX_SIZE %d\n", UNABTO_CONNECT_REQ_PACKET_MAX_SIZE);
    fprintf(f, "#define UNABTO_CONNECT_RSP_PACKET_SIZE     %d\n", UNABTO_CONNECT_RSP_PACKET_SIZE);
    fprintf(f, "#define UNABTO_ALIVE_REQ_SIZE              %d\n", UNABTO_ALIVE_REQ_SIZE);
    fprintf(f, "#define UNABTO_ALIVE_RSP_MAX_SIZE          %d\n", UNABTO_ALIVE_RSP_MAX_SIZE);
    fprintf(f, "/*---------------------------------------------------------*/\n");
#if NABTO_ENABLE_STREAM
    fprintf(f, "#define USER_STREAM_DATA_SIZE              %d\n", USER_STREAM_DATA_SIZE);
    fprintf(f, "#define UNABTO_STREAM_DATA_SIZE            %d\n", UNABTO_STREAM_DATA_SIZE);
    fprintf(f, "#define UNABTO_STREAM_CTRL_SIZE            %d\n", UNABTO_STREAM_CTRL_SIZE);
    fprintf(f, "#define UNABTO_STREAM_PAYLOADS_MAX_SIZE    %d\n", UNABTO_STREAM_PAYLOADS_MAX_SIZE);
    fprintf(f, "/*---------------------------------------------------------*/\n");
#endif
    fprintf(f, "#define UNABTO_PAYLOAD_MAX_SIZE            %d\n", UNABTO_PAYLOAD_MAX_SIZE);
    fprintf(f, "#define UNABTO_DATA_PACKET_MAX_SIZE        %d\n", UNABTO_DATA_PACKET_MAX_SIZE);
    fprintf(f, "/*---------------------------------------------------------*/\n");
    fprintf(f, "#define UNABTO_COMMUNICATION_BUFFER_SIZE   %d\n", UNABTO_COMMUNICATION_BUFFER_SIZE);
}

