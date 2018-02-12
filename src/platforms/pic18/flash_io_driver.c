/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include "flash_io_driver.h"

#if NABTO_FLASH_IO_DRIVER_ENABLE

#include <flash.h>
#include <bootloader_base.h>

#define TableReadPostIncrement() do { _asm TBLRDPOSTINC _endasm } while(0)
#define TableWritePostIncrement() do { _asm TBLWTPOSTINC _endasm } while(0)

static bool is_range_valid(far rom const void* address);
static bool blank_check_block(far rom const void* address);

bool flash_io_driver_erase_block(__ROM void* address)
{
  if(is_range_valid(address) == false)
  {
    return false;
  }

  // already blank?
  if(blank_check_block(address))
  {
    return true;
  }

  {
    critical_section_enter();

    // flash erase-block not blank so erase it now
    TBLPTR = (uint24_t) address;

    EECON1bits.FREE = 1;
    EECON1bits.WREN = 1;

    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;

    critical_section_exit();
  }

  // is it blank now?
  return blank_check_block(address);
}

bool flash_io_driver_write_block(__ROM void* address, const void* data)
{
  if(is_range_valid(address) == false)
  {
    return false;
  }

  {
    uint8_t i = FLASH_WRITE_BLOCK;
    uint8_t* d = (uint8_t*) data;

    critical_section_enter();

    // load holding registers
    TBLPTR = (uint24_t) address;

    while(i--)
    {
      TABLAT = *d++;
      TableWritePostIncrement();
    }

    // write holding registers to flash
    TBLPTR = (uint24_t) address;

    EECON1bits.FREE = 0;
    EECON1bits.WREN = 1;

    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;

    EECON1bits.WREN = 0;

    // verify
    i = FLASH_WRITE_BLOCK;
    TBLPTR = (uint24_t) address;
    d = (uint8_t*) data;

    while(i--)
    {
      TableReadPostIncrement();
      if(TABLAT != *d++)
      {
        critical_section_exit();
        return false;
      }
    }

    critical_section_exit();
  }

  return true;
}

// ensure write is in application data section

static bool is_range_valid(const far rom void* address)
{
  if(((uint24_t) address < APPLICATION_DATA_START) || (APPLICATION_DATA_END < (uint24_t) address))
  {
    return false;
  }
  else
  {
    return true;
  }
}

static bool blank_check_block(const far rom void* address)
{
  uint16_t i = FLASH_ERASE_BLOCK;

  critical_section_enter();

  TBLPTR = (uint24_t) address;

  while(i--)
  {
    TableReadPostIncrement();
    if(TABLAT != 0xff)
    {
      critical_section_exit();
      return false;
    }
  }

  critical_section_exit();

  return true;
}

#endif
