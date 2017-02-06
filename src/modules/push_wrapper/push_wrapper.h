/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _PUSH_WRAPPER_H_
#define _PUSH_WRAPPER_H_
#define UNABTO_PUSH_CALLBACK_FUNCTIONS
#include "unabto/unabto_push.h"


#ifdef __cplusplus
extern "C" {
#endif
enum {
    PUSH_DATA_PURPOSE_STATIC = 1,
    PUSH_DATA_PURPOSE_MSG = 2
};

enum {
    PUSH_DATA_TYPE_JSON = 1
};
uint8_t* unabto_push_notification_get_data(uint8_t* bufStart, const uint8_t* bufEnd, uint32_t seq);
void unabto_push_notification_callback(uint32_t seq, unabto_push_hint* hint);

void send_push_notification(uint16_t pnsid, const char * staticData, size_t lenSD, const char * msg, size_t lenMsg );


#ifdef __cplusplus
} //extern "C"
#endif

#endif
