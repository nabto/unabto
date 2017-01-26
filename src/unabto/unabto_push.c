
#include "unabto_push.h"

#if NABTO_ENABLE_PUSH
#include "unabto_packet_util.h"
#include "unabto_main_contexts.h"
#include "unabto_util.h"
#include "unabto_memory.h"
#include "unabto_protocol_defines.h"

// WHAT HEADER LENGHT SHOULD BE USED, IS NSI.co USED HERE?
#define UNABTO_PUSH_DATA_SIZE nabtoCommunicationBufferSize-NP_PACKET_HDR_MIN_BYTELENGTH-NP_PAYLOAD_PUSH_BYTELENGTH-NP_PAYLOAD_CRYPTO_BYTELENGTH-NP_PAYLOAD_PUSH_DATA_SIZE_WO_DATA-16

#ifndef UNABTO_PUSH_RETRANS_CHECK_INTERVAL
#define UNABTO_PUSH_RETRANS_CHECK_INTERVAL 500
#endif

#ifndef UNABTO_PUSH_MIN_SEND_INTERVAL
#define UNABTO_PUSH_MIN_SEND_INTERVAL 500
#endif

#ifndef UNABTO_PUSH_QUOTA_EXCEEDED_BACKOFF
#define UNABTO_PUSH_QUOTA_EXCEEDED_BACKOFF 60000 //ms (1 minute)
#endif

/* ---------------------------------------------------- *
 * These functions must be implemented by the developer *
 * ---------------------------------------------------- */
// THIS DOESN'T WORK FIX LATER IMPLEMENT IN unabto_push_test.c
/*
#ifndef UNABTO_PUSH_CALLBACK_FUNCTIONS
#define UNABTO_PUSH_CALLBACK_FUNCTIONS 1
uint8_t* unabto_push_notification_get_data(uint8_t* bufStart, const uint8_t* bufEnd, uint32_t seq){return bufStart;}
void unabto_push_notification_callback(uint32_t seq, unabto_push_hint* hint){}
#endif
*/
// THIS WORKS BUT SEEMS LIKE AN UGLY SOLUTION
#ifndef UNABTO_PUSH_CALLBACK_FUNCTIONS
#define UNABTO_PUSH_CALLBACK_FUNCTIONS
#define unabto_push_notification_get_data(bufStart,bufEnd,seq) unabto_push_get_data_mock(bufStart,bufEnd, seq)
#define unabto_push_notification_callback(seq,hint) unabto_push_callback_mock(seq, hint)
uint8_t* unabto_push_get_data_mock(uint8_t* bufStart, const uint8_t* bufEnd, uint32_t seq){return bufStart;}
void unabto_push_callback_mock(uint32_t seq, unabto_push_hint* hint){}
#endif

/* ---------------------------------------------------- *
 * Help function definitions                            *
 * ---------------------------------------------------- */
void unabto_push_create_and_send_packet(unabto_push_element *elem);
void unabto_push_set_next_event(void);
bool unabto_push_verify_integrity(nabto_packet_header* header);
bool unabto_push_reattach_needed(void);

// I found no other unabto core functionallities which uses global variables like this
// how else should context be kept? Added to main context?
/* ---------------------------------------------------- *
 * Push global state variables                          *
 * ---------------------------------------------------- */
unabto_push_element pushSeqQ[NABTO_PUSH_QUEUE_LENGTH];
unabto_push_element* nextPushEvent;
int pushSeqQHead = 0;
uint32_t nextSeq = 0;
nabto_stamp_t lastSent;
nabto_stamp_t backOffLimit;
bool reattachNeeded = false;

/* -------------------------------------------------------*
 * initialization function to be called before using push *
 * -------------------------------------------------------*/
void unabto_push_init(void){
    nextPushEvent = NULL;
    lastSent = nabtoGetStamp();
    backOffLimit = lastSent;
    
    for (size_t i = 0; i<NABTO_PUSH_QUEUE_LENGTH; i++){
        pushSeqQ[i].state = UNABTO_PUSH_IDLE;
    }
}

