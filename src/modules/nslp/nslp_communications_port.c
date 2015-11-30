/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NSLP

#include "nslp_communications_port.h"

bool nslpCommunicationsPortLock = false;

#if _WIN32

#define END                                                                   0xc0

static void flush_cache(void);

static uint8_t transmitCache[10240];
static uint16_t transmitCachePointer = 0;

void nslp_communications_port_write(uint8_t value)
{
  if (transmitCachePointer >= sizeof(transmitCache))
  {
    flush_cache();
  }

  transmitCache[transmitCachePointer++] = value;

  if (value == END && transmitCachePointer > 1)
  {
    flush_cache();
  }
}

void nslp_communications_port_write_buffer(const void* buffer, uint16_t length)
{
  uint8_t* b = (uint8_t*)buffer;

  while (length--)
  {
    nslp_communications_port_write(*b++);
  }
}

static void flush_cache(void)
{
  uart_write_buffer(0, transmitCache, transmitCachePointer);
  transmitCachePointer = 0;
}

#endif
