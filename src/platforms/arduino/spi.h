#ifndef _SPI_H_
#define _SPI_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <unabto/unabto_env_base.h>

#ifdef __cplusplus
extern "C" {
#endif

enum
{
  SPI_CLOCK_DIVIDER_4,
  SPI_CLOCK_DIVIDER_16,
  SPI_CLOCK_DIVIDER_64
};

void spi_initialize();
uint8_t spi_transfer(uint8_t value);
void spi_transfer_buffer(uint8_t* buffer, uint16_t length);
void setClockDivider(uint8_t rate);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
