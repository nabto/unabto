/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */


#ifndef _UNABTO_PUSH_H_
#define _UNABTO_PUSH_H_


//#if NABTO_ENABLE_PUSH
#include <stdlib.h>
#include <stdint.h>
#include "unabto_env_base.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
//enum unabto_push_hint{
UNABTO_PUSH_HINT_OK,
    UNABTO_PUSH_HINT_QUEUE_FULL,
    UNABTO_PUSH_HINT_FAILED,
    UNABTO_PUSH_HINT_QUOTA_EXCEEDED,
    UNABTO_PUSH_HINT_QUOTA_EXCEEDED_REATTACH
} unabto_push_hint;

typedef enum {
IDLE,
    WAITING_TO_BE_SENT,
    AWAITING_ACK
}unabto_push_element_state;

typedef struct unabto_push_element{
    uint32_t seq;
    uint8_t retrans;
    unabto_push_element_state state;
    nabto_stamp_t stamp;
}unabto_push_element;

// Should be defined by the developer before Push is used. Not sure if you want extern or not!
extern uint16_t* unabto_push_notification_get_data(const uint8_t* bufStart, const uint8_t* bufEnd, uint32_t seq);
extern void unabto_push_notification_callback(uint32_t seq, unabto_push_hint* hint);


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
 */
void unabto_push_notification_remove(uint32_t seq);

/**
 * Get the data size available for push notification data
 * @return size The data size available for push notifications
 */
uint16_t unabto_push_notification_data_size();

void nabto_time_event_push(void);

#ifdef __cplusplus
} //extern "C"
#endif

//#endif // NABTO_ENABLE_PUSH

#endif // _UNABTO_PUSH_H_
