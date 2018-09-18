/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/**
 * @file
 * Common main functions for all targets.
 */

#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_UNABTO

#ifdef WIN32
#include <winsock2.h>
#endif
#include "unabto_common_main.h"
#include "unabto_app.h"
#include "unabto_app_adapter.h"
#include "unabto_stream.h"
#include "unabto_attach.h"
#include "unabto_push.h"
#include "unabto_message.h"
#include "unabto_logging.h"
#include "unabto_env_base.h"
#include "unabto_external_environment.h"
#include "unabto_dns_fallback.h"
#include "unabto_version.h"
#include "unabto_util.h"

#include "unabto_connection.h"
#include "unabto_memory.h"
#include "unabto_packet.h"
#include "unabto_tcp_fallback.h"

#include <string.h>


NABTO_THREAD_LOCAL_STORAGE uint8_t nabtoCommunicationBuffer[NABTO_COMMUNICATION_BUFFER_SIZE];     /**< the communication buffer */
NABTO_THREAD_LOCAL_STORAGE uint16_t nabtoCommunicationBufferSize = NABTO_COMMUNICATION_BUFFER_SIZE;

#if NABTO_ENABLE_CONNECTIONS
#if NABTO_ENABLE_REMOTE_ACCESS || NABTO_ENABLE_UCRYPTO
// These crypto contexts have been split out to reduce maximum object size (specifically for the nmc object) in honour of the PIC18.
NABTO_THREAD_LOCAL_STORAGE nabto_crypto_context cryptoContextAttach;                            /**< context for U_ATTACH packets    */
NABTO_THREAD_LOCAL_STORAGE nabto_crypto_context cryptoContextConnection;                        /**< context for U_CONNECT + U_ALIVE */
#endif
#endif

static ssize_t read_event_socket(nabto_socket_t socket, message_event* event);
static void ensureValidDeviceId(const char* id);
#if NABTO_ENABLE_DNS_FALLBACK
static bool unabto_read_dns_fallback();
#endif

NABTO_THREAD_LOCAL_STORAGE nabto_main_context nmc;

void unabto_init_default_values(nabto_main_setup* nms) {
    nms->controllerArg.port = 5566;
    nms->controllerArg.addr.type = NABTO_IP_NONE; // NONE==> use DNS, ANY ==> don't use BS/GSP, other ==> use option as BS address
    nms->localPort = 5570;
    nms->bufsize = NABTO_COMMUNICATION_BUFFER_SIZE;
    nms->id = 0;
    nms->version = 0;
    nms->url = 0;
    nms->gspPollTimeout = 13000; /* should be between 1 and 2 times the GSP interval(10000) */
    nms->gspTimeoutCount = 5; /* timeout after 5*13000 msecs                             */
    nms->secureAttach = false;
    nms->secureData = false;
    nms->configuredForAttach = true;
    nms->enableRemoteAccess = true;
#if NABTO_ENABLE_CONNECTIONS
    nms->cryptoSuite = CRYPT_W_NULL_DATA;
#endif
    memset(nms->presharedKey, 0, PRE_SHARED_KEY_SIZE);
#if NABTO_ENABLE_TCP_FALLBACK
    nms->enableTcpFallback = true;
#endif
#if NABTO_ENABLE_DNS_FALLBACK
    nms->enableDnsFallback = false;
    nms->forceDnsFallback = false;
    nms->dnsAddress = UNABTO_INADDR_NONE;
#endif

#if NABTO_ENABLE_DYNAMIC_MEMORY
    nms->connectionsSize = NABTO_CONNECTIONS_SIZE;
    nms->streamMaxStreams = NABTO_STREAM_MAX_STREAMS;
    nms->streamReceiveWindowSize = NABTO_STREAM_RECEIVE_WINDOW_SIZE;
    nms->streamSendWindowSize = NABTO_STREAM_SEND_WINDOW_SIZE;
    nms->streamSendSegmentPoolSize = NABTO_STREAM_SEND_SEGMENT_POOL_SIZE;
#endif

#if NABTO_ENABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS
    nms->enableExtendedRendezvousMultipleSockets = true;
#endif
    
}

/**
 * Initialize basic structures and give a setup structure back to the
 * user.
 * When the setup struct is changed appropriately for the application
 * call unabto_init();
 */
nabto_main_setup* unabto_init_context(void) {
    memset(&nmc, 0, sizeof(nabto_main_context));
#if NABTO_ENABLE_CONNECTIONS
#if NABTO_ENABLE_REMOTE_ACCESS || NABTO_ENABLE_UCRYPTO
    nmc.context.cryptoAttach = &cryptoContextAttach;
    nmc.context.cryptoConnect = &cryptoContextConnection;
#endif
#endif
    nmc.socketGSP = NABTO_INVALID_SOCKET;
    nmc.socketLocal = NABTO_INVALID_SOCKET;
    unabto_init_default_values(&nmc.nabtoMainSetup);
    return &nmc.nabtoMainSetup;
}

