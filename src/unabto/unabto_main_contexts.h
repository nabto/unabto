/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_MAIN_CONTEXTS_H_
#define _UNABTO_MAIN_CONTEXTS_H_

#include <unabto/unabto_env_base.h>
#include <unabto/unabto_context.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    PRE_SHARED_KEY_SIZE = 16
};

/** 
 * The Main Setup struct.
 * The content is never changed during execution.
 */
typedef struct {
    nabto_endpoint    controllerArg;      /**< Controller endpoint argument     */
    uint16_t          localPort;          /**< The port to use for local connections */
    uint32_t          ipAddress;          /**< The IP address to bind to        */
    const char*       interfaceName;      /**< The interface to bind to, unix only */
    uint16_t          bufsize;            /**< Communication buffer size        */
    const char*       id;                 /**< Id for the nabto device          
                                           * The id should be a valid url, 
                                           * all lowercase, and no special
                                           * characters like spaces etc.        */
    const char*       version;            /**< version string (given to clients)*/
    const char*       url;                /**< URL string (given to clients)    */
    uint32_t          gspPollTimeout;     /**< GSP poll timeout in ms           */
    int               gspTimeoutCount;    /**< GSP keep alive times out after 
                                               gspPollTimeout*gspTimeOutCount   */
    bool              secureAttach;       /**< Attach using secure channel      */
    bool              secureData;         /**< Encrypt data                     */
    bool              configuredForAttach; /**< Default true, but it can be set to false if cryptosetup ius delayed. */
#if NABTO_ENABLE_CONNECTIONS
    crypto_suite      cryptoSuite;        /**< Crypto mode, aes, bf or null     */
#endif
    uint8_t           presharedKey[PRE_SHARED_KEY_SIZE];
                                          /**< Preshared key to use with the gsp*/
#if NABTO_ENABLE_TCP_FALLBACK
    bool              enableTcpFallback;  /**< Enable tcp fallback. This is enabled by default if TCP Fallback is compiled into the executable. */
#endif
#if NABTO_ENABLE_DNS_FALLBACK
    bool              enableDnsFallback;  /**< As long as the dns fallback feature is in beta it's neccessary to explicitly enable it on runtime */
    bool              forceDnsFallback;        /**< Force unabto to use dns fallback */
    uint32_t          dnsAddress;         /**< Force a specific dns server address */
    const char*       dnsFallbackDomain;  /**< Force a specific dns fallback node */
#endif

#if NABTO_ENABLE_DYNAMIC_MEMORY
    uint16_t          connectionsSize;
    uint16_t          streamMaxStreams;
    uint16_t          streamReceiveWindowSize;
    uint16_t          streamSendWindowSize;
#endif
    
} nabto_main_setup;

/**
 * Main context of unabto.
 * This context is dynamic.
 */
typedef struct nabto_main_context {
    nabto_socket_t        socketLocal;    /**< socket for local traffic  */
    nabto_socket_t        socketGSP;      /**< socket for remote traffic */
    nabto_endpoint        controllerEp;   /**< controller endpoint       */
    nabto_context         context;        /**< Communication context     */
    nabto_main_setup      nabtoMainSetup; /**< Main setup for uNabto     */
    nabto_endpoint        socketGSPLocalEndpoint;  /**< The local endpoint for the gsp socket. 
                                                      This is the local ip and port the socket is boudnd to. */
} nabto_main_context;


#ifdef __cplusplus
} //extern "C"
#endif

#endif
