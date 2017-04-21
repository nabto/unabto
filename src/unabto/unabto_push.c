
#include "unabto_push.h"

#if NABTO_ENABLE_PUSH
#include "unabto_main_contexts.h"
#include "unabto_util.h"
#include "unabto_memory.h"
#include "unabto_protocol_defines.h"
#include "unabto_packet.h"


#ifndef UNABTO_PUSH_MIN_SEND_INTERVAL
#define UNABTO_PUSH_MIN_SEND_INTERVAL 100
#endif

#ifndef UNABTO_PUSH_QUOTA_EXCEEDED_BACKOFF
#define UNABTO_PUSH_QUOTA_EXCEEDED_BACKOFF 60000ul //ms (1 minute)
#endif

/* ---------------------------------------------------- *
 * Help function definitions                            *
 * ---------------------------------------------------- */
void unabto_push_create_and_send_packet(unabto_push_element *elem);
void unabto_push_set_next_event(void);
bool unabto_push_verify_integrity(nabto_packet_header* header);
bool unabto_push_seq_exists(uint32_t seq);

/* ---------------------------------------------------- *
 * Push global state variables                          *
 * ---------------------------------------------------- */

unabto_push_element pushSeqQ[NABTO_PUSH_QUEUE_LENGTH];
/* -------------------------------------------------------*
 * initialization function to be called before using push *
 * -------------------------------------------------------*/
void unabto_push_init(void){
    pushCtx.nextPushEvent = NULL;
    pushCtx.pushSeqQHead = 0;
    pushCtx.nextSeq = 0;
    pushCtx.reattachNeeded = false;
    pushCtx.lastSent = nabtoGetStamp();
    pushCtx.backOffLimit = pushCtx.lastSent;
    size_t i;
    for (i = 0; i<NABTO_PUSH_QUEUE_LENGTH; i++){
        pushSeqQ[i].state = UNABTO_PUSH_IDLE;
        pushSeqQ[i].hint = UNABTO_PUSH_HINT_OK;
    }
}

unabto_push_hint unabto_send_push_notification(uint16_t pnsId, uint32_t* seq){
    nabto_stamp_t now = nabtoGetStamp();
    // With reattach needed or queue full we must still call callback without releasing Zalgo
    if(pushCtx.reattachNeeded){
        pushSeqQ[pushCtx.pushSeqQHead].hint = UNABTO_PUSH_HINT_QUOTA_EXCEEDED_REATTACH;
    } else if (pushCtx.pushSeqQHead >= NABTO_PUSH_QUEUE_LENGTH){
        pushSeqQ[pushCtx.pushSeqQHead].hint = UNABTO_PUSH_HINT_QUEUE_FULL;
    } else {
        pushSeqQ[pushCtx.pushSeqQHead].hint = UNABTO_PUSH_HINT_OK;
    }
    pushSeqQ[pushCtx.pushSeqQHead].seq = pushCtx.nextSeq;
    *seq = pushCtx.nextSeq;
    pushCtx.nextSeq++;
    pushSeqQ[pushCtx.pushSeqQHead].retrans = 0;
    pushSeqQ[pushCtx.pushSeqQHead].state = UNABTO_PUSH_WAITING_SEND;
    if(nabtoStampLess(&now, &pushCtx.backOffLimit)){
        pushSeqQ[pushCtx.pushSeqQHead].stamp = pushCtx.backOffLimit;
    } else {
        pushSeqQ[pushCtx.pushSeqQHead].stamp = now;
    }        
    pushSeqQ[pushCtx.pushSeqQHead].pnsId = pnsId;

    pushCtx.pushSeqQHead++;
    unabto_push_set_next_event();
    return pushSeqQ[pushCtx.pushSeqQHead].hint;
}

bool unabto_push_notification_remove(uint32_t seq)
{
    int i;
    for(i = 0; i<pushCtx.pushSeqQHead; i++){
        if(pushSeqQ[i].seq == seq){
            memmove(&pushSeqQ[i],&pushSeqQ[pushCtx.pushSeqQHead-1],sizeof(unabto_push_element));
            pushCtx.pushSeqQHead--;
            pushSeqQ[pushCtx.pushSeqQHead].state = UNABTO_PUSH_IDLE;
            unabto_push_set_next_event();
            return true;
        }
    }
    return false;
    
}

