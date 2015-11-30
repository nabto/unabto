/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_STORAGE

#include "configuration_store.h"
#include <modules/crc/crc32.h>

#if NABTO_CONFIGURATION_STORE_ENABLE

#define CACHE_SIGNATURE         0xfeedbeeful

typedef struct {
    uint32_t signature;
    uint32_t counter;
    uint8_t data[NABTO_CONFIGURATION_STORE_SIZE];
    uint32_t crc;
} configuration_store_cache;

// cache meta operations
static bool verify_cache_integrity(void);
static void update_cache_meta_section(void);

// cache - backing store IO operations
static bool cache_fill(void);
static bool cache_flush(void);

static configuration_store_cache cache;

static const char* config_filename = "config.bin";

bool configuration_store_initialize(__ROM application_configuration* defaultApplicationConfiguration)
{
    if(sizeof(application_configuration) > NABTO_CONFIGURATION_STORE_SIZE)
    {
        NABTO_LOG_FATAL(("Configuration store is not big enough to hold the defined application configuration!"));
        return false;
    }

    // open the cache backing store and fill the cache
    if(cache_fill() == false)
    {
        return false;
    }
    
    // ensure cache is valid
    if(verify_cache_integrity() == false)
    {
        NABTO_LOG_INFO(("Configuration store failed integrity check - formatting."));
        if(configuration_store_format(defaultApplicationConfiguration) == false)
        {
            return false;
        }
    }

    NABTO_LOG_INFO(("Initialized configuration store."));

    return true;
}

bool configuration_store_format(__ROM application_configuration* defaultApplicationConfiguration)
{
    memset(cache.data, 0, sizeof(cache.data));

    if(defaultApplicationConfiguration != NULL)
    {
        UNABTO_ASSERT(sizeof(application_configuration) <= sizeof(cache.data));
        memcpy(cache.data, defaultApplicationConfiguration, sizeof(application_configuration));
    }

    update_cache_meta_section();

    if(cache_flush() == false)
    {
        return false;
    }

    NABTO_LOG_INFO(("Formatted configuration store."));

    return true;
}

bool configuration_store_read(uint16_t offset, void* data, uint16_t length)
{
  if((offset + length) > sizeof(cache.data))
  {
    NABTO_LOG_FATAL(("Out of bounds read in configuration store."));
    return false;
  }

  memcpy(data, &cache.data[offset], length);
  
  NABTO_LOG_TRACE(("Read %u bytes starting at offset %x", (int) length, (int)offset));

  return true;
}

bool configuration_store_write(uint16_t offset, const void* data, uint16_t length)
{
    if((offset + length) > sizeof(cache.data))
    {
        NABTO_LOG_FATAL(("Out of bounds write in configuration store."));
        return false;
    }

    memcpy(&cache.data[offset], data, length);
  
    update_cache_meta_section();

    if(cache_flush() == false)
    {
        NABTO_LOG_ERROR(("Unable to flush configuration store cache!"));
        return false;
    }

    NABTO_LOG_TRACE(("Wrote %u bytes starting at offset %x", (int) length, (int)offset));

    return true;
}

bool configuration_store_set(uint16_t offset, uint8_t value, uint16_t length)
{
    if((offset + length) > sizeof(cache.data))
    {
        NABTO_LOG_FATAL(("Out of bounds set in configuration store."));
        return false;
    }

    memset(&cache.data[offset], value, length);
  
    update_cache_meta_section();

    if(cache_flush() == false)
    {
        NABTO_LOG_ERROR(("Unable to flush configuration store cache!"));
        return false;
    }

    NABTO_LOG_TRACE(("Set %u bytes to %u starting at offset %x", (int) length, (int)value, (int)offset));

    return true;
}

bool configuration_store_compare(uint16_t offset, const void* data, uint16_t length, bool* match)
{
  uint8_t buffer[NABTO_CONFIGURATION_STORE_SIZE];

  if(configuration_store_read(offset, buffer, length) == false)
  {
    return false;
  }

  *match = memcmp(data, buffer, length) == 0;

  return true;
}

static bool verify_cache_integrity(void)
{
    if(cache.signature != CACHE_SIGNATURE)
    {
        NABTO_LOG_INFO(("Configuration store backing file did not have the correct signature."));
        return false;
    }

    if(crc32_calculate(&cache, sizeof(cache) - 4) != cache.crc)
    {
        NABTO_LOG_INFO(("Configuration store backing file did not have the correct CRC."));
        return false;
    }

    return true;
}

static void update_cache_meta_section(void)
{
    cache.signature = CACHE_SIGNATURE;
    cache.counter++;
    cache.crc = crc32_calculate(&cache, sizeof(cache) - 4);
}

static FILE* cacheFile = NULL;

static bool cache_fill(void)
{
    // open file on initial cache fill
    if(cacheFile == NULL)
    {
        cacheFile = fopen(config_filename, "r+b"); // open existing file
        
        // verify that the size is correct
        if(cacheFile != NULL)
        {
            long fileSize;

            if(fseek(cacheFile, 0, SEEK_END) != 0)
            {
                return false;
            }

            fileSize = ftell(cacheFile);

            if(fileSize != sizeof(cache))
            {
                fclose(cacheFile);
                cacheFile = NULL;
            }
        }
        
        // if file did not exist or had the wrong size create a new file
        if(cacheFile == NULL)
        {
            cacheFile = fopen(config_filename, "w+b");
            if(cacheFile == NULL)
            {
                return false;
            }

            memset(&cache, 0, sizeof(cache));
            if(cache_flush() == false)
            {
                return false;
            }
        }
    }

    if(cacheFile == NULL)
    {
        return false;
    }
    
    if(fseek(cacheFile, 0, SEEK_SET) != 0)
    {
        return false;
    }
    
    if(1 != fread(&cache, sizeof(cache), 1, cacheFile))
    {
        memset(&cache, 0, sizeof(cache));
        return false;
    }
    
    return true;
}

static bool cache_flush(void)
{
    if(cacheFile == NULL)
    {
        return false;
    }
    
    if(fseek(cacheFile, 0, SEEK_SET) != 0)
    {
        return false;
    }
    
    if(1 != fwrite(&cache, sizeof(cache), 1, cacheFile))
    {
        return false;
    }

    if(fflush(cacheFile) != 0)
    {
        return false;
    }

    return true;
}

#endif
