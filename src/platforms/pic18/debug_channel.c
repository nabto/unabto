/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#include "debug_channel.h"

#if NABTO_USE_DEBUG_CHANNEL

#include <stdlib.h>

#define BIT_STRETCH_10 nop nop nop nop nop nop nop nop nop nop

// bitrates at 25 MHz * 5 / 3 ~ 42 MHz
// 84 - 115741 bps
//#define BIT_STRETCH BIT_STRETCH_10 BIT_STRETCH_10 BIT_STRETCH_10 BIT_STRETCH_10 BIT_STRETCH_10 BIT_STRETCH_10 BIT_STRETCH_10 BIT_STRETCH_10 nop nop nop nop
// 9 - 694444 bps
//#define BIT_STRETCH nop nop nop nop nop nop nop nop nop
// 4 - 1041667 bps
//#define BIT_STRETCH nop nop nop nop
// 2 - 1302083 bps
//#define BIT_STRETCH nop nop
// 1 - 7 clocks per bit => 1488095 bps (~1.5 Mbps) @ 41.67 MHz
#define BIT_STRETCH nop
// 0 - 6 blocks per bit => 1736111 bps - seems to be too fast to work
//#define BIT_STRETCH

#ifndef BIT_STRETCH
#define BIT_STRETCH
#endif

#pragma udata access debug_channel_access_section
static near uint8_t tempValue;
#pragma udata

void debug_channel_initialize(void)
{
  LATBbits.LATB6 = 1;
  TRISBbits.TRISB6 = 0;

  stdout = _H_USER;

  debug_channel_write('\n');
}

void debug_channel_uninitialize(void)
{
  TRISBbits.TRISB6 = 1;
  LATBbits.LATB6 = 0;
}

void debug_channel_write(uint8_t value)
{
  critical_section_enter();

  tempValue = value;

  _asm

  // start bit
startbit:
  bcf LATB, 6, 0
  nop

  // data bits (lsb first)

  // bit 0
bit0:
  BIT_STRETCH
  //            rrcf value, 1, 0
  btfss tempValue, 0, 0
  bra bit0_low
bit0_high:
  nop
  bsf LATB, 6, 0
  bra bit1
bit0_low:
  bcf LATB, 6, 0
  nop
  nop
  // bit 1
bit1:
  BIT_STRETCH
  btfss tempValue, 1, 0
  bra bit1_low
bit1_high:
  nop
  bsf LATB, 6, 0
  bra bit2
bit1_low:
  bcf LATB, 6, 0
  nop
  nop
  // bit 2
bit2:
  BIT_STRETCH
  btfss tempValue, 2, 0
  bra bit2_low
bit2_high:
  nop
  bsf LATB, 6, 0
  bra bit3
bit2_low:
  bcf LATB, 6, 0
  nop
  nop
  // bit 3
bit3:
  BIT_STRETCH
  btfss tempValue, 3, 0
  bra bit3_low
bit3_high:
  nop
  bsf LATB, 6, 0
  bra bit4
bit3_low:
  bcf LATB, 6, 0
  nop
  nop
  // bit 4
bit4:
  BIT_STRETCH
  btfss tempValue, 4, 0
  bra bit4_low
bit4_high:
  nop
  bsf LATB, 6, 0
  bra bit5
bit4_low:
  bcf LATB, 6, 0
  nop
  nop
  // bit 5
bit5:
  BIT_STRETCH
  btfss tempValue, 5, 0
  bra bit5_low
bit5_high:
  nop
  bsf LATB, 6, 0
  bra bit6
bit5_low:
  bcf LATB, 6, 0
  nop
  nop
  // bit 6
bit6:
  BIT_STRETCH
  btfss tempValue, 6, 0
  bra bit6_low
bit6_high:
  nop
  bsf LATB, 6, 0
  bra bit7
bit6_low:
  bcf LATB, 6, 0
  nop
  nop
  // bit 7
bit7:
  BIT_STRETCH
  btfss tempValue, 7, 0
  bra bit7_low
bit7_high:
  nop
  bsf LATB, 6, 0
  bra stopbit
bit7_low:
  bcf LATB, 6, 0
  nop
  nop

  // stop bit
stopbit:
  BIT_STRETCH
  nop
  nop
  nop
  bsf LATB, 6, 0

  _endasm

  critical_section_exit();
}

int _user_putc(char c)
{
  debug_channel_write(c);
  return c;
}

static const rom uint8_t letters[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

void debug_channel_write_string(const char* string)
{
  while(*string)
  {
    debug_channel_write(*string++);
  }
}

void debug_channel_write_string_pgm(const far rom char* string)
{
  while(*string)
  {
    debug_channel_write(*string++);
  }
}

void debug_channel_write_uint8_hex(uint8_t value)
{
  debug_channel_write(letters[value >> 4]);
  debug_channel_write(letters[value & 0x0f]);
}

void debug_channel_write_uint16_hex(uint16_t value)
{
  debug_channel_write(letters[value >> 12]);
  debug_channel_write(letters[(value >> 8) & 0x0f]);
  debug_channel_write(letters[(value >> 4) & 0x0f]);
  debug_channel_write(letters[value & 0x0f]);
}

void debug_channel_write_int32(int32_t value)
{
  char buffer[12];

  ltoa(value, buffer);

  debug_channel_write_string(buffer);
}

#endif
