/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/**
 * This file describes the various compile time configuration
 * parameters for uNabto.
 */

#ifndef _UNABTO_CONFIG_DEFAULTS_H_
#define _UNABTO_CONFIG_DEFAULTS_H_

/**
 * Define whether to compile remote access functionality.
 * Without this option enabled it is only possible to connect to
 * the device locally. If remote access is enabled the device will
 * connect to the basestation it resolves based on its hostname.
 * It's then possible to make remote p2p connections to the device.
 */
#ifndef NABTO_ENABLE_REMOTE_CONNECTION
#define NABTO_ENABLE_REMOTE_CONNECTION 1
#endif

/**
 * The local connection is a local connection type which can be
 * established without being attached attached to the
 * basestation. This is neccessary if you want to make connections to
 * a device on an offline lan.
 */
#ifndef NABTO_ENABLE_LOCAL_CONNECTION
#define NABTO_ENABLE_LOCAL_CONNECTION 1
#endif

/**
 * The local psk connection is a local connection type which is
 * associated with a psk
 */
#ifndef NABTO_ENABLE_LOCAL_PSK_CONNECTION
#define NABTO_ENABLE_LOCAL_PSK_CONNECTION 0
#endif

/**
 * Enable the old deprecated legacy 953 bytes protocol. This protocol
 * is deprecated since lot of the newer features requires a connection
 * resource. If the client is newer than 2012 this feature can safely
 * be disabled.
 */
#ifndef NABTO_ENABLE_LOCAL_ACCESS_LEGACY_PROTOCOL
#define NABTO_ENABLE_LOCAL_ACCESS_LEGACY_PROTOCOL 1
#endif

/**
 * Enable usage of multiple sockets for extended rendezvous, this
 * makes p2p connections possible from a device behind symmetric nat
 * to clients behind port restricted nat and firewalls.
 */
#ifndef NABTO_ENABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS
#define NABTO_ENABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS 0
#endif

/**
 * Maximum number of extended rendezvous sockets to use. If set to
 * zero this end will not participate in the extended rendezvous when
 * this is the side between symmetric nat.
 */
#ifndef NABTO_EXTENDED_RENDEZVOUS_MAX_SOCKETS
#define NABTO_EXTENDED_RENDEZVOUS_MAX_SOCKETS 250
#endif

#ifndef NABTO_ENABLE_DNS_FALLBACK
#define NABTO_ENABLE_DNS_FALLBACK 0
#endif

#ifndef NABTO_ENABLE_GET_LOCAL_IP
#define NABTO_ENABLE_GET_LOCAL_IP 0
#endif


/**
 * Limit how many addresses the dns resolver can return to unabto
 * after a successfull resolution.
 */
#ifndef NABTO_DNS_RESOLVED_IPS_MAX
#define NABTO_DNS_RESOLVED_IPS_MAX 3
#endif

/****************************/
/* Connection configuration */
/****************************/

/** Define default max number of concurrent connections */
#ifndef NABTO_CONNECTIONS_SIZE
#define NABTO_CONNECTIONS_SIZE 10
#endif

/** 
 * Define behavior if connection size is exceeded. If enabled, the
 *  application will exit, otherwise MICROSERVER_BUSY is reported to
 *  client
 */
#ifndef NABTO_ENABLE_DEVICE_BUSY_AS_FATAL
#define NABTO_ENABLE_DEVICE_BUSY_AS_FATAL 0
#endif

/** 
 * Enable support for EVENTCHANNEL (piggyback data) functionality.
 * If enabled, events can be sent to the base station by
 * implementing the get_event_buffer2 function described in
 * unabto_app.h.
 */
#ifndef NABTO_ENABLE_EVENTCHANNEL
#define NABTO_ENABLE_EVENTCHANNEL 0
#endif

/** Enable support for setting time from GSP */
#ifndef NABTO_SET_TIME_FROM_ALIVE
#define NABTO_SET_TIME_FROM_ALIVE 1
#endif

/** Define (default) support for saving client ID in connection description */
#ifndef NABTO_ENABLE_CLIENT_ID
#define NABTO_ENABLE_CLIENT_ID 0
#endif

/** Define (default) support for ACL check during connection establishment */
#ifndef NABTO_ENABLE_CONNECTION_ESTABLISHMENT_ACL_CHECK
#define NABTO_ENABLE_CONNECTION_ESTABLISHMENT_ACL_CHECK 0
#endif

/**
 * Enable status callbacks. The status callback functionality enables
 * applications which uses the uNabto framework to get status callbacks
 * when the status of the application changes.
 */
#ifndef NABTO_ENABLE_STATUS_CALLBACKS
#define NABTO_ENABLE_STATUS_CALLBACKS 0
#endif

/**
 * Enable fallback connections to client using TCP/IP
 * connections from micro app.
 */
#ifndef NABTO_ENABLE_TCP_FALLBACK
#define NABTO_ENABLE_TCP_FALLBACK 0
#endif

/***********************************/
/* End of connection configuration */
/***********************************/


/*********************************/
/* packet max size configuration */
/*********************************/

