/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
/**
 * @file
 * Nabto uServer attachment and remote connect protocol - Interface.
 */

#ifndef _UNABTO_ATTACH_H_
#define _UNABTO_ATTACH_H_

#include "unabto_context.h"
#include "unabto_packet_util.h"

#if NABTO_ENABLE_REMOTE_ACCESS

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/**
 * Invite event from the controller. This is received as a reponse on
 * our initial invite packet to the controller.
 * @param ctx the context @param hdr the decoded packet header
 * @return true iff fully treated
 */
bool nabto_invite_event(nabto_packet_header* hdr);

/******************************************************************************/
/**
 * Handle the attach event
 * @param peer     the endpoint of the sender
 * @param hdr      the decoded packet header
 * @return         true iff fully treated
 * A response may be sent
 */
bool nabto_attach_event(nabto_packet_header* hdr);

/******************************************************************************/
/**
 * Handle the alive event
 * @param ctx      the context
 * @param peer     the endpoint of the sender
 * @param hdr      the decoded packet header
 * @return         true iff fully treated
 * A response may be sent
 */
bool nabto_alive_event(nabto_packet_header* hdr);


void send_basestation_attach_failure(uint8_t error_code);

/******************************************************************************/

/******************************************************************************/
/**
 * To be called periodically.
 * @param ctx   the context
 * One or more packets may be sent as response
 */
void nabto_attach_time_event(void);


/**
 * Potentially called if the network configuration has changed
 */
void nabto_network_changed(void);

/**
 * Called whenever the context is released or re-initialised (outside unabto_attach).
 * This function ensures the state change is logged and all listeners are notified.
 */
void nabto_change_context_state(nabto_state state);

#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_INFO)
text stName(nabto_state state);
#else
#define stName(st) #st
#endif


#ifdef __cplusplus
} //extern "C"
#endif

#endif

#endif
