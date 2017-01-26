/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_PUSH_H_
#define _UNABTO_PUSH_H_

#include <stdlib.h>
#include <stdint.h>
#include "unabto_env_base.h"
#include "unabto_packet_util.h"

//#include "unabto_config_defaults.h"

#if NABTO_ENABLE_PUSH

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UNABTO_PUSH_HINT_OK,
    UNABTO_PUSH_HINT_QUEUE_FULL,
    UNABTO_PUSH_HINT_INVALID_DATA_PROVIDED,
    UNABTO_PUSH_HINT_ENCRYPTION_FAILED,
    UNABTO_PUSH_HINT_FAILED,
    UNABTO_PUSH_HINT_QUOTA_EXCEEDED,
    UNABTO_PUSH_HINT_QUOTA_EXCEEDED_REATTACH
} unabto_push_hint;

typedef enum {
    UNABTO_PUSH_IDLE,
    UNABTO_PUSH_WAITING_SEND,
    UNABTO_PUSH_AWAITING_ACK,
    UNABTO_PUSH_RETRANS
}unabto_push_element_state;

typedef struct unabto_push_element{
    uint32_t seq;
    uint8_t retrans;
    unabto_push_element_state state;
    nabto_stamp_t stamp;
    uint16_t pnsId;
}unabto_push_element;

//#ifndef UNABTO_PUSH_CALLBACK_FUNCTIONS
//#define UNABTO_PUSH_CALLBACK_FUNCTIONS
//#define unabto_push_notification_get_data(bufStart,bufEnd,seq) unabto_push_get_data_mock(bufStart,bufEnd, seq)
//#define unabto_push_notification_callback(seq,hint) unabto_push_callback_mock(seq, hint)
//uint8_t* unabto_push_get_data_mock(uint8_t* bufStart, const uint8_t* bufEnd, uint32_t seq){return bufStart;}
//void unabto_push_callback_mock(uint32_t seq, unabto_push_hint* hint){}
//extern uint16_t* unabto_push_notification_get_data(const uint8_t* bufStart, const uint8_t* bufEnd, uint32_t seq){}
//extern void unabto_push_notification_callback(uint32_t seq, unabto_push_hint* hint){}
//#endif


/**
 * Initialization of push notifications. Should be called before using push. 
 */
void unabto_push_init(void);

/**
 * Send a Push notification
 * @param pnsId    The ID of the used push notification service as configured in the basestation
 * @param seq      A pointer to where the sequence number assigned to the PN should be placed
 * @param hint     Information about status of the particular PN
 * -  UNABTO_PUSH_HINT_OK
 * -  UNABTO_PUSH_HINT_QUEUE_FULL
 * @return seqNr   The sequence number assigned to the particular PN.
 */
unabto_push_hint unabto_send_push_notification(uint16_t pnsId, uint32_t* seq);

/**
 * Remove a push notification if it has not already been acknowledged
 * @param seq The sequence number which should be removed
 * @return true if the seq was removed, false if seq not found
 */
bool unabto_push_notification_remove(uint32_t seq);

/**
 * Get the data size available for push notification data
 * @return size The data size available for push notifications
 */
uint16_t unabto_push_notification_data_size(void);

void nabto_time_event_push(void);

bool nabto_push_event(nabto_packet_header* hdr);

#ifdef __cplusplus
} //extern "C"
#endif

#endif // NABTO_ENABLE_PUSH

#endif // _UNABTO_PUSH_H_
