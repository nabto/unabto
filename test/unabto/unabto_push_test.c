#include <unistd.h>
#include "unabto_push_test.h"
#if NABTO_ENABLE_PUSH
extern int pushSeqQHead;
extern void unabto_push_init(void);

//#ifndef UNABTO_PUSH_CALLBACK_FUNCTIONS
//#define UNABTO_PUSH_CALLBACK_FUNCTIONS 1
bool getDataCalled = false;
uint8_t* unabto_push_notification_get_data(uint8_t* bufStart, const uint8_t* bufEnd, uint32_t seq){
    NABTO_LOG_INFO(("Getting data"));
    getDataCalled = true;
    return bufStart;
}

bool callbackCalled = false;
void unabto_push_notification_callback(uint32_t seq, unabto_push_hint* hint){
    callbackCalled = true;
    if(hint == NULL) return;
    NABTO_LOG_INFO(("Push_notification_callback with seq: %i, hint: %i",seq,*hint));
}
//#endif


bool unabto_push_test(void){

    NABTO_LOG_INFO(("push test starting"));
    unabto_push_init();

    if (pushCtx.pushSeqQHead != 0){
        NABTO_LOG_INFO(("Queue not initialized to 0 elements"));
        return false;
    }

    uint32_t seq;
    unabto_push_hint ret = unabto_send_push_notification(1,&seq);
    if (ret != UNABTO_PUSH_HINT_OK){
        NABTO_LOG_INFO(("send push notification did not return OK"));
        return false;
    }
    if (pushCtx.pushSeqQHead != 1){
        NABTO_LOG_INFO(("Push notification not in push queue"));
        return false;
    }
    ret = unabto_send_push_notification(1,&seq);
    ret = unabto_send_push_notification(1,&seq);
    ret = unabto_send_push_notification(1,&seq);
    ret = unabto_send_push_notification(1,&seq);
    if (pushCtx.pushSeqQHead != 5){
        NABTO_LOG_INFO(("Push Queue does not contain 5 elements, it contains: %d",pushCtx.pushSeqQHead));
        return false;
    }
    // Manually ticking push events since we do not have a main loop
    nabto_time_event_push();
    nabto_time_event_push();
    nabto_time_event_push();
    nabto_time_event_push();
    nabto_time_event_push();
    nabto_time_event_push();
    nabto_time_event_push();
    nabto_time_event_push();
    nabto_time_event_push();
    if (!getDataCalled) {
        NABTO_LOG_INFO(("GetData function not called"));
        return false;
    }
    if (!callbackCalled) {
        NABTO_LOG_INFO(("Callback function not called"));
        return false;
    }
    NABTO_LOG_INFO(("Push test succeeded"));
    return true;
}
#endif //NABTO_ENABLE_PUSH
