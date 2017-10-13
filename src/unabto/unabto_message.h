/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_MESSAGE_H_
#define _UNABTO_MESSAGE_H_
/**
 * @file
 * Nabto uServer common protocol - Interface.
 */

#include <unabto/unabto_context.h>

/******************************************************************************/
/******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MT_UDP,
    MT_TCP_FALLBACK
} message_type;

typedef struct {
    message_type type;
    struct {
        nabto_socket_t socket;
        nabto_endpoint peer;
    } udpMessage;
} message_event;

#if NABTO_ENABLE_LOCAL_ACCESS
/**
 * To be called to handle packets on the local socket.
 * @param ilen     length of received packet
 * @param peer     peer to send answer to
 */
void nabto_message_local_event(message_event* event, uint16_t ilen);
#endif

#if NABTO_ENABLE_CONNECTIONS
/**
 * Handle a connection based packet.
 * @param ilen     the length of the received packet (in ctx->buf)
 * @param peer     the source endpoint
 */
void nabto_message_event(message_event* event, uint16_t ilen);
#endif

#if NABTO_ENABLE_CONNECTIONS
/**
 * To be called to send an asynchroneous response from previously queued requests.
 * @return     true if a message has been sent.
 */
bool nabto_message_async_response_poll(void);
#endif

#ifdef __cplusplus
} //extern "C"
#endif

#endif
