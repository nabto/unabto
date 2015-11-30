/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _SIGNAL_GENERATOR_H_
#define _SIGNAL_GENERATOR_H_

#include <unabto/unabto_env_base.h>

#ifdef __cplusplus
extern "C" {
#endif

void signal_generator_tick(void);

// frequency is in milli Hertz!
void signal_generator_set_frequency(uint32_t frequency);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
