/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include <unabto/unabto_env_base.h>

#if EEPROM_24xx08

#include <iic.h>

#define EEPROM_PAGE_SIZE            16

static bool eeprom_page_write(uint8_t channel, uint32_t address, void* data, uint8_t length);

bool eeprom_read(uint8_t channel, uint32_t address, void* data, uint16_t length)
{
  uint8_t control[2];
  control[0] = (uint8_t) (0xa0 | ((address >> 7) & 0x06) | 0);
  control[1] = (uint8_t) address;

  iic_start(channel);

  if(iic_write(channel, control, sizeof (control)) == false)
  {
    iic_stop(channel);
    return false;
  }

  iic_repeated_start(channel);

  control[0] |= 1; // switch to read mode

  if(iic_write(channel, control, 1) == false)
  {
    iic_stop(channel);
    return false;
  }

  iic_read(channel, data, length);

  iic_stop(channel);

  return true;
}

bool eeprom_write(uint8_t channel, uint32_t address, const void* data, uint16_t length)
{
  uint8_t* p = (uint8_t*) data;

  while(length > 0)
  {
    uint8_t currentPageAddress = (uint8_t) (address & (EEPROM_PAGE_SIZE - 1));
    uint8_t currentLength = (uint8_t) (EEPROM_PAGE_SIZE - currentPageAddress); // calculate maximum number of bytes that can be written to this page from the current offset
    if(length < currentLength)
    {
      currentLength = (uint8_t) length;
    }

    if(eeprom_page_write(channel, address, p, currentLength) == false)
    {
      return false;
    }
    address += currentLength;
    p += currentLength;
    length -= currentLength;
  }

  return true;
}

static bool eeprom_page_write(uint8_t channel, uint32_t address, void* data, uint8_t length)
{
  uint8_t control[2];
  uint16_t timeout = 100;

  control[0] = (uint8_t) (0xa0 | ((address >> 7) & 0x06) | 0);
  control[1] = (uint8_t) address;

  iic_start(channel);

  if(iic_write(channel, control, sizeof (control)) == false)
  {
    iic_stop(channel);
    return false;
  }

  if(iic_write(channel, data, length) == false)
  {
    iic_stop(channel);
    return false;
  }

  iic_stop(channel);

  while(timeout--)
  {
    iic_start(channel);
    if(iic_write(channel, control, 1))
    {
      iic_stop(channel);
      return true;
    }
    iic_stop(channel);
  }

  return false;
}

#endif