uint16_t unabto_push_notification_data_size()
{
    return nabtoCommunicationBufferSize
        -NP_PACKET_HDR_MIN_BYTELENGTH
        -NP_PAYLOAD_PUSH_BYTELENGTH
        -NP_PAYLOAD_CRYPTO_BYTELENGTH
        -NP_PAYLOAD_VERIFY_BYTELENGTH;
}

void nabto_time_event_push(void)
{
//    NABTO_LOG_TRACE(("Push Next event called"));
    nabto_stamp_t now = nabtoGetStamp();
    if (!pushCtx.nextPushEvent){
//        NABTO_LOG_TRACE(("No Next event"));
        return;
    }
    if (!nabtoStampLess(&pushCtx.nextPushEvent->stamp,&now)){
//        NABTO_LOG_TRACE(("Next event is not ready"));
        return;
    } else {
        NABTO_LOG_TRACE(("Invoking Event"));
        unabto_push_create_and_send_packet(pushCtx.nextPushEvent);
        unabto_push_set_next_event();
    }
    
}

bool nabto_push_event(nabto_packet_header* hdr){
    const uint8_t* begin = nabtoCommunicationBuffer + hdr->hlen;
    const uint8_t* end = nabtoCommunicationBuffer + hdr->len;
    struct unabto_payload_push pushData;
    unabto_push_hint hint;

    if(!unabto_push_verify_integrity(hdr)){
        return false;
    }

    {
        struct unabto_payload_packet pushPayload;
        if(!unabto_find_payload(begin,end,NP_PAYLOAD_TYPE_PUSH, &pushPayload)) {
            NABTO_LOG_ERROR(("Missing push payload in push response"));
            return false;
        }

        if (!unabto_payload_read_push(&pushPayload, &pushData)) {
            NABTO_LOG_ERROR(("Cannot parse push payload"));
            return false;
        }
    }
    if(!unabto_push_seq_exists(pushData.sequence)){
        NABTO_LOG_ERROR(("Push packet with unknown sequence number received. Sequence number: %i",pushData.sequence));
        return false;
    }
    if (pushData.flags & NP_PAYLOAD_PUSH_FLAG_ACK){
        hint = UNABTO_PUSH_HINT_OK;
    }

    if (pushData.flags & NP_PAYLOAD_PUSH_FLAG_FAIL){
        hint = UNABTO_PUSH_HINT_FAILED;
    }
    
    if (pushData.flags & NP_PAYLOAD_PUSH_FLAG_QUOTA_EXCEEDED){
        int i;
        for (i = 0; i<pushCtx.pushSeqQHead; i++){
            nabtoAddStamp(&pushSeqQ[i].stamp,UNABTO_PUSH_QUOTA_EXCEEDED_BACKOFF);
        }
        nabtoSetFutureStamp(&pushCtx.backOffLimit,UNABTO_PUSH_QUOTA_EXCEEDED_BACKOFF);
    }
    
    if (pushData.flags & NP_PAYLOAD_PUSH_FLAG_QUOTA_EXCEEDED_REATTACH){
        pushCtx.reattachNeeded = true;
    }
    unabto_push_notification_callback(pushData.sequence, &hint);
    unabto_push_notification_remove(pushData.sequence);
    
    return true;
}

void unabto_push_notify_reattach(void){
    pushCtx.reattachNeeded = false;
}


////////////////////////////
// Local helper functions //
////////////////////////////
bool unabto_push_seq_exists(uint32_t seq){
    size_t i;
    for (i=0; i<pushCtx.pushSeqQHead; i++){
        if(pushSeqQ[i].seq == seq){
            return true;
        }
    }
    return false;
}

