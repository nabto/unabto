#ifndef __SYSTIMER_H__
#define __SYSTIMER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define CLOCKS_PER_TICK   (SystemCoreClock / 1000)
#define TICKS_PER_SEC (SystemCoreClock / CLOCKS_PER_TICK)

void systimer_init(void);
uint32_t systimer_tick(void);
void systimer_delay_ms(uint32_t ms);
void systimer_set_indic(uint32_t tick_indic);
void systimer_delay_indic(uint32_t n_tick);

#ifdef __cplusplus
}
#endif

#endif //__SYSTIMER_H__
