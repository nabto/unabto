#include "unabto_push_test.h"
#if NABTO_ENABLE_PUSH
extern int pushSeqQHead;
extern void unabto_push_init(void);
/*
#ifndef UNABTO_PUSH_CALLBACK_FUNCTIONS
#define UNABTO_PUSH_CALLBACK_FUNCTIONS 1
uint16_t* unabto_push_notification_get_data(const uint8_t* bufStart, const uint8_t* bufEnd, uint32_t seq){

}

void unabto_push_notification_callback(uint32_t seq, unabto_push_hint* hint){

}
#endif
*/
bool unabto_push_test(void){

    unabto_push_init();

    if (pushSeqQHead != 0){
        return false;
    }

    int seq;
    unabto_push_hint ret = unabto_send_push_notification(1,&seq);
    if (ret != UNABTO_PUSH_HINT_OK){
        return false;
    }
    if (pushSeqQHead != 1){
        return false;
    }
    ret = unabto_send_push_notification(1,&seq);
    ret = unabto_send_push_notification(1,&seq);
    ret = unabto_send_push_notification(1,&seq);
    ret = unabto_send_push_notification(1,&seq);
    if (pushSeqQHead != 5){
        return false;
    }

    
    return true;
}
#endif //NABTO_ENABLE_PUSH
