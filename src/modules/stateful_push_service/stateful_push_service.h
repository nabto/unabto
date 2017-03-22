/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _STATEFUL_PUSH_SERVICE_H_
#define _STATEFUL_PUSH_SERVICE_H_
#include "unabto/unabto_push.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * callback function typedef
 * the callback function used with send_push_notification should be of this type
 * @param void* void pointer to any data the user needs in this context
 * @param unabto_push_hint* pointer to hint to the status of the PN (defined in unabto_push.h)
 */
typedef void (*pushCallback)(void*, const unabto_push_hint*);

/**
 * Structure for the data to be put in the push notification
 * @param purpose  Defines the usage of the data, 1 for static data, 2 for message data
 * @param encoding Defines how the data is encoded, 1 for JSON
 * @param data     Pointer to the data 
 */
typedef struct push_payload_data
{
    uint8_t purpose;
    uint8_t encoding;
    uint8_t data[NABTO_PUSH_BUFFER_ELEMENT_SIZE]; // reusing the length of a buffer element may be too much!
    uint16_t len;
}push_payload_data;

/**
 * push message structure defining a complete push notification
 * @param staticData  Data passed directly from the subscribed client
 * @param dynamicData Data containing message specifics from the device
 * @param pnsId       ID of the PNS to be used
 */
typedef struct push_message
{
    push_payload_data staticData;
    push_payload_data dynamicData;
    uint16_t pnsId;
}push_message;

/**
 * Buffer structure for internal use in the module
 */
typedef struct buffer_element
{
    uint8_t data[NABTO_PUSH_BUFFER_ELEMENT_SIZE];
    uint32_t seq;
    uint16_t len;
    pushCallback cb;
    void * args;
} buffer_element;


unabto_push_hint send_push_message(push_message* msg, pushCallback cb, void* cbArgs);

/**
 * Functions used to build a push_message structure to be sent
 * init_push_message should be called for every push notification
 * add_* functions can be called as needed.
 * only add_title_loc_string_arg and add_body_loc_string_arg should be called multiple
 * times if multiple arguments are needed. 
 * All const char* arguments must be zero terminated
 */
bool init_push_message(push_message* msg, uint16_t pnsid, const char* staticData);
bool add_title(push_message* msg, const char* title);
bool add_body(push_message* msg, const char* body);
bool add_title_loc_key(push_message* msg, const char* titleKey);
bool add_title_loc_string_arg(push_message* msg, const char* titleArg);
bool add_body_loc_key(push_message* msg, const char* bodyKey);
bool add_body_loc_string_arg(push_message* msg, const char* bodyArg);



/**
 * Function used to send push notifications. Push notifications are sent asyncronously, meaning this 
 * function will return immediately. The PN can be considered handled when the provided callback 
 * function is called. This function maintains its own memory, any data provided can be freed after 
 * function return.
 * @param pnsid      The ID of the PNS used, 1 for silent OneSignals
 * @param staticData Data structure containing info static to all PNs of this device, for pnsid=1 
 *                   should be purpose=1 using JSON encoding(encoding=1) and contain App_id and Player_id.
 * @param msg        Data structure containing PN specific information for pnsid=1 should be purpose=2
 *                    using JSON encoding(encoding=1) and contain any data to be pushed.
 * @param cb         Callback function to be called once response is received from basestation, will 
 *                   be called once and only once in a new context, if this succeeds.
 * @param cbArgs     Pointer to arguments to be passed to the callback function for context
 * @return           Hint to the status of the PN, UNABTO_PUSH_HINT_OK if success, UNABTO_PUSH_HINT_QUEUE_FULL
 *                   if queue is full. Callback is NOT called on queue full.
 */
unabto_push_hint send_push_notification(uint16_t pnsid, push_payload_data staticData, push_payload_data msg, pushCallback cb, void* cbArgs);

/**
 * Functions needed for internal interface with the core
 */
uint8_t* unabto_push_notification_get_data(uint8_t* bufStart, const uint8_t* bufEnd, uint32_t seq);
void unabto_push_notification_callback(uint32_t seq, unabto_push_hint* hint);


#ifdef __cplusplus
} //extern "C"
#endif

#endif // _STATEFUL_PUSH_SERVICE_H_
