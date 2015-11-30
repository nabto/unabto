/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_STORAGE

#include <unabto/unabto_util.h>
#include "configuration_store.h"
#include <string.h>
#include <flash_io_driver.h>
#include "nano1xx_fmc.h"

#if NABTO_CONFIGURATION_STORE_ENABLE

#define PAGE_SIGNATURE              0xbeef

// Flash is written in blocks of 4 bytes so it is required that: sizeof(flash_page_meta) % FLASH_WRITE_BLOCK_SIZE == 0
typedef struct
{
  uint16_t checksum;
  uint16_t signature;
  uint32_t counter;
} flash_page_meta;

#define FLASH_PAGE_SIZE             (NABTO_CONFIGURATION_STORE_SIZE)

#if FLASH_PAGE_SIZE % FLASH_ERASE_BLOCK_SIZE != 0
#error FLASH_PAGE_SIZE must be be a multiple of the erase block size!
#endif

#define FLASH_PAGE_DATA_SIZE        (NABTO_CONFIGURATION_STORE_SIZE - sizeof(flash_page_meta))

typedef struct
{
  uint8_t content[FLASH_PAGE_DATA_SIZE];
  flash_page_meta meta;
} flash_page;

static bool erase_page(uint8_t page);
static bool write_meta_section(uint8_t page, uint32_t counter);
static bool is_page_valid(uint8_t page);

//static __no_init const flash_page pages[2] @ 0xF800; // reserve room for 2 x 1 kB
static __no_init const flash_page pages[2] @ 0x1e400; // reserve room for 2 x 1 kB
//static const flash_page pages[2];

static uint8_t activePage;
static uint8_t inactivePage;

bool configuration_store_initialize(__ROM application_configuration* defaultApplicationConfiguration)
{
  if(is_page_valid(0))
  {
    activePage = 0;
    inactivePage = 1;
    return erase_page(1);
  }
  else if(is_page_valid(1))
  {
    activePage = 1;
    inactivePage = 0;
    return erase_page(0);
  }
  else
  {
    return configuration_store_format(defaultApplicationConfiguration);
  }
}

bool configuration_store_format(__ROM application_configuration* defaultApplicationConfiguration)
{
  uint16_t length = sizeof(application_configuration);
  const uint8_t* source = (uint8_t*) defaultApplicationConfiguration;
  const uint8_t* destination = pages[0].content;

  // erase page 0
  if(erase_page(0) == false)
  {
    return false;
  }
  // erase page 1
  if(erase_page(1) == false)
  {
    return false;
  }

  // if a default value was given write it to page 0 as this will be the active page after a format
  while(length > 0)
  {
    uint32_t block = 0xffffffff;
    uint16_t currentBlockSize = FLASH_WRITE_BLOCK_SIZE;
    
    if(length < FLASH_WRITE_BLOCK_SIZE)
    {
      currentBlockSize = length;
    }
    
    memcpy(&block, (const void*) source, currentBlockSize);
    
    // write block
    if(flash_io_driver_write(destination, &block, currentBlockSize) == false)
    {
      return false;
    }
    
    source += currentBlockSize;
    destination += currentBlockSize;
    length -= currentBlockSize;
  }

  if(write_meta_section(0, 1) == false) // create the meta section - counter starts at 1 for a newly formatted configuration store
  {
    return false;
  }

  activePage = 0;
  inactivePage = 1;

  return true;
}

bool configuration_store_read(uint16_t offset, void* data, uint16_t length)
{
  memcpy(data, &pages[activePage].content[offset], length);

  return true;
}

bool configuration_store_write(uint16_t offset, const void* data, uint16_t length)
{
  uint8_t buffer[FLASH_WRITE_BLOCK_SIZE];
  uint8_t j;
  uint16_t i;
  uint8_t* source = (uint8_t*) data;
  uint16_t startAddress = offset;
  uint16_t endAddress = offset + length - 1;
  const uint8_t* destination = (const uint8_t*) pages[inactivePage].content;
  const uint8_t* readAddress = (const uint8_t*) pages[activePage].content;

  for(i = 0; i < FLASH_PAGE_DATA_SIZE; /*i is incremented in the next for-loop*/)
  {
    memcpy(buffer, readAddress, FLASH_WRITE_BLOCK_SIZE); // read current block

    // modify appropriate parts of the block
    for(j = 0; j < FLASH_WRITE_BLOCK_SIZE; j++, i++)
    {
      if(startAddress <= i && i <= endAddress)
      {
        buffer[j] = *source++;
      }
    }

    // write back block
    if(flash_io_driver_write(destination, (const void*) buffer, FLASH_WRITE_BLOCK_SIZE) == false)
    {
      return false;
    }

    readAddress += FLASH_WRITE_BLOCK_SIZE;
    destination += FLASH_WRITE_BLOCK_SIZE;
  }

  if(write_meta_section(inactivePage, pages[activePage].meta.counter + 1) == false)
  {
    return false;
  }

  // swap active/inactive pages
  j = inactivePage;
  inactivePage = activePage;
  activePage = j;

  // erase the previously active page
  return flash_io_driver_erase_block(&pages[inactivePage]);
}