unabto_push_hint unabto_send_push_notification(uint16_t pnsId, uint32_t* seq){
    // Called by the developer
    // Construct a unabto_push_element for the PN
    // if queue full: return UNABTO_PUSH_HINT_QUEUE_FULL
    // start asyncronous setup of the packet
    // return: UNABTO_PUSH_HINT_OK
    if(unabto_push_reattach_needed()){
        return UNABTO_PUSH_HINT_QUOTA_EXCEEDED_REATTACH;
    }

    nabto_stamp_t now = nabtoGetStamp();
    if (pushSeqQHead >= NABTO_PUSH_QUEUE_LENGTH){
        return UNABTO_PUSH_HINT_QUEUE_FULL;
    }
    pushSeqQ[pushSeqQHead].seq = nextSeq;
    *seq = nextSeq;
    nextSeq++;
    pushSeqQ[pushSeqQHead].retrans = 0;
    pushSeqQ[pushSeqQHead].state = UNABTO_PUSH_WAITING_SEND;
    if(nabtoStampLess(&now, &backOffLimit)){
        pushSeqQ[pushSeqQHead].stamp = backOffLimit;
    } else {
        pushSeqQ[pushSeqQHead].stamp = now;
    }        
    pushSeqQ[pushSeqQHead].pnsId = pnsId;

    pushSeqQHead++;
    unabto_push_set_next_event();
    return UNABTO_PUSH_HINT_OK;
}

bool unabto_push_notification_remove(uint32_t seq)
{
    // remove unabto_push_element from queue
    for(int i = 0; i<pushSeqQHead; i++){
        if(pushSeqQ[i].seq == seq){
            memmove(&pushSeqQ[i],&pushSeqQ[i+1],pushSeqQHead-i);
            pushSeqQ[pushSeqQHead-1].state = UNABTO_PUSH_IDLE;
            pushSeqQHead--;
            unabto_push_set_next_event();
            return true;
        }
    }
    return false;
    
}

uint16_t unabto_push_notification_data_size()
{
    return UNABTO_PUSH_DATA_SIZE;
}

