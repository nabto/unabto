/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_PERIPHERAL

#include "spi.h"

void spi_initialize(uint8_t clockDivider)
{
  TRISCbits.TRISC3 = 0;
  TRISCbits.TRISC4 = 1;
  TRISCbits.TRISC5 = 0;
  TRISFbits.TRISF7 = 0;
  SSP1CON1 = 0x20 | clockDivider;
  SSP1STAT = 0x40;
}

uint8_t spi_transfer(uint8_t value)
{
  PIR1bits.SSP1IF = 0;

  SSP1BUF = value;

  while(PIR1bits.SSP1IF == 0);

  return SSP1BUF;
}

void spi_transfer_buffer(uint8_t* buffer, uint16_t length)
{
  while(length-- > 0)
  {
    PIR1bits.SSP1IF = 0;

    SSP1BUF = *buffer;

    while(PIR1bits.SSP1IF == 0);

    *buffer = SSP1BUF;

    buffer++;
  }
}