bool configuration_store_set(uint16_t offset, const uint8_t value, uint16_t length)
{
  uint8_t buffer[FLASH_WRITE_BLOCK_SIZE];
  uint8_t j;
  uint16_t i;
  uint16_t startAddress = offset;
  uint16_t endAddress = offset + length - 1;
  const uint8_t* destination = (const uint8_t*) pages[inactivePage].content;
  const uint8_t* readAddress = (const uint8_t*) pages[activePage].content;

  for(i = 0; i < FLASH_PAGE_DATA_SIZE; /*i is incremented in the next for-loop*/)
  {
    memcpy(buffer, readAddress, FLASH_WRITE_BLOCK_SIZE); // read current block

    // modify appropriate parts of the block
    for(j = 0; j < FLASH_WRITE_BLOCK_SIZE; j++, i++)
    {
      if(startAddress <= i && i <= endAddress)
      {
        buffer[j] = value;
      }
    }

    // write back block
    if(flash_io_driver_write(destination, (const void*) buffer, FLASH_WRITE_BLOCK_SIZE) == false)
    {
      return false;
    }

    readAddress += FLASH_WRITE_BLOCK_SIZE;
    destination += FLASH_WRITE_BLOCK_SIZE;
  }

  if(write_meta_section(inactivePage, pages[activePage].meta.counter + 1) == false)
  {
    return false;
  }

  // swap active/inactive pages
  j = inactivePage;
  inactivePage = activePage;
  activePage = j;

  // erase the previously active page
  return flash_io_driver_erase_block(&pages[inactivePage]);
}
/*
bool configuration_store_set(uint16_t offset, uint8_t value, uint16_t length)
{
  uint8_t buffer[FLASH_ERASE_BLOCK_SIZE];
  uint8_t j;
  uint16_t i;
  uint16_t startAddress = offset;
  uint16_t endAddress = offset + length - 1;
  const uint8_t* writeAddress = (const uint8_t*) pages[inactivePage].content;
  const uint8_t* readAddress = (const uint8_t*) pages[activePage].content;

  for(i = 0; i < FLASH_PAGE_DATA_SIZE;)
  {
    memcpy(buffer, readAddress, FLASH_WRITE_BLOCK_SIZE); // read current block

    // modify appropriate parts of the block
    for(j = 0; j < FLASH_WRITE_BLOCK_SIZE; j++, i++)
    {
      if(startAddress <= i && i <= endAddress)
      {
        buffer[j] = value;
      }
    }

    if(flash_io_driver_write(writeAddress, (const void*) buffer, FLASH_WRITE_BLOCK_SIZE) == false) // write back block
    {
      return false;
    }

    readAddress += FLASH_WRITE_BLOCK_SIZE;
    writeAddress += FLASH_WRITE_BLOCK_SIZE;
  }

  if(write_meta_section(inactivePage, pages[activePage].meta.counter + 1) == false)
  {
    return false;
  }

  // swap active/inactive pages
  j = inactivePage;
  inactivePage = activePage;
  activePage = j;

  // erase the previous active page
  return erase_page(inactivePage);
}
*/
bool configuration_store_compare(uint16_t offset, const void* data, uint16_t length, bool* match)
{
  *match = memcmp(&pages[activePage].content[offset], data, length) == 0;

  return true;
}

static bool erase_page(uint8_t page)
{
  uint8_t* address = (uint8_t*)&pages[page];
  uint8_t* endAddress = address + sizeof(flash_page);
  
  while(address < endAddress)
  {
    if(flash_io_driver_erase_block(address) == false)
    {
      return false;
    }
      
    address += FLASH_ERASE_BLOCK_SIZE;
  }
  
  return true;
}

static bool write_meta_section(uint8_t page, uint32_t counter)
{
  union
  {
    uint32_t _32;
    uint16_t _16[2];
  } checksum;
  uint16_t i;
  uint8_t j;
  flash_page_meta meta;
  const uint16_t* p = (const uint16_t*) pages[page].content;
  const uint16_t* m = (const uint16_t*) &meta;

  // create new meta section
  meta.signature = PAGE_SIGNATURE;
  meta.counter = counter;
  meta.checksum = 0;

  checksum._32 = 0;

  // calculate checksum of the data part of the page (already in flash)
  for(i = 0; i < (FLASH_PAGE_DATA_SIZE / sizeof (uint16_t)); i++)
  {
    checksum._32 += *p++;
  }

  // calculate checksum of the meta part of the page (in temporary RAM)
  for(j = 0; j < (sizeof(flash_page_meta) / sizeof (uint16_t)); j++)
  {
    checksum._32 += *m++;
  }
  
  // add overflow to make it a one's complement sum
  checksum._32 = (uint32_t) checksum._16[0] + (uint32_t) checksum._16[1];
  checksum._16[0] += checksum._16[1];

  meta.checksum = ~checksum._16[0];

  return flash_io_driver_write(&pages[page].meta, &meta, sizeof(meta));
}

static bool is_page_valid(uint8_t page)
{
  union
  {
    uint32_t _32;
    uint16_t _16[2];
  } checksum;
  uint16_t i;
  const uint16_t* p = (const uint16_t*) pages[page].content;

  if(pages[page].meta.signature != PAGE_SIGNATURE)
  {
    return false;
  }

  checksum._32 = 0;

  for(i = 0; i < (sizeof (flash_page) / sizeof (uint16_t)); i++)
  { // perform checksum verification over the entire page
    checksum._32 += *p++;
  }

  checksum._32 = (uint32_t) checksum._16[0] + (uint32_t) checksum._16[1];
  checksum._16[0] += checksum._16[1];

  checksum._16[0] = ~checksum._16[0];

  return checksum._16[0] == 0;
}

#endif