bool unabto_init(void) {
    ensureValidDeviceId(nmc.nabtoMainSetup.id);
    NABTO_LOG_INFO(("Device id: '%s'", nmc.nabtoMainSetup.id));
    NABTO_LOG_INFO(("Program Release " PRIversion, MAKE_VERSION_PRINTABLE()));
    if(nmc.nabtoMainSetup.version)
    {
        NABTO_LOG_INFO(("Version string: '%s'", nmc.nabtoMainSetup.version));
    }

#if NABTO_ENABLE_DYNAMIC_MEMORY
    if (!unabto_allocate_memory(&nmc.nabtoMainSetup))
    {
        NABTO_LOG_FATAL(("Could not initialize memory"));
        return false;
    }
#endif

#if NABTO_ENABLE_TCP_FALLBACK
    if(!unabto_tcp_fallback_module_init()) {
        NABTO_LOG_FATAL(("Could not initialize tcp fallback module"));
    }
#endif
    
#if NABTO_ENABLE_CONNECTIONS
    nabto_init_connections();
#endif

#if NABTO_ENABLE_LOCAL_ACCESS
    if (!nabto_init_socket(&nmc.nabtoMainSetup.localPort, &nmc.socketLocal)) {
        NABTO_LOG_ERROR(("failed to initialize local socket continueing without local"));
        nmc.socketLocal = NABTO_INVALID_SOCKET;
    }
#endif

#if NABTO_ENABLE_REMOTE_ACCESS
    nmc.socketGSPLocalEndpoint.port = 0;
    if (nmc.nabtoMainSetup.enableRemoteAccess) {
        if (!nabto_init_socket(&nmc.socketGSPLocalEndpoint.port, &nmc.socketGSP))
        {
            return false;
        }
    } else {
        nmc.socketGSP = NABTO_INVALID_SOCKET;
    }
#endif

#if NABTO_ENABLE_CONNECTIONS
    /* Initialize framework event handler (SYNC or ASYNC model) */
    init_application_event_framework();
#endif

#if NABTO_ENABLE_CONNECTIONS
    unabto_packet_init_handlers();
    unabto_packet_set_handler(3,3,&handle_framing_ctrl_packet, 0);
    unabto_packet_set_handler(0,0,&handle_naf_packet, 0);
#endif

#if NABTO_ENABLE_STREAM
    unabto_stream_init();
#endif

#if NABTO_ENABLE_PUSH
    unabto_push_init();
#endif

#if NABTO_ENABLE_REMOTE_ACCESS
    if (!nabto_context_init()) {
        NABTO_LOG_ERROR(("Failed to initialize buffers"));
        return false;
    }
#endif

#if NABTO_ENABLE_DNS_FALLBACK
    if (nmc.nabtoMainSetup.enableDnsFallback) {
        if (!unabto_dns_fallback_init()) {
            NABTO_LOG_ERROR(("Failed to initialize dns fallback"));
            return false;
        }
    }
#endif

#if NABTO_ENABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS
    unabto_extended_rendezvous_init();
#endif
    
    NABTO_LOG_INFO(("Nabto was successfully initialized"));
    return true;
}

void unabto_close(void) {
#if NABTO_ENABLE_CONNECTIONS
    nabto_terminate_connections();
#endif

#if NABTO_ENABLE_LOCAL_ACCESS
    if (nmc.socketLocal != NABTO_INVALID_SOCKET) {
        nabto_close_socket(&nmc.socketLocal);
    }
#endif
#if NABTO_ENABLE_REMOTE_ACCESS
    if (nmc.socketGSP != NABTO_INVALID_SOCKET) {
        nabto_close_socket(&nmc.socketGSP);
    }
#endif
#if NABTO_ENABLE_REMOTE_ACCESS
    nabto_context_release();
#endif

#if NABTO_ENABLE_PUSH
    unabto_push_stop();
#endif

#if NABTO_ENABLE_EXTENDED_RENDEZVOUS_MULTIPLE_SOCKETS
    unabto_extended_rendezvous_close();
#endif

#if NABTO_ENABLE_DYNAMIC_MEMORY
    unabto_free_memory();
#endif
        
}

/***************************************/

void unabto_tick(void) {
#if NABTO_ENABLE_LOCAL_ACCESS
    if (unabto_read_socket(nmc.socketLocal)) {
        return; // local answer produced
    }
#endif

#if NABTO_ENABLE_REMOTE_ACCESS
    if (nmc.socketGSP == NABTO_INVALID_SOCKET) {
        return; // not ready to operate remote connections
    }
    if (unabto_read_socket(nmc.socketGSP)) {
        return; // remote answer produced
    }
#if NABTO_ENABLE_DNS_FALLBACK
    if (nmc.nabtoMainSetup.enableDnsFallback) {
        if (unabto_read_dns_fallback()) {
            return; // dns answer produced
        }
    }
#endif
#endif

    unabto_time_event();
}

