

#include "unabto_push.h"
#include "unabto_packet_util.h"
#include "unabto_main_contexts.h"
#include "unabto_memory.h"

#define UNABTO_PUSH_DATA_SIZE nabtoCommunicationBufferSize-35-16

unabto_push_element pushSeqQ[NABTO_PUSH_QUEUE_LENGTH];

int pushSeqQHead = 0;
int pushSeqQTail = 0;
//unabto_push_element* pushSeqQEnd = pushSeqQ+NABTO_PUSH_QUEUE_LENGTH;
uint32_t nextSeq = 0;

void unabto_push_init(void){
    for (size_t i = 0; i<NABTO_PUSH_QUEUE_LENGTH; i++){
        pushSeqQ[i].state = IDLE;
    }
}

unabto_push_hint unabto_send_push_notification(uint16_t pnsId, uint32_t* seq){
    // Called by the developer
    // Construct a unabto_push_element for the PN
    // if queue full: return UNABTO_PUSH_HINT_QUEUE_FULL
    // start asyncronous setup of the packet
    // return: UNABTO_PUSH_HINT_OK

    // HOW TO GET THIS VALUE?
    nabto_stamp_t* g;

    if (pushSeqQ[pushSeqQHead].state != IDLE){
        return UNABTO_PUSH_HINT_QUEUE_FULL;
    }
    pushSeqQ[pushSeqQHead].seq = nextSeq;
    *seq = nextSeq;
    nextSeq++;
    pushSeqQ[pushSeqQHead].retrans = 0;
    pushSeqQ[pushSeqQHead].state = WAITING_TO_BE_SENT;
    pushSeqQ[pushSeqQHead].stamp = *g;// How to get current nabto_stamp_t
    pushSeqQHead = (pushSeqQHead == NABTO_PUSH_QUEUE_LENGTH ? 0 : pushSeqQHead+1);
    

    //

    return UNABTO_PUSH_HINT_OK;


}

void unabto_push_notification_remove(uint32_t seq)
{
    // remove unabto_push_element from queue
    
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

    // Not sure of the format here I guess it should get a time stamp as argument
    // Is there an "addHandler" function where this should be registered
    
}

/*
    uint8_t* buf = nabtoCommunicationBuffer;
    uint8_t* end = nabtoCommunicationBuffer + nabtoCommunicationBufferSize;

    uint8_t* ptr = insert_header(buf, CPNSI, nmc.context.gspnsi, U_PUSH, false, HEADER_SEQ, 0, NSICO);
    
    ptr = insert_payload(ptr, NP_PAYLOAD_TYPE_PUSH, 0, dataLength+NP_PAYLOAD_HDR_BYTELENGTH);
*/
    
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
