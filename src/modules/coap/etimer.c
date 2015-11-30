#include <unabto/unabto.h>
#include "etimer.h"

void etimer_set(struct etimer *et, uint32_t interval)
{
    nabto_stamp_t t = nabtoGetStamp();
    memcpy(&et->timer.start, &t, sizeof(nabto_stamp_t));
    nabtoAddStamp(&et->timer.start, interval);
    et->timer.interval = interval;
}

void etimer_stop (struct etimer *et)
{
    et->timer.interval = 0;
}

int etimer_expired (struct etimer *et)
{
    nabto_stamp_t t;
    t = et->timer.start; 
    return nabtoIsStampPassed(&et->timer.start);
}

void etimer_restart (struct etimer *et)
{
    nabto_stamp_t t;
    t = et->timer.start; 
    nabtoSetFutureStamp(&et->timer.start, et->timer.interval);
}

void etimer_reset (struct etimer *et)
{
    nabto_stamp_t t;
    t = et->timer.start; 
    /* FIXME - we don't have any functions to retrieve the remaining time to epoch */
    nabtoSetFutureStamp(&et->timer.start, et->timer.interval);
}