void unabto_time_event(void) {
#if NABTO_ENABLE_CONNECTIONS
    nabto_time_event_connection();
    nabto_message_async_response_poll();
#endif

#if NABTO_ENABLE_STREAM
    unabto_time_event_stream();
#endif
    
#if NABTO_ENABLE_REMOTE_ACCESS
    nabto_attach_time_event();
#endif

#if NABTO_ENABLE_PUSH
    if(nmc.context.state == NABTO_AS_ATTACHED){
        nabto_time_event_push();
    }
#endif

#if NABTO_ENABLE_DNS_FALLBACK
    if (nmc.nabtoMainSetup.enableDnsFallback) {
        unabto_dns_fallback_handle_timeout();
    }
#endif

}

bool unabto_set_aes_crypto(nabto_main_setup* nms, uint8_t* preSharedKey, size_t pskLength){
    nms->secureAttach = true;
    nms->secureData = true;
    nms->cryptoSuite = CRYPT_W_AES_CBC_HMAC_SHA256;
    if (pskLength != PRE_SHARED_KEY_SIZE) {
        NABTO_LOG_ERROR(("The pre shared key buffer needs to be exactly %i bytes", PRE_SHARED_KEY_SIZE));
        return false;
    }
    memcpy(nms->presharedKey,preSharedKey,pskLength);
    return true;
}

void unabto_set_no_crypto(nabto_main_setup* nms){
    nms->secureAttach = false;
    nms->secureData = false;
}

static ssize_t read_event_socket(nabto_socket_t socket, message_event* event) {
    ssize_t ilen;
    event->type = MT_UDP;
    event->udpMessage.socket = socket;
    ilen = nabto_read(socket, nabtoCommunicationBuffer, nabtoCommunicationBufferSize, &event->udpMessage.peer.addr, &event->udpMessage.peer.port);
    return ilen;
}

bool unabto_read_socket(nabto_socket_t socket) {
    message_event event;
    ssize_t ilen;    
    ilen = read_event_socket(socket, &event);

    if (ilen <= 0) {
        return false; /* no packets are sent for sure */
    }
#if NABTO_ENABLE_LOCAL_ACCESS 
    if (socket == nmc.socketLocal) {
        NABTO_LOG_TRACE(("Received local packet length %" PRIsize, ilen));
        
        nabto_message_local_event(&event, (uint16_t)ilen);
    }
#endif

#if NABTO_ENABLE_REMOTE_ACCESS
    if (socket != nmc.socketLocal) {
        NABTO_LOG_TRACE(("Received remote packet length %" PRIsize, ilen));
        nabto_message_event(&event, (uint16_t)ilen);
    }
#endif
    return true;
}

#if NABTO_ENABLE_DNS_FALLBACK
bool unabto_read_dns_fallback() {
    {
        uint16_t ilen = unabto_dns_fallback_recv_socket(nabtoCommunicationBuffer, nabtoCommunicationBufferSize);
        if (ilen > 0) {
            unabto_dns_fallback_handle_packet(nabtoCommunicationBuffer, ilen);
        }
    }
    if (nmc.context.useDnsFallback) {
        size_t ilen;
        uint32_t addr;
        uint16_t port;
        ilen = unabto_dns_fallback_recv_from(nabtoCommunicationBuffer, nabtoCommunicationBufferSize, &addr, &port);
        if (ilen > 0) {
            message_event event;
            event.type = MT_UDP;
            event.udpMessage.socket = nmc.socketGSP;
            event.udpMessage.peer.addr = addr;
            event.udpMessage.peer.port = port;
            nabto_message_event(&event, (uint16_t)ilen);
            return true;
        }
    }
    return false;
}
#endif

bool unabto_is_connected_to_gsp(void) {
    return nmc.context.state == NABTO_AS_ATTACHED;
}

nabto_main_context* unabto_get_main_context(void) {
    return &nmc;
}

#if NABTO_ENABLE_REMOTE_ACCESS
void unabto_notify_ip_changed(struct nabto_ip_address* ip) {
    nmc.socketGSPLocalEndpoint.addr = *ip;
    nabto_network_changed();
}
#endif

static void ensureValidDeviceId(const char* id) {
    if (strlen(id) > NABTO_DEVICE_NAME_MAX_SIZE) {
        NABTO_LOG_FATAL(("Device id exceeds NABTO_DEVICE_NAME_MAX_SIZE (%d characters)", NABTO_DEVICE_NAME_MAX_SIZE));
    }
    while(*id)
    {
        if(!((*id >= 'a' && *id <= 'z') || (*id >= '0' && *id <= '9') || *id == '-' || *id == '.')) 
        {
            NABTO_LOG_FATAL(("%s is not a valid device id, only \"a\" to \"z\", \"0\" to \"9\", hyphen (\"-\") and period (\".\") are allowed in a device id.", nmc.nabtoMainSetup.id));
        }
        id++;
    }
}