void nabto_time_event_push(void)
{
    // Called every tick by the core
    // Should check if it is time for next event
    // if so run it
    // else return

    nabto_stamp_t now = nabtoGetStamp();
    if (!nextPushEvent){
        return;
    }
    if (!nabtoStampLess(&nextPushEvent->stamp,&now)){
        return;
    } else {
        unabto_push_create_and_send_packet(nextPushEvent);
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
    if (pushData.flags & NP_PAYLOAD_PUSH_FLAG_ACK){
        unabto_push_notification_remove(pushData.sequence);
        hint = UNABTO_PUSH_HINT_OK;
    }

    if (pushData.flags & NP_PAYLOAD_PUSH_FLAG_FAIL){
        unabto_push_notification_remove(pushData.sequence);
        hint = UNABTO_PUSH_HINT_FAILED;
    }
    
    if (pushData.flags & NP_PAYLOAD_PUSH_FLAG_QUOTA_EXCEEDED){
        for (int i = 0; i<pushSeqQHead; i++){
            nabtoAddStamp(&pushSeqQ[i].stamp,UNABTO_PUSH_QUOTA_EXCEEDED_BACKOFF);
        }
        nabtoSetFutureStamp(&backOffLimit,UNABTO_PUSH_QUOTA_EXCEEDED_BACKOFF);
    }
    
    if (pushData.flags & NP_PAYLOAD_PUSH_FLAG_QUOTA_EXCEEDED_REATTACH){
        // handle quota exceeded reattach, need som attach context
        reattachNeeded = true;
        
    }
    unabto_push_notification_callback(pushData.sequence, &hint);
    
    return true;
}



////////////////////////////
// Local helper functions //
////////////////////////////
bool unabto_push_reattach_needed(void){
    // Must check if the device has reattached since QUOTA_EXCEEDED_REATTACH was last received
    reattachNeeded = false;
    return false;
}

void unabto_push_create_and_send_packet(unabto_push_element *elem){
    uint8_t* buf = nabtoCommunicationBuffer;
    uint8_t* end = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;
    // need to get value for arg[1,5,7] cpnsi, seq, and nsico
    uint32_t cpnsi =11;
    uint16_t hdrseq = 11;
    uint8_t* nsico = 0;

    if(elem->retrans >= 8){
        unabto_push_hint hint = UNABTO_PUSH_HINT_FAILED;
        unabto_push_notification_callback(elem->seq,&hint);
        unabto_push_notification_remove(elem->seq);
        unabto_push_set_next_event();
       return;
    }
    
    bool retrans = elem->retrans;
    uint8_t* ptr = insert_header(buf, cpnsi, nmc.context.gspnsi, U_PUSH, false, hdrseq, 0, nsico);

    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_PUSH, 0, NP_PAYLOAD_PUSH_BYTELENGTH-NP_PAYLOAD_HDR_BYTELENGTH);
    WRITE_U32(ptr, elem->seq); ptr += 4;
    WRITE_U16(ptr, elem->pnsId); ptr += 2;
    WRITE_U8(ptr, NP_PAYLOAD_PUSH_FLAG_SEND); ptr++;

    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_CRYPTO, 0, 0);
    // IS THERE A DEFINE FOR VERIFICATION DATA LENGHT? as to not use 16
    ptr = unabto_push_notification_get_data(ptr, end-16, elem->seq);
    if(ptr > end-16){
        unabto_push_hint hint = UNABTO_PUSH_HINT_INVALID_DATA_PROVIDED;
        unabto_push_notification_callback(elem->seq,&hint);
        unabto_push_notification_remove(elem->seq);
        unabto_push_set_next_event();
        return;
    }
    
    // Encrypt and make verification field
    /*
    uint16_t len;
    if(!encrypt_packet(nmc.context.cryptoAttach,plaintext,sizeof(plaintext), cryptoPayloadStart,&len)){
        unabto_push_notification_callback(elem->seq,UNABTO_PUSH_HINT_ENCRYPTION_FAILED);
        unabto_push_notification_remove(elem->seq);
        return;
    }
    */
    // set length in crypto header
    // set length in header
    // send packet

    lastSent = nabtoGetStamp();
    elem->retrans++;
    nabtoSetFutureStamp(&elem->stamp,(2^elem->retrans)*1000);
    unabto_push_set_next_event();
}

/*
    uint8_t tmp[NONCE_SIZE + SEED_SIZE];

    memcpy(tmp, nonceGSP, NONCE_SIZE);
    nabto_random(tmp + NONCE_SIZE, SEED_SIZE);
    unabto_crypto_reinit_c(nonceGSP, tmp + NONCE_SIZE, seedGSP);

    send_and_encrypt_packet(&nmc.context.gsp, nmc.context.cryptoAttach, tmp, sizeof(tmp), cryptoPayloadStart);
*/

void unabto_push_set_next_event(void){
    // Find the lowest stamp in the queue
    // if no elements in queue set next event = NULL;
    if (pushSeqQHead == 0){
        nextPushEvent = NULL;
        return;
    } else {
        nextPushEvent = &pushSeqQ[0];
    }
    for (int i = 0; i<pushSeqQHead; i++){
        if(nabtoStampLess(&pushSeqQ[i].stamp,&nextPushEvent->stamp)){
            nextPushEvent = &pushSeqQ[i];
        }
    }
    if (nabtoStampLess(&nextPushEvent->stamp,&lastSent)){
        nabtoAddStamp(&nextPushEvent->stamp, UNABTO_PUSH_MIN_SEND_INTERVAL);
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
