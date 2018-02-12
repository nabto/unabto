/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_STORAGE

#include <unabto/unabto_util.h>
#include "configuration_store.h"
#include <flash.h>
#include <flash_io_driver.h>

#if NABTO_CONFIGURATION_STORE_ENABLE

#define PAGE_SIGNATURE              0xbeeful
#define FLASH_PAGE_META_SIZE        (64ul)
#define FLASH_PAGE_SIZE             (1024ul)
#define FLASH_PAGE_DATA_SIZE        (FLASH_PAGE_SIZE - FLASH_PAGE_META_SIZE)

#if NABTO_CONFIGURATION_STORE_SIZE > FLASH_PAGE_DATA_SIZE
#error BAD SIZE!
#endif

typedef union
{
  struct
  {
    uint16_t checksum;
    uint16_t signature;
    uint32_t counter;
  };
  uint8_t filler[FLASH_PAGE_META_SIZE];
} flash_page_meta;

typedef struct
{
  uint8_t content[FLASH_PAGE_DATA_SIZE];
  flash_page_meta meta;
} flash_page;

static bool write_meta_section(uint8_t page, uint32_t counter);
static bool is_page_valid(uint8_t page);

#pragma romdata configuration_section = 0x1f400
static const far rom flash_page pages[2];
#pragma romdata

static uint8_t activePage;
static uint8_t inactivePage;

bool configuration_store_initialize(__ROM application_configuration* defaultApplicationConfiguration)
{
  if(is_page_valid(0))
  {
    activePage = 0;
    inactivePage = 1;
    return flash_io_driver_erase_block(&pages[1]);
  }
  else if(is_page_valid(1))
  {
    activePage = 1;
    inactivePage = 0;
    return flash_io_driver_erase_block(&pages[0]);
  }
  else
  {
    return configuration_store_format(defaultApplicationConfiguration);
  }
}

bool configuration_store_format(__ROM application_configuration* defaultApplicationConfiguration)
{
  uint8_t buffer[FLASH_WRITE_BLOCK];
  uint16_t defaultApplicationConfigurationSize = sizeof (application_configuration);
  const far rom void* source = (const far rom void*) defaultApplicationConfiguration;
  const far rom void* destination = pages[0].content;

  // erase page 0
  if(flash_io_driver_erase_block(&pages[0]) == false)
  {
    return false;
  }
  // erase page 1
  if(flash_io_driver_erase_block(&pages[1]) == false)
  {
    return false;
  }

  // if a default value was given write it to page 0 as this will be the active page after a format
  while(defaultApplicationConfigurationSize > 0)
  {
    uint8_t size = MIN(defaultApplicationConfigurationSize, FLASH_WRITE_BLOCK);

    memset(buffer, 0xff, FLASH_WRITE_BLOCK); // leave unused bytes at the default erased level (bits set high)
    memcpypgm2ram(buffer, (const far rom void*) source, size); // overwrite part of/the whole buffer with data from the default block

    // write the buffer to the actual flash
    if(flash_io_driver_write_block(destination, (const void*) buffer) == false)
    {
      return false;
    }

    // Move on to next block.
    // The last block might not be FLASH_WRITE_BLOCK bytes long but it is safe to move the pointers that far as 'defaultApplicationConfigurationSize' will decrement correctly (saves 26 bytes of code).
    source += FLASH_WRITE_BLOCK;
    destination += FLASH_WRITE_BLOCK;
    defaultApplicationConfigurationSize -= size;
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
  memcpypgm2ram(data, (const far rom void*) &pages[activePage].content[offset], length);

  return true;
}

bool configuration_store_write(uint16_t offset, const void* data, uint16_t length)
{
  uint8_t buffer[FLASH_WRITE_BLOCK];
  uint8_t j;
  uint16_t i;
  uint8_t* source = (uint8_t*) data;
  uint16_t startAddress = offset;
  uint16_t endAddress = offset + length - 1;
  const far rom void* writeAddress = (const far rom void*) pages[inactivePage].content;
  const far rom void* readAddress = (const far rom void*) pages[activePage].content;

  for(i = 0; i < FLASH_PAGE_DATA_SIZE; /*i is incremented in the next for-loop*/)
  {
    memcpypgm2ram(buffer, readAddress, FLASH_WRITE_BLOCK); // read current block

    // modify appropriate parts of the block
    for(j = 0; j < FLASH_WRITE_BLOCK; j++, i++)
    {
      if(startAddress <= i && i <= endAddress)
      {
        buffer[j] = *source++;
      }
    }

    // write back block
    if(flash_io_driver_write_block(writeAddress, (const void*) buffer) == false)
    {
      return false;
    }

    readAddress += FLASH_WRITE_BLOCK;
    writeAddress += FLASH_WRITE_BLOCK;
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

bool configuration_store_set(uint16_t offset, uint8_t value, uint16_t length)
{
  uint8_t buffer[FLASH_WRITE_BLOCK];
  uint8_t j;
  uint16_t i;
  uint16_t startAddress = offset;
  uint16_t endAddress = offset + length - 1;
  const far rom void* writeAddress = (const far rom void*) pages[inactivePage].content;
  const far rom void* readAddress = (const far rom void*) pages[activePage].content;

  for(i = 0; i < FLASH_PAGE_DATA_SIZE;)
  {
    memcpypgm2ram(buffer, readAddress, FLASH_WRITE_BLOCK); // read current block

    // modify appropriate parts of the block
    for(j = 0; j < FLASH_WRITE_BLOCK; j++, i++)
    {
      if(startAddress <= i && i <= endAddress)
      {
        buffer[j] = value;
      }
    }

    if(flash_io_driver_write_block(writeAddress, (const void*) buffer) == false) // write back block
    {
      return false;
    }

    readAddress += FLASH_WRITE_BLOCK;
    writeAddress += FLASH_WRITE_BLOCK;
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
  return flash_io_driver_erase_block(&pages[inactivePage]);
}

bool configuration_store_compare(uint16_t offset, const void* data, uint16_t length, bool* match)
{
  *match = memcmppgm2ram((void*) data, (const far rom void*) &pages[activePage].content[offset], length) == 0;

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
  const far rom uint16_t* p = (const far rom uint16_t*) pages[page].content;
  const uint16_t* m = (const uint16_t*) &meta;

  // create new meta section
  memset(&meta, 0xff, sizeof (meta));
  meta.signature = PAGE_SIGNATURE;
  meta.counter = counter;
  meta.checksum = 0;

  checksum._32 = 0;

  // calculate checksum of the data part of the page
  for(i = 0; i < (FLASH_PAGE_DATA_SIZE / sizeof (uint16_t)); i++)
  {
    checksum._32 += *p++;
  }

  // calculate checksum of the meta part of the page
  for(j = 0; j < (FLASH_PAGE_META_SIZE / sizeof (uint16_t)); j++)
  {
    checksum._32 += *m++;
  }

  // add overflow to make it a one's complement sum
  checksum._32 = (uint32_t) checksum._16[0] + (uint32_t) checksum._16[1];
  checksum._16[0] += checksum._16[1];

  meta.checksum = ~checksum._16[0];

  return flash_io_driver_write_block((const far rom void*) &pages[page].meta, (const void*) &meta);
}

static bool is_page_valid(uint8_t page)
{

  union
  {
    uint32_t _32;
    uint16_t _16[2];
  } checksum;
  uint16_t i;
  const far rom uint16_t* p = (const far rom uint16_t*) pages[page].content;

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
