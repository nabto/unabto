/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_PLATFORM

#include <unabto/unabto_env_base.h>
#include <TCPIP Stack/TCPIP.h>
#include <debug_channel.h>

#if NABTO_DONT_USE_BOOTLOADER != 1
#include <bootloader_base.h>
#endif

#ifndef STACK_CHECK_SIZE
#define STACK_CHECK_SIZE 0
#endif

#if STACK_CHECK_SIZE

static const rom uint8_t STACK_CHECK_PATTERN[STACK_CHECK_SIZE] = { 0xca, 0xca, 0xca, 0xca };

#pragma udata stack_check
static uint8_t stackCheck[STACK_CHECK_SIZE];
#pragma udata

#endif

bool platform_initialize(void)
{
#if NABTO_USE_DEBUG_CHANNEL
  debug_channel_initialize();
#endif

#if NABTO_ENABLE_LOGGING && NABTO_USE_DEBUG_CHANNEL != 1
  uart_initialize(115200);
#endif

#if STACK_CHECK_SIZE
  memcpypgm2ram(stackCheck, STACK_CHECK_PATTERN, sizeof(STACK_CHECK_PATTERN));
#endif
  
  // Reset reset indicator flags
  RCON |= 0x33;
  STKPTRbits.STKFUL = 0;
  STKPTRbits.STKUNF = 0;

  // Enable Interrupts
  RCONbits.IPEN = 1; // Enable interrupt priorities
  INTCONbits.GIEH = 1;
  INTCONbits.GIEL = 1;

  TickInit();

  return true;
}

void platform_tick(void)
{
#if STACK_CHECK_SIZE
  if(memcmppgm2ram(stackCheck, STACK_CHECK_PATTERN, sizeof(STACK_CHECK_PATTERN)) != 0)
  {
    NABTO_LOG_FATAL(("Stack overflow!"));
  }
#endif
}

#if !USE_W5100
static uint16_t ledMask;
#endif

void platform_configure_led(uint16_t ledConfiguration)
{
#if !USE_W5100
  if(ledConfiguration == PLATFORM_LED_A) // application controls LED A
  {
    WritePHYReg(PHLCON, 0x39d2);
    ledMask = 0x0100;
  }
  else if(ledConfiguration == PLATFORM_LED_B) // application controls LED B
  {
    WritePHYReg(PHLCON, 0x3d92);
    ledMask = 0x0010;
  }
#endif
}

void platform_control_led(bool state)
{
#if !USE_W5100
  uint16_t readback = (uint16_t) ReadPHYReg(PHLCON).VAL.Val;

  if(state)
  {
    readback &= ~ledMask;
  }
  else
  {
    readback |= ledMask;
  }

  WritePHYReg(PHLCON, readback);
#endif
}
// Default Random Number Generator seed. 0x41FE9F9E corresponds to calling LFSRSeedRand(1)
static uint32_t randSeed = 0x41FE9F9E;

// TODO init with ADC values

void nabto_random(uint8_t* buf, size_t len)
{
    uint8_t i;
    size_t ix;
    for (ix = 0; ix < len; ++ix) {
        // Taps: 32 31 29 1
        // Characteristic polynomial: x^32 + x^31 + x^29 + x + 1
        // Repeat 15 times to make the shift pattern less obvious
        for (i = 0; i < 15; i++) {
            randSeed = (randSeed >> 1) ^ ((0ul - (randSeed & 1ul)) & 0xD0000001ul);
        }

        *buf++ = (uint8_t) randSeed;
    }
}

#if NABTO_ENABLE_LOG_FILENAME

const far rom char* clean_filename(const far rom char* path)
{
  const far rom char* filename = path + strlenpgm(path);

  while(filename > path)
  {
    if(*filename == '/' || *filename == '\\')
    {
      return filename + 1;
    }
    filename--;
  }

  return path;
}

#endif
