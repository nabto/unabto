
#include "unabto_push.h"

#if NABTO_ENABLE_PUSH
#include "unabto_packet_util.h"
#include "unabto_main_contexts.h"
#include "unabto_memory.h"
#include "unabto_protocol_defines.h"

#define UNABTO_PUSH_DATA_SIZE nabtoCommunicationBufferSize-35-16

#ifndef UNABTO_PUSH_RETRANS_CHECK_INTERVAL
#define UNABTO_PUSH_RETRANS_CHECK_INTERVAL 500
#endif

#ifndef UNABTO_PUSH_MIN_SEND_INTERVAL
#define UNABTO_PUSH_MIN_SEND_INTERVAL 500
#endif

/* ---------------------------------------------------- *
 * These functions must be implemented by the developer *
 * ---------------------------------------------------- */
// THIS DOESN'T WORK FIX LATER IMPLEMENT IN unabto_push_test.c
#ifndef UNABTO_PUSH_CALLBACK_FUNCTIONS
#define UNABTO_PUSH_CALLBACK_FUNCTIONS 1
extern uint16_t* unabto_push_notification_get_data(const uint8_t* bufStart, const uint8_t* bufEnd, uint32_t seq){}
extern void unabto_push_notification_callback(uint32_t seq, unabto_push_hint* hint){}
#endif

/* ---------------------------------------------------- *
 * Help function definitions                            *
 * ---------------------------------------------------- */
void unabto_push_create_and_send_packet(unabto_push_element *elem);
unabto_push_element* unabto_push_find_next_event();
bool unabto_push_verify_integrity(nabto_packet_header* header);

unabto_push_element pushSeqQ[NABTO_PUSH_QUEUE_LENGTH];
unabto_push_element* nextPushEvent;
int pushSeqQHead = 0;
uint32_t nextSeq = 0;
nabto_stamp_t lastSent;

void unabto_push_init(void){
    nextPushEvent = NULL;
    lastSent = nabtoGetStamp();
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

    nabto_stamp_t now = nabtoGetStamp();
    if (pushSeqQHead >= NABTO_PUSH_QUEUE_LENGTH){
        return UNABTO_PUSH_HINT_QUEUE_FULL;
    }
    pushSeqQ[pushSeqQHead].seq = nextSeq;
    *seq = nextSeq;
    nextSeq++;
    pushSeqQ[pushSeqQHead].retrans = 0;
    pushSeqQ[pushSeqQHead].state = UNABTO_PUSH_WAITING_SEND;
    pushSeqQ[pushSeqQHead].stamp = now;
    pushSeqQ[pushSeqQHead].pnsId = pnsId;

    pushSeqQHead++;
    nextPushEvent = unabto_push_find_next_event();
    return UNABTO_PUSH_HINT_OK;
}

void unabto_push_notification_remove(uint32_t seq)
{
    // remove unabto_push_element from queue
    for(int i = 0; i<pushSeqQHead; i++){
        if(pushSeqQ[i].seq == seq){
            memmove(&pushSeqQ[i],&pushSeqQ[i+1],pushSeqQHead-i);
            pushSeqQ[pushSeqQHead-1].state = UNABTO_PUSH_IDLE;
            pushSeqQHead--;
            nextPushEvent = unabto_push_find_next_event();
            return;
        }
    }
    
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
            
        nextPushEvent = unabto_push_find_next_event();
    }
    
}

bool nabto_push_event(nabto_packet_header* hdr){
    const uint8_t* begin = nabtoCommunicationBuffer + hdr->hlen;
    const uint8_t* end = nabtoCommunicationBuffer + hdr->len;
    struct unabto_payload_push pushData;
    unabto_push_hint hint;
    // Verify packet or is that done ?
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
        hint = UNABTO_PUSH_HINT_FAILED;
    }
    
    if (pushData.flags & NP_PAYLOAD_PUSH_FLAG_QUOTA_EXCEEDED){
        // handle quota exceeded
        hint = UNABTO_PUSH_HINT_QUOTA_EXCEEDED;
    }
    
    if (pushData.flags & NP_PAYLOAD_PUSH_FLAG_QUOTA_EXCEEDED_REATTACH){
        // handle quota exceeded reattach
        hint = UNABTO_PUSH_HINT_QUOTA_EXCEEDED_REATTACH;
    }
    unabto_push_notification_callback(pushData.sequence, &hint);
    
    return true;
}

/*
    uint8_t junk[] = {2,5,3,6,3}
    // TO BE DETERMINED:
    uint32_t CPNSI = 1;
    uint16_t HEADER_SEQ = 1;
    uint8_t NSICO = 1;
    const uint8_t* nonceGSP = junk;
    const uint8_t* seedGSP = junk;
    // END TO BE DETERMINED
    
    // MAKE SEQ NUMBER HANDLING HERE:
    uint32_t seq = 1;
    //


    uint8_t* buf = nabtoCommunicationBuffer;
    uint8_t* end = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;

    uint8_t* ptr = insert_header(buf, CPNSI, nmc.context.gspnsi, U_PUSH, false, HEADER_SEQ, 0, NSICO);
    uint16_t dataLength = lenCb+lenMb+10;
    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_CRYPTO, 0, dataLength+NP_PAYLOAD_HDR_BYTELENGTH);
    uint8_t* cryptoPayloadStart = ptr;
    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_PUSH, 0, dataLength);
    WRITE_U32(ptr, seq); ptr += 4;
    WRITE_U16(ptr, 0); ptr +=2; // Not using segmented packets for now
    WRITE_U16(ptr, 0); ptr +=2; // Not using segmented packets for now
    WRITE_U16(ptr, lenCb); ptr +=2;
    memcpy(ptr, confBuf, lenCb); ptr += lenCb;
    memcpy(ptr, msgBuf, lenMb); ptr += lenMb;

    uint8_t tmp[NONCE_SIZE + SEED_SIZE];

    memcpy(tmp, nonceGSP, NONCE_SIZE);
    nabto_random(tmp + NONCE_SIZE, SEED_SIZE);
    unabto_crypto_reinit_c(nonceGSP, tmp + NONCE_SIZE, seedGSP);

    send_and_encrypt_packet(&nmc.context.gsp, nmc.context.cryptoAttach, tmp, sizeof(tmp), cryptoPayloadStart);
*/



////////////////////////////
// Local helper functions //
////////////////////////////
void unabto_push_create_and_send_packet(unabto_push_element *elem){
    uint8_t* buf = nabtoCommunicationBuffer;
    uint8_t* end = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;

//    uint8_t* ptr = insert_header(buf, CPNSI, nmc.context.gspnsi, U_PUSH, false, HEADER_SEQ, 0, NSICO);
    
//    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_PUSH, 0, dataLength+NP_PAYLOAD_HDR_BYTELENGTH);
}

unabto_push_element* unabto_push_find_next_event(){
    // Find the lowest stamp in the queue
    // if no elements in queue return null
    for (int i = 0; i<pushSeqQHead; i++){
        
    }
    return NULL;
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
