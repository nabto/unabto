/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#include "flash_io_driver.h"

#if NABTO_FLASH_IO_DRIVER_ENABLE

#include "nano1xx_fmc.h"
#include <string.h>

#define APPLICATION_DATA_START    0x1e400
#define APPLICATION_DATA_END      (0x1e400 + 2 * 1024 - 1)

static bool is_range_valid(const void* address);
static bool blank_check_block(const void* address);

bool flash_io_driver_initialize(void)
{
  uint32_t desiredConfig0 = 0xF8FFFF7E;
  uint32_t desiredConfig1 = APPLICATION_DATA_START;
  uint32_t config0;
  uint32_t config1;
  bool fail = false;

  critical_section_enter();
  
  UNLOCKREG();    
  FMC_Init(); 
  FMC_EnableAPUpdate();
  LOCKREG();
  
  if(FMC_Read(CONFIG0, &config0) != E_FMC_OK)
  {
    fail = true;
  }
  
  if(FMC_Read(CONFIG1, &config1) != E_FMC_OK)
  {
    fail = true;
  }
  
  if(fail || (desiredConfig0 != config0) || (desiredConfig1 != config1))
  {
    FMC_EnableConfigUpdate();

    if(FMC_WriteConfig(config0, config1) != E_FMC_OK)
    {
      fail = true;
    }
    else
    {
      fail = false;
    }

    FMC_DisableConfigUpdate();
  }
  
  critical_section_exit();
  
  return fail == false;
}

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

    UNLOCKREG();
    if(FMC_Erase((uint32_t)address) != E_FMC_OK)
    {
      LOCKREG();
      critical_section_exit();
      return false;
    }
    LOCKREG();
    
    critical_section_exit();
  }

  // is it blank now?
  return blank_check_block(address);
}

bool flash_io_driver_write(__ROM void* address, const void* data, uint16_t length)
{
  if(is_range_valid(address) == false)
  {
    return false;
  }

  uint32_t _address = (uint32_t)address;
  uint32_t* _data = (uint32_t*)data;
  
  length /= sizeof(uint32_t); // number of bytes -> number of whole words
  
  // write
  for(uint16_t i = 0; i < length; i++)
  {
    UNLOCKREG();
    if(FMC_Write(_address, *_data++) != E_FMC_OK)
    {
      LOCKREG();
      return false;
    }
    LOCKREG();
    
    _address += sizeof(uint32_t);
  }
  
  // verify
  uint32_t* source = (uint32_t*)address;
  _data = (uint32_t*)data;
  
  for(uint16_t i = 0; i < length; i++)
  {
    uint32_t s = *source++;
    uint32_t t = *_data++;
    if(s != t)
    {
      return false;
    }
  }
  
  return true;
}

static bool is_range_valid(const void* address)
{/*
  if(((uint32_t) address < APPLICATION_DATA_START) || (APPLICATION_DATA_END < (uint32_t) address))
  {
    return false;
  }
  else
  {*/
    return true;
  //}
}

static bool blank_check_block(const void* address)
{
  uint32_t* p = (uint32_t*)address;
  
  for(uint16_t i = 0; i < (PAGE_SIZE / sizeof(uint32_t)); i++)
  {
    if(*p++ != 0xffffffff)
    {
      return false;
    }
  }
  
  return true;
}

#endif
