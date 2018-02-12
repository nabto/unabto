/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_PUSH_H_
#define _UNABTO_PUSH_H_

#include <stdlib.h>
#include <stdint.h>
#include "unabto_env_base.h"
#include "unabto_packet_util.h"

#if NABTO_ENABLE_PUSH

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UNABTO_PUSH_HINT_OK,
    UNABTO_PUSH_HINT_QUEUE_FULL,
    UNABTO_PUSH_HINT_INVALID_DATA_PROVIDED,
    UNABTO_PUSH_HINT_NO_CRYPTO_CONTEXT,
    UNABTO_PUSH_HINT_ENCRYPTION_FAILED,
    UNABTO_PUSH_HINT_FAILED,
    UNABTO_PUSH_HINT_QUOTA_EXCEEDED,
    UNABTO_PUSH_HINT_QUOTA_EXCEEDED_REATTACH
} unabto_push_hint;

typedef enum {
    UNABTO_PUSH_PNS_ID_FIREBASE = 1,
    UNABTO_PUSH_PNS_ID_WEBHOOK = 2
} unabto_push_pns_id;

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
    unabto_push_hint hint;
}unabto_push_element;

struct{
    unabto_push_element* nextPushEvent;
    int pushSeqQHead;
    uint32_t nextSeq;
    nabto_stamp_t lastSent;
    nabto_stamp_t backOffLimit;
    bool reattachNeeded;
}pushCtx;

    
/* ------------------------------------------------------------- *
 * These next two functions must be implemented by the developer *
 * ------------------------------------------------------------- */

/*
 * @param bufStart A pointer to the start of the address space available for data
 * @param bufEnd   A pointer to the end of the address space available for data
 * @param seq      The sequence number of the push notification for which the data is requested
 * @return         Pointer to the first empty space of the buffer after data is added
 */
uint8_t* unabto_push_notification_get_data(uint8_t* bufStart, const uint8_t* bufEnd, uint32_t seq);

/*
 * @param seq  The sequence number of push notification
 * @param hint Pointer to a hint showing the status of the push notification
 */
void unabto_push_notification_callback(uint32_t seq, unabto_push_hint* hint);


/**
 * Send a Push notification. If this function return UNABTO_PUSH_HINT_OK, the 
 * unabto_push_notification_callback function will call once and only once, and always from a new
 * context. If this function returns any other unabto_push_hint, the callback will not be called.
 * @param  pnsId    The ID of the used push notification service as configured in the basestation
 * @param  seq      A pointer to where the sequence number assigned to the PN should be placed
 * @return hint     Information about status of the particular PN
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

/**
 * Initialization function for push notifications called by the core.
 * Should never be called from the application. 
 */
void unabto_push_init(void);


/**
 * Function to stop push notifications called by the core.
 * This resolves all outstanding push notifications with hint FAILED.
 */
void unabto_push_stop(void);

/**
 * help functions for internal use
 */
void nabto_time_event_push(void);

bool nabto_push_event(nabto_packet_header* hdr);

void unabto_push_notify_reattach(void);

#ifdef __cplusplus
} //extern "C"
#endif

#endif // NABTO_ENABLE_PUSH

#endif // _UNABTO_PUSH_H_
