#include <Nano1X2Series.h>
#include "systimer.h"

static volatile uint32_t tick_cur = 0;
static uint32_t tick_indic = 0;

void SysTick_Handler(void)
{
    tick_cur ++;
}

void systimer_init(void)
{
    tick_cur = 0;
    SysTick_Config(CLOCKS_PER_TICK);
}

uint32_t systimer_tick(void)
{
    return tick_cur;
}

void systimer_delay_ms(uint32_t ms)
{
    systimer_set_indic(systimer_tick());
    systimer_delay_indic(ms * TICKS_PER_SEC / 1000);
}

void systimer_set_indic(uint32_t tick_indic_)
{
    tick_indic = tick_indic_;
}

/**
  * @brief          Delay specified ticks relative to last indicated mark.
  */
void systimer_delay_indic(uint32_t n_tick)
{
    uint32_t tick_start = tick_indic;
    while (1) {
        uint32_t tick_end = systimer_tick();
        uint32_t tick_elapsed;
        if (tick_end >= tick_start) {
            tick_elapsed = tick_end - tick_start;
        }
        else {  // Take wrap-around case into consideration.
            tick_elapsed = 0xFFFFFFFF - tick_start + tick_end + 1;
        }
        if (tick_elapsed >= n_tick) {
            break;
        }
    }
}
