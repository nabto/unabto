/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#include "stateful_push_service.h"
#include <unabto/unabto_protocol_defines.h>
#include <unabto/unabto_util.h>

#ifndef PUSH_NOTIFICATION_TABLE_SIZE
#define PUSH_NOTIFICATION_TABLE_SIZE 5
#endif
uint8_t* dataBuffer[PUSH_NOTIFICATION_TABLE_SIZE];
uint32_t sequences[PUSH_NOTIFICATION_TABLE_SIZE];
uint16_t lengths[PUSH_NOTIFICATION_TABLE_SIZE];
pushCallback cbList[PUSH_NOTIFICATION_TABLE_SIZE];
void * cbArgList[PUSH_NOTIFICATION_TABLE_SIZE];
uint8_t dataHead = 0;



void send_push_notification(uint16_t pnsid, push_payload_data staticData, push_payload_data msg, pushCallback cb, void* cbArgs){
    uint32_t seq;
    uint8_t* ptr;
    if(dataHead >= PUSH_NOTIFICATION_TABLE_SIZE){
        return;
    }
    lengths[dataHead] = staticData.len+msg.len+2*NP_PAYLOAD_PUSH_DATA_SIZE_WO_DATA;
    dataBuffer[dataHead] = (uint8_t *)malloc(lengths[dataHead]);
    ptr = dataBuffer[dataHead];
    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_PUSH_DATA, 0,staticData.len+2);
    WRITE_U8(ptr, staticData.purpose); ptr++;
    WRITE_U8(ptr, staticData.encoding); ptr++;
    memcpy(ptr,staticData.data,staticData.len); ptr += staticData.len;
    
    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_PUSH_DATA, 0,msg.len+2);
    WRITE_U8(ptr, msg.purpose); ptr++;
    WRITE_U8(ptr, msg.encoding); ptr++;
    memcpy(ptr,msg.data,msg.len); ptr += msg.len;

    cbList[dataHead] = cb;
    cbArgList[dataHead] = cbArgs;
    
    unabto_send_push_notification(pnsid, &seq);
    sequences[dataHead] = seq;
    dataHead++; 
}

uint8_t* unabto_push_notification_get_data(uint8_t* bufStart, const uint8_t* bufEnd, uint32_t seq){
    int i;
    for(i=0;i<dataHead;i++){
        if(sequences[i] == seq){
            if(lengths[i]>bufEnd-bufStart){
                return NULL;
            }
            memcpy( bufStart,dataBuffer[i],lengths[i]);
            return bufStart+lengths[i];
        }
    }
}

void unabto_push_notification_callback(uint32_t seq, unabto_push_hint* hint){
    int i;
    for(i=0;i<dataHead;i++){
        if(sequences[i] == seq){
            free(dataBuffer[i]);
            if(i<PUSH_NOTIFICATION_TABLE_SIZE-1){
                memmove(&dataBuffer[i], &dataBuffer[i+1],dataHead-i);
            }
            cbList[i](cbArgList[i],hint);
            dataHead--;
            return;
        }
    }   
}
