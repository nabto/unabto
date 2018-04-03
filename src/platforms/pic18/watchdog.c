/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include "../../unabto_env_base.h"

void watchdog_enable()
{
    ClrWdt();
    WDTCONbits.SWDTEN = 1;
    ClrWdt();
}

void watchdog_disable()
{
    WDTCONbits.SWDTEN = 0;
}

void watchdog_reset()
{
    ClrWdt();
}