/** Define default maximum length of the device name */
#ifndef NABTO_DEVICE_NAME_MAX_SIZE
#define NABTO_DEVICE_NAME_MAX_SIZE       40
#endif

/** Define default maximum length of the device version string */
#ifndef NABTO_DEVICE_VERSION_MAX_SIZE
#define NABTO_DEVICE_VERSION_MAX_SIZE    10
#endif

/** Define default maximum length of the URL override string */
#ifndef NABTO_URL_OVERRIDE_MAX_SIZE
#define NABTO_URL_OVERRIDE_MAX_SIZE      64
#endif

/** Define default maximum length of the Client Identification (the email) */
#ifndef NABTO_CLIENT_ID_MAX_SIZE
#define NABTO_CLIENT_ID_MAX_SIZE         64
#endif

/**
 * Define default maximum length of the area for event channel data in
 * one alive packet. More than one complete event may be concatenated
 * up to the designated max length. The length includes all characters
 * as in "1:L:message\nEOM\n" (16 or 18 depending on length of /n)
 */
#ifndef NABTO_EVENT_CHANNEL_MAX_SIZE
#define NABTO_EVENT_CHANNEL_MAX_SIZE     224
#endif

/** Define default maximum length of the response data in a dialogue */
#ifndef NABTO_RESPONSE_MAX_SIZE
#define NABTO_RESPONSE_MAX_SIZE          200
#endif

/** Define default maximum length of the request data in a dialogue */
#ifndef NABTO_REQUEST_MAX_SIZE
#define NABTO_REQUEST_MAX_SIZE           NABTO_RESPONSE_MAX_SIZE
#endif

/****************************************/
/* End of packet max size configuration */
/****************************************/


/******************************/
/* Asynchronous configuration */
/******************************/

/** Sets synchronous event model as default */
#ifndef NABTO_APPLICATION_EVENT_MODEL_ASYNC
#define NABTO_APPLICATION_EVENT_MODEL_ASYNC  0
#endif

/** Sets the number of request resources queued for execution to 1 */
#ifndef NABTO_APPREQ_QUEUE_SIZE
#define NABTO_APPREQ_QUEUE_SIZE              1
#endif

// Deprecated option
#ifdef NABTO_APPLICATION_EVENT_APPLICATION_SAVES_PARAMETERS
#error "NABTO_APPLICATION_EVENT_APPLICATION_SAVES_PARAMETERS is removed. An application is now forced to save the parameters when it first sees the request or return AER_REQ_OUT_OF_RESOURCES if it is not possible." 
#endif

/*************************************/
/* End of asynchronous configuration */
/*************************************/


/*************************/
/* Logging configuration */
/*************************/

/** 
 * Enable logging by default.
 */
#ifndef NABTO_ENABLE_LOGGING
#define NABTO_ENABLE_LOGGING 1
#endif


/********************************/
/* End of logging configuration */
/********************************/




/****************************/
/* uStreaming configuration */
/****************************/


/** 
 * Define default inclusion of STREAM related functionality. 
 * If NABTO_ENABLE_STREAM is 0 then all streaming functionality is 
 * disabled and the memory requirement will be 0 bytes.
 */
#ifndef NABTO_ENABLE_STREAM
#define NABTO_ENABLE_STREAM 1
#endif

/**
 * MICRO streaming is the default streaming module.
 */
#ifndef NABTO_ENABLE_MICRO_STREAM
#define NABTO_ENABLE_MICRO_STREAM 1
#endif

/**
 * If this option is enabled the user application should implement the
 * unabto_stream_event function which is called each time an event has
 * happened on a stream. This can be used to implement an non polled
 * streaming application.
 */
#ifndef NABTO_ENABLE_STREAM_EVENTS
#define NABTO_ENABLE_STREAM_EVENTS 1
#endif


/**
 * Streams can be configured in the following ways.
 * * Number of simultaneous streams
 * * The send/receive segment size
 * * The send/receive window size
 * 
 * The memory usage for streaming can be calculated as follow
 * mem = NABTO_STREAM_MAX_STREAMS * 
 *       ((NABTO_STREAM_RECEIVE_SEGMENT_SIZE*NABTO_STREAM_RECEIVE_WINDOW_SIZE) +
 *        (NABTO_STREAM_SEND_SEGMENT_SIZE*NABTO_STREAM_SEND_WINDOW_SIZE))
 *
 * The default memory requirement is 4*((1400*4)+(1400*4)) = 44800 bytes.
 */


/** The maximum simultaneous streams. */
#ifndef NABTO_STREAM_MAX_STREAMS
#define NABTO_STREAM_MAX_STREAMS 4
#endif

/** The maximum segment size the streaming can receive. 
 * 1391 bytes buffers is the largest possible buffers we can use 
 * when encryption is enables, this results in 1468 bytes nabto packets
 * then add 8 bytes of udp headers and 12 bytes of ip headers and the ip
 * packet has size 1488bytes. Because padding is often 16 bytes choosing
 * a 1392 bytes buffer would result in 1504 bytes ip packets which would
 * result in ip fragmentation on most networks.
 *
 * In practice lower MTU's exists on networks that disallow ip
 * fragmentation, Which means in practice 1311 works really well, but
 * at least in one network it has been observed that 1311 bytes is a
 * problem (NABTO-1117), this is why the value ends at 1247 bytes.
 */
