/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#include "adc.h"

void adc_initialize(uint8_t portConfiguration)
{
  ADCON0 = 0x01; // Enable AD converter
  ADCON1 = portConfiguration;
  ADCON2 = (1 << 7) | (0b111 << 3) | 0b110; // Right justify, 20 Tad ACQ time, Fosc / 64 (8 Tad ACQ + 12 tAD conversion -> ~77 ksps)

  // Do a calibration A/D conversion
  ADCON0bits.ADCAL = 1;
  ADCON0bits.GO = 1;
  while(ADCON0bits.GO);
  ADCON0bits.ADCAL = 0;
}

uint16_t adc_read(uint8_t channel)
{
  ADCON0 &= ~(0b1111 << 2);
  ADCON0 |= channel << 2;

  ADCON0bits.GO = 1;
  while(ADCON0bits.GO);

  return ADRES;
}

void adc_begin_read(uint8_t channel)
{
  ADCON0 &= ~(0b1111 << 2);
  ADCON0 |= channel << 2;

  ADCON0bits.GO = 1;
}

bool adc_end_read(uint16_t* value)
{
  if(ADCON0bits.GO)
  {
    return false;
  }
  else
  {
    *value = ADRES;
    return true;
  }
}