void unabto_push_create_and_send_packet(unabto_push_element *elem){
    uint8_t* buf = nabtoCommunicationBuffer;
    uint8_t* end = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;

    if(elem->retrans >= 8){
        unabto_push_hint hint = UNABTO_PUSH_HINT_FAILED;
        unabto_push_notification_callback(elem->seq,&hint);
        unabto_push_notification_remove(elem->seq);
        return;
    } else if (elem->hint == UNABTO_PUSH_HINT_QUOTA_EXCEEDED){
        unabto_push_notification_callback(elem->seq,&elem->hint);
        unabto_push_notification_remove(elem->seq);
        return;
    } else if (elem->hint == UNABTO_PUSH_HINT_QUOTA_EXCEEDED_REATTACH){
        unabto_push_notification_callback(elem->seq,&elem->hint);
        unabto_push_notification_remove(elem->seq);
        return;
    }
        
    
    uint8_t* ptr = insert_header(buf,0, nmc.context.gspnsi, U_PUSH, false, 0, 0, NULL);
    uint8_t* cryptHdrEnd;
    uint8_t* cryptDataStart;

    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_PUSH, 0, NP_PAYLOAD_PUSH_BYTELENGTH-NP_PAYLOAD_HDR_BYTELENGTH);
    WRITE_FORWARD_U32(ptr, elem->seq);
    WRITE_FORWARD_U16(ptr, elem->pnsId);
    WRITE_FORWARD_U8(ptr, NP_PAYLOAD_PUSH_FLAG_SEND);
    ptr = insert_payload(ptr, end, NP_PAYLOAD_TYPE_CRYPTO, 0, 0);
    WRITE_U8((ptr-3), NP_PAYLOAD_HDR_FLAG_NONE | NP_PAYLOAD_CRYPTO_HEADER_FLAG_PAYLOADS);
    cryptHdrEnd = ptr;

    cryptDataStart = ptr;
    ptr = unabto_push_notification_get_data(ptr, end-NP_PAYLOAD_VERIFY_BYTELENGTH, elem->seq);
    if(ptr > end-NP_PAYLOAD_VERIFY_BYTELENGTH || ptr < cryptDataStart){
        unabto_push_hint hint = UNABTO_PUSH_HINT_INVALID_DATA_PROVIDED;
        unabto_push_notification_callback(elem->seq,&hint);
        unabto_push_notification_remove(elem->seq);
        return;
    }
    if (nmc.context.cryptoAttach == NULL){
        unabto_push_hint hint = UNABTO_PUSH_HINT_NO_CRYPTO_CONTEXT;
        unabto_push_notification_callback(elem->seq,&hint);
        unabto_push_notification_remove(elem->seq);
        return;
    }
    if(!send_and_encrypt_packet(&nmc.context.gsp, nmc.context.cryptoConnect, cryptDataStart, ptr-cryptDataStart, cryptHdrEnd)){
        unabto_push_hint hint = UNABTO_PUSH_HINT_FAILED;
        unabto_push_notification_callback(elem->seq,&hint);
        unabto_push_notification_remove(elem->seq);
        return;
    }        

    pushCtx.lastSent = nabtoGetStamp();
    elem->retrans++;
    nabtoSetFutureStamp(&elem->stamp,(2^elem->retrans)*1000);
    unabto_push_set_next_event();
}


void unabto_push_set_next_event(void){
    // Find the lowest stamp in the queue
    // if no elements in queue set next event = NULL;
    if (pushCtx.pushSeqQHead == 0){
        pushCtx.nextPushEvent = NULL;
        return;
    } else {
        pushCtx.nextPushEvent = &pushSeqQ[0];
    }
    int i;
    for (i = 0; i<pushCtx.pushSeqQHead; i++){
        if(nabtoStampLess(&pushSeqQ[i].stamp,&pushCtx.nextPushEvent->stamp)){
            pushCtx.nextPushEvent = &pushSeqQ[i];
        }
    }
    if (nabtoStampLess(&pushCtx.nextPushEvent->stamp,&pushCtx.lastSent)){
        nabtoAddStamp(&pushCtx.nextPushEvent->stamp, UNABTO_PUSH_MIN_SEND_INTERVAL);
    }
}


bool unabto_push_verify_integrity(nabto_packet_header* header){
    uint8_t* buf = nabtoCommunicationBuffer;
    uint8_t* end = nabtoCommunicationBuffer+nabtoCommunicationBufferSize;
    struct unabto_payload_crypto crypto;

    buf += header->hlen;

    {
        struct unabto_payload_packet payload;
        if (!unabto_find_payload(buf, end, NP_PAYLOAD_TYPE_CRYPTO, &payload)) {
            NABTO_LOG_ERROR(("No crypto payload in debug packet."));
            return false;
        }

        if (!unabto_payload_read_crypto(&payload, &crypto)) {
            NABTO_LOG_ERROR(("Crypto packet too short."));
            return false;
        }
    }
    {
        uint16_t verifSize;
        if (!unabto_verify_integrity(nmc.context.cryptoConnect, crypto.code, nabtoCommunicationBuffer, header->len, &verifSize)) {
            NABTO_LOG_DEBUG(("U_DEBUG Integrity verification failed"));
            return false;
        }
    }
    return true;

}


#endif //NABTO_ENABLE_PUSH