#ifndef NABTO_STREAM_RECEIVE_SEGMENT_SIZE
#define NABTO_STREAM_RECEIVE_SEGMENT_SIZE 1247
#endif

/** The size of the receive window */
#ifndef NABTO_STREAM_RECEIVE_WINDOW_SIZE
#define NABTO_STREAM_RECEIVE_WINDOW_SIZE 4
#endif

/** The maximum send segment size */
#ifndef NABTO_STREAM_SEND_SEGMENT_SIZE
#define NABTO_STREAM_SEND_SEGMENT_SIZE NABTO_STREAM_RECEIVE_SEGMENT_SIZE
#endif

/** The size of the send window size */
#ifndef NABTO_STREAM_SEND_WINDOW_SIZE
#define NABTO_STREAM_SEND_WINDOW_SIZE NABTO_STREAM_RECEIVE_WINDOW_SIZE
#endif

/** Timeout before a new streaming packet is sent, value in ms. */
#ifndef NABTO_STREAM_TIMEOUT
#define NABTO_STREAM_TIMEOUT 1000 
#endif

/** Max number of retransmissions of a packet. */
#ifndef NABTO_STREAM_MAX_RETRANS
#define NABTO_STREAM_MAX_RETRANS 12
#endif

/**
 * Minimum number of retransmissions of data, used in microstreaming.
 * If the rto is large in micro streaming the minimum retrans ensures a packet is
 * resent atleast this number of times.
 */
#ifndef NABTO_STREAM_MIN_RETRANS
#define NABTO_STREAM_MIN_RETRANS 6
#endif

/**
 * Max retransmission time, used in micro streaming.
 * maximum time between retransmission of a packet.
 */
#ifndef NABTO_STREAM_MAX_RETRANSMISSION_TIME
#define NABTO_STREAM_MAX_RETRANSMISSION_TIME 16000 // 16 seconds.
#endif

/**
 * Time to delay sending advertised window data in case the receiver
 * blocks for consuming of received data.
 * 
 * 10 ms seems like a good value, it gives the receiver time to
 * consume the data while also makes the feedback sufficient rapid.
 */
#ifndef NABTO_STREAM_ADVERTISED_WINDOW_REPORT_DELAY
#define NABTO_STREAM_ADVERTISED_WINDOW_REPORT_DELAY 10
#endif

/**
 * Enable unabto_next_event function
 */
#ifndef NABTO_ENABLE_NEXT_EVENT
#define NABTO_ENABLE_NEXT_EVENT 1
#endif

/***********************************/
/* End of uStreaming configuration */
/***********************************/

/*******************************/
/* uNabto crypto configuration */
/*******************************/

/** Define whether to include micro crypto */
#ifndef NABTO_ENABLE_UCRYPTO
#define NABTO_ENABLE_UCRYPTO 1
#endif

/**************************************/
/* end of uNabto crypto configuration */
/**************************************/

/******************************************/
/* uNabto push notification configuration */
/******************************************/

/** Define whether to include push notifications */
#ifndef NABTO_ENABLE_PUSH
#define NABTO_ENABLE_PUSH 0
#endif

/** Define length of push notification queue */
#ifndef NABTO_PUSH_QUEUE_LENGTH
#define NABTO_PUSH_QUEUE_LENGTH 10
#endif

/** Define size of push notification buffer_element data buffer */
#ifndef NABTO_PUSH_BUFFER_ELEMENT_SIZE
#define NABTO_PUSH_BUFFER_ELEMENT_SIZE 1000
#endif

/*************************************************/
/* end of uNabto push notification configuration */
/*************************************************/

/*****************/
/* Debug packets */
/*****************/

#ifndef NABTO_ENABLE_DEBUG_PACKETS
#define NABTO_ENABLE_DEBUG_PACKETS 0
#endif

#ifndef NABTO_ENABLE_DEBUG_SYSLOG_CONFIG
#define NABTO_ENABLE_DEBUG_SYSLOG_CONFIG 0
#endif

/**************************************/
/* End of debug packets configuration */
/**************************************/


/****************/
/* Memory model */
/****************/

/**
 * By defining the NABTO_THREAD_LOCAL_STORAGE to the platforms thread
 * local storage option enables that all static variables are in the
 * threads local storage 
 */
#ifndef NABTO_THREAD_LOCAL_STORAGE
#define NABTO_THREAD_LOCAL_STORAGE
#endif

/**
 * If the memory model is dynamic the memory is allocated when the
 * application starts. Otherwise the memory is allocated static on
 * compile time.
 */
#ifndef NABTO_ENABLE_DYNAMIC_MEMORY
#define NABTO_ENABLE_DYNAMIC_MEMORY 0
#endif

/*************************************/
/* End of memory model configuration */
/*************************************/


#endif
