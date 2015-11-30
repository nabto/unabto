/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _ADC_H_
#define _ADC_H_

#include <unabto_platform_types.h>

#ifdef __cplusplus
extern "C" {
#endif

void adc_initialize(uint8_t portConfiguration);
uint16_t adc_read(uint8_t channel);
void adc_begin_read(uint8_t channel);
bool adc_end_read(uint16_t* value);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
