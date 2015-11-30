/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _SPI_H_
#define _SPI_H_

#include <unabto_platform_types.h>

#ifdef __cplusplus
extern "C" {
#endif

enum
{
  SPI_CLOCK_DIVIDER_4,
  SPI_CLOCK_DIVIDER_16,
  SPI_CLOCK_DIVIDER_64
};

void spi_initialize(uint8_t clockDivider);
uint8_t spi_transfer(uint8_t value);
void spi_transfer_buffer(uint8_t* buffer, uint16_t length);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
