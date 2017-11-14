/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#include "push_service.h"
#include <unabto/unabto_protocol_defines.h>
#include <unabto/unabto_util.h>
#include <platforms/unabto_printf_logger.h>

buffer_element buffer[NABTO_PUSH_QUEUE_LENGTH];

uint8_t dataHead = 0;

unabto_push_hint send_push_message(push_message *msg, pushCallback cb, void* cbArgs){
    return send_push_notification(msg->pnsId, msg->staticData, msg->dynamicData,cb,cbArgs);
}

unabto_push_hint send_push_notification(uint16_t pnsid, push_payload_data staticData, push_payload_data msg, pushCallback cb, void* cbArgs){
    uint32_t seq;
    uint8_t* ptr;
    uint8_t* end;
    unabto_push_hint hint;
    if(dataHead >= NABTO_PUSH_QUEUE_LENGTH){
        return UNABTO_PUSH_HINT_QUEUE_FULL;
    }
    buffer[dataHead].len = staticData.len+msg.len+2*NP_PAYLOAD_PUSH_DATA_SIZE_WO_DATA;
    if(buffer[dataHead].len > NABTO_PUSH_BUFFER_ELEMENT_SIZE){
        return UNABTO_PUSH_HINT_INVALID_DATA_PROVIDED;
    }
    ptr = buffer[dataHead].data;
    end = ptr + NABTO_PUSH_BUFFER_ELEMENT_SIZE;
    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_PUSH_DATA, 0,staticData.len+2);
    WRITE_FORWARD_U8(ptr, staticData.purpose);
    WRITE_FORWARD_U8(ptr, staticData.encoding);
    memcpy(ptr,staticData.data,staticData.len); ptr += staticData.len;
    
    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_PUSH_DATA, 0,msg.len+2);
    WRITE_FORWARD_U8(ptr, msg.purpose);
    WRITE_FORWARD_U8(ptr, msg.encoding);
    memcpy(ptr,msg.data,msg.len); ptr += msg.len;

    buffer[dataHead].cb = cb;
    buffer[dataHead].args = cbArgs;
    
    hint = unabto_send_push_notification(pnsid, &seq);
    if(hint == UNABTO_PUSH_HINT_OK){
        buffer[dataHead].seq = seq;
        dataHead++;
        return hint;
    } else {
        return hint;
    }
}


bool init_push_message(push_message* msg, uint16_t pnsid, const char* staticData){
    msg->staticData.purpose = NP_PAYLOAD_PUSH_DATA_PURPOSE_STATIC;
    msg->staticData.encoding = NP_PAYLOAD_PUSH_DATA_ENCODING_JSON;
    msg->staticData.len = strlen(staticData);
    if(msg->staticData.len > NABTO_PUSH_BUFFER_ELEMENT_SIZE){
        return false;
    }
    strcpy((char *)msg->staticData.data,staticData);
    msg->pnsId = pnsid;
    msg->dynamicData.purpose = NP_PAYLOAD_PUSH_DATA_PURPOSE_DYNAMIC;
    msg->dynamicData.encoding = NP_PAYLOAD_PUSH_DATA_ENCODING_TLV;
    msg->dynamicData.len = 0;
    return true;
}
bool add_title(push_message* msg, const char* title){
    uint8_t* ptr = msg->dynamicData.data + msg->dynamicData.len;
    if((msg->dynamicData.len += strlen(title)+2) > NABTO_PUSH_BUFFER_ELEMENT_SIZE){
        return false;
    }
    WRITE_FORWARD_U8(ptr, NP_PAYLOAD_PUSH_DATA_VALUE_TITLE);
    WRITE_FORWARD_U8(ptr, strlen(title)+2);
    strcpy((char *)ptr,title);
    return true;
}
bool add_body(push_message* msg, const char* body){
    uint8_t* ptr = msg->dynamicData.data + msg->dynamicData.len;
    if((msg->dynamicData.len += strlen(body)+2) > NABTO_PUSH_BUFFER_ELEMENT_SIZE){
        return false;
    }
    WRITE_FORWARD_U8(ptr, NP_PAYLOAD_PUSH_DATA_VALUE_BODY);
    WRITE_FORWARD_U8(ptr, strlen(body)+2);
    strcpy((char *)ptr,body);
    return true;
}
bool add_title_loc_key(push_message* msg, const char* titleKey){
    uint8_t* ptr = msg->dynamicData.data + msg->dynamicData.len;
    if((msg->dynamicData.len += strlen(titleKey)+2) > NABTO_PUSH_BUFFER_ELEMENT_SIZE){
        return false;
    }
    WRITE_FORWARD_U8(ptr, NP_PAYLOAD_PUSH_DATA_VALUE_TITLE_LOC_KEY);
    WRITE_FORWARD_U8(ptr, strlen(titleKey)+2);
    strcpy((char *)ptr,titleKey);
    return true;
}
bool add_title_loc_string_arg(push_message* msg, const char* titleArg){
    uint8_t* ptr = msg->dynamicData.data + msg->dynamicData.len;
    if((msg->dynamicData.len += strlen(titleArg)+2) > NABTO_PUSH_BUFFER_ELEMENT_SIZE){
        return false;
    }
    WRITE_FORWARD_U8(ptr, NP_PAYLOAD_PUSH_DATA_VALUE_TITLE_LOC_STRING_ARG);
    WRITE_FORWARD_U8(ptr, strlen(titleArg)+2);
    strcpy((char *)ptr,titleArg);
    return true;
}
bool add_body_loc_key(push_message* msg, const char* bodyKey){
    uint8_t* ptr = msg->dynamicData.data + msg->dynamicData.len;
    if((msg->dynamicData.len += strlen(bodyKey)+2) > NABTO_PUSH_BUFFER_ELEMENT_SIZE){
        return false;
    }
    WRITE_FORWARD_U8(ptr, NP_PAYLOAD_PUSH_DATA_VALUE_BODY_LOC_KEY);
    WRITE_FORWARD_U8(ptr, strlen(bodyKey)+2);
    strcpy((char *)ptr,bodyKey);
    return true;
}
bool add_body_loc_string_arg(push_message* msg, const char* bodyArg){
    uint8_t* ptr = msg->dynamicData.data + msg->dynamicData.len;
    if((msg->dynamicData.len += strlen(bodyArg)+2) > NABTO_PUSH_BUFFER_ELEMENT_SIZE){
        return false;
    }
    WRITE_FORWARD_U8(ptr, NP_PAYLOAD_PUSH_DATA_VALUE_BODY_LOC_STRING_ARG);
    WRITE_FORWARD_U8(ptr, strlen(bodyArg)+2);
    strcpy((char *)ptr,bodyArg);
    return true;
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
    return NULL;
}

void unabto_push_notification_callback(uint32_t seq, unabto_push_hint* hint){
    int i;
    for(i=0;i<dataHead;i++){
        if(buffer[i].seq == seq){
            buffer[i].cb(buffer[i].args,hint);
            memmove(&buffer[i].data, &buffer[dataHead-1].data,sizeof(buffer_element));
            dataHead--;
            return;
        }
    }   
}
