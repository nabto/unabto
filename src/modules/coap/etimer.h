#ifndef __ETIMER_H__
#define __ETIMER_H__
#include <stdint.h>

#include <unabto/unabto.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CLOCK_SECOND 1000

typedef uint32_t clock_time_t;

struct timer {
  nabto_stamp_t start;
  uint32_t interval;
};

struct etimer {
  struct timer timer;
  struct etimer *next;
};


void etimer_set(struct etimer *et, uint32_t interval);
void etimer_stop (struct etimer *et);
int etimer_expired (struct etimer *et);
void etimer_restart (struct etimer *et);
void etimer_reset (struct etimer *et);



#ifdef __cplusplus
} //extern "C"
#endif

#endif /* __ETIMER_H__ */
