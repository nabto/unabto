/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _STATEFUL_PUSH_SERVICE_H_
#define _STATEFUL_PUSH_SERVICE_H_
#include "unabto/unabto_push.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef void (*pushCallback)(void*, const unabto_push_hint*);
typedef struct push_payload_data
{
    uint8_t purpose;
    uint8_t encoding;
    const uint8_t* data;
    uint16_t len;
}push_payload_data;

uint8_t* unabto_push_notification_get_data(uint8_t* bufStart, const uint8_t* bufEnd, uint32_t seq);
void unabto_push_notification_callback(uint32_t seq, unabto_push_hint* hint);

void send_push_notification(uint16_t pnsid, push_payload_data staticData, push_payload_data msg, pushCallback cb, void* cbArgs);


#ifdef __cplusplus
} //extern "C"
#endif

#endif // _STATEFUL_PUSH_SERVICE_H_
