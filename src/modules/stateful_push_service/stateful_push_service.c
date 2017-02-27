/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#include "stateful_push_service.h"
#include <unabto/unabto_protocol_defines.h>
#include <unabto/unabto_util.h>

buffer_element buffer[NABTO_PUSH_QUEUE_LENGTH];
uint8_t dataHead = 0;

unabto_push_hint send_push_notification(uint16_t pnsid, push_payload_data staticData, push_payload_data msg, pushCallback cb, void* cbArgs){
    uint32_t seq;
    uint8_t* ptr;
    if(dataHead >= NABTO_PUSH_QUEUE_LENGTH){
        return UNABTO_PUSH_HINT_QUEUE_FULL;
    }
    buffer[dataHead].len = staticData.len+msg.len+2*NP_PAYLOAD_PUSH_DATA_SIZE_WO_DATA;
    ptr = buffer[dataHead].data;
    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_PUSH_DATA, 0,staticData.len+2);
    WRITE_FORWARD_U8(ptr, staticData.purpose);
    WRITE_FORWARD_U8(ptr, staticData.encoding);
    memcpy(ptr,staticData.data,staticData.len); ptr += staticData.len;
    
    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_PUSH_DATA, 0,msg.len+2);
    WRITE_FORWARD_U8(ptr, msg.purpose);
    WRITE_FORWARD_U8(ptr, msg.encoding);
    memcpy(ptr,msg.data,msg.len); ptr += msg.len;

    buffer[dataHead].cb = cb;
    buffer[dataHead].args = cbArgs;
    
    unabto_send_push_notification(pnsid, &seq);
    buffer[dataHead].seq = seq;
    dataHead++;
    return UNABTO_PUSH_HINT_OK;
}

uint8_t* unabto_push_notification_get_data(uint8_t* bufStart, const uint8_t* bufEnd, uint32_t seq){
    int i;
    for(i=0;i<dataHead;i++){
        if(buffer[i].seq == seq){
            if(buffer[i].len>bufEnd-bufStart){
                return NULL;
            }
            memcpy( bufStart,buffer[i].data,buffer[i].len);
            return bufStart+buffer[i].len;
        }
    }
}

void unabto_push_notification_callback(uint32_t seq, unabto_push_hint* hint){
    int i;
    for(i=0;i<dataHead;i++){
        if(buffer[i].seq == seq){
            memmove(&buffer[i].data, &buffer[dataHead-1].data,sizeof(buffer_element));
            buffer[i].cb(buffer[i].args,hint);
            dataHead--;
            return;
        }
    }   
}
