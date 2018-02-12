/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_STORAGE

#include "configuration_store.h"

#if NABTO_CONFIGURATION_STORE_ENABLE

#include <string.h>

static uint8_t store[NABTO_CONFIGURATION_STORE_SIZE];

bool configuration_store_initialize(__ROM application_configuration* defaultApplicationConfiguration)
{
  NABTO_LOG_TRACE(("Initialized store."));

  return configuration_store_format(defaultApplicationConfiguration);
}

bool configuration_store_format(__ROM application_configuration* defaultApplicationConfiguration)
{
  memset(store, 0, sizeof(store));

  memcpy(store, defaultApplicationConfiguration, sizeof(application_configuration));
  
  NABTO_LOG_TRACE(("Formatted store."));

  return true;
}

bool configuration_store_read(uint16_t offset, void* data, uint16_t length)
{
  if((offset + length) > sizeof(store))
  {
    NABTO_LOG_FATAL(("Out of bounds read in configuration store."));
    return false;
  }

  memcpy(data, &store[offset], length);
  
  NABTO_LOG_TRACE(("Read %u bytes starting at offset %x", (int) length, (int)offset));

  return true;
}

bool configuration_store_write(uint16_t offset, const void* data, uint16_t length)
{
  if((offset + length) > sizeof(store))
  {
    NABTO_LOG_FATAL(("Out of bounds write in configuration store."));
    return false;
  }

  memcpy(&store[offset], data, length);
  
  NABTO_LOG_TRACE(("Wrote %u bytes starting at offset %x", (int) length, (int)offset));

  return true;
}

bool configuration_store_set(uint16_t offset, uint8_t value, uint16_t length)
{
  if((offset + length) > sizeof(store))
  {
    NABTO_LOG_FATAL(("Out of bounds write in configuration store."));
    return false;
  }

  memset(&store[offset], value, length);

  NABTO_LOG_TRACE(("Set %u bytes to %u starting at offset %x", (int) length, (int)value, (int)offset));

  return true;
}

bool configuration_store_compare(uint16_t offset, const void* data, uint16_t length, bool* match)
{
  *match = memcmp(data, (const void*) &store[offset], length) == 0;

  return true;
}

#endif
