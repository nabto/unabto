#define UNABTO_PUSH_CALLBACK_FUNCTIONS
#include <unistd.h>
#include "unabto_push_test.h"
#if NABTO_ENABLE_PUSH
extern int pushSeqQHead;
extern void unabto_push_init(void);

//#ifndef UNABTO_PUSH_CALLBACK_FUNCTIONS
//#define UNABTO_PUSH_CALLBACK_FUNCTIONS 1
uint8_t* unabto_push_notification_get_data(uint8_t* bufStart, const uint8_t* bufEnd, uint32_t seq){
    NABTO_LOG_INFO(("Getting data"));
    return bufStart;
}

void unabto_push_notification_callback(uint32_t seq, unabto_push_hint* hint){
    if(hint == NULL) return;
    NABTO_LOG_INFO(("Push_notification_callback with seq: %i, hint: %i",seq,*hint));
}
//#endif


bool unabto_push_test(void){

    NABTO_LOG_INFO(("push test starting"));
    unabto_push_init();

    if (pushCtx.pushSeqQHead != 0){
        return false;
    }

    int seq;
    unabto_push_hint ret = unabto_send_push_notification(1,&seq);
    if (ret != UNABTO_PUSH_HINT_OK){
        return false;
    }
    if (pushCtx.pushSeqQHead != 1){
        return false;
    }
    ret = unabto_send_push_notification(1,&seq);
    ret = unabto_send_push_notification(1,&seq);
    ret = unabto_send_push_notification(1,&seq);
    ret = unabto_send_push_notification(1,&seq);
    nabto_time_event_push();
    nabto_time_event_push();
    nabto_time_event_push();
    nabto_time_event_push();
    nabto_time_event_push();
    nabto_time_event_push();
    nabto_time_event_push();
    nabto_time_event_push();
    sleep(4);
    nabto_time_event_push();
    if (pushCtx.pushSeqQHead != 5){
        return false;
    }
    bool r = unabto_push_notification_remove(seq);
    if (!r){
        return false;
    }
    if (pushCtx.pushSeqQHead != 4){
        return false;
    }
    r = unabto_push_notification_remove(seq-3);
    if (!r){
        return false;
    }

    if (pushCtx.pushSeqQHead != 3){
        return false;
    }
    
    return true;
}
#endif //NABTO_ENABLE_PUSH