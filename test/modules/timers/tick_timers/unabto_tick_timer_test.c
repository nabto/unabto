#include <unabto/unabto_env_base.h>
#include <unabto/unabto_external_environment.h>
#include <modules/timers/tick_timer/unabto_tick_timer.h>
#include <stdio.h>
#include <limits.h>

// Just a simple sanity check for the tick timer.
// compile it as gcc -DTEST_MODE unabto_tick_timer_test.c

int main(int argc, char** argv) {
    int i;
    
    nabto_stamp_t initial = nabtoGetStamp();

    // test that going forward in time works as expected.
    for (i = 0; i < 100000; i++) {
        nabto_stamp_t stamp = nabtoGetStamp();
        uint32_t ticks = rand();
        ticks = ticks % (INT_MAX);
        unabto_tick_timer_tick(ticks);
        if (!nabtoIsStampPassed(&stamp)) {
            printf("test failed stamp: %i, ticks: %i", stamp, ticks);
            return 1;
        }

    }

    for (i = 0; i < 100000; i++) {
        unabto_tick_timer_tick(rand());
        nabto_stamp_t realstamp = nabtoGetStamp();
        uint32_t ticks = rand();
        nabto_stamp_t stamp;
        ticks = ticks % (INT_MAX);
        stamp = realstamp + ticks;
        if (nabtoIsStampPassed(&stamp)) {
            printf("test failed stamp: %i, ticks: %i\n", realstamp, ticks);
            return 1;
        }
    }
    
    {
        nabto_stamp_t stamp = nabtoGetStamp();
        if (nabtoIsStampPassed(&stamp)) {
            printf("stamp wrongly passed\n");
            return 1;
        }
        unabto_tick_timer_tick(1);
        if (!nabtoIsStampPassed(&stamp)) {
            printf("stamp wrongly not passed\n");
            return 1;
        }
    }
    printf("Tick timer test passed\n");
    return 0;
}
