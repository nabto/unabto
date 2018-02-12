/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include "iic.h"

#define SDA_HIGH(channel) do { if(channel == 0) { TRISBbits.TRISB5 = 1; } else { TRISEbits.TRISE5 = 1; } } while(0)
#define SDA_LOW(channel) do { if(channel == 0) { TRISBbits.TRISB5 = 0; } else { TRISEbits.TRISE5 = 0; } } while(0)
#define SCL_HIGH(channel) do { if(channel == 0) { TRISBbits.TRISB4 = 1; } else { TRISEbits.TRISE4 = 1; } } while(0)
#define SCL_LOW(channel) do { if(channel == 0) { TRISBbits.TRISB4 = 0; } else { TRISEbits.TRISE4 = 0; } } while(0)
#define SDA_IN(channel)  (channel == 0 ? PORTBbits.RB5 : PORTEbits.RE5)

static uint8_t readByte(uint8_t channel, bool acknowledge);
static bool writeByte(uint8_t channel, uint8_t value);

#define delay(n) Nop(); Nop(); Nop(); Nop()

void iic_initialize(uint8_t channel)
{
  if(channel == 0)
  {
    TRISBbits.TRISB4 = 1;
    TRISBbits.TRISB5 = 1;
    LATBbits.LATB4 = 0;
    LATBbits.LATB5 = 0;
  }
  else
  {
    TRISEbits.TRISE4 = 1;
    TRISEbits.TRISE5 = 1;
    LATEbits.LATE4 = 0;
    LATEbits.LATE5 = 0;
  }
}

void iic_start(uint8_t channel)
{
  SDA_LOW(channel);
  delay();
  SCL_LOW(channel);
}

void iic_repeated_start(uint8_t channel)
{
  SDA_HIGH(channel);
  SCL_HIGH(channel);
  delay();
  SDA_LOW(channel);
  delay();
  SCL_LOW(channel);
}

void iic_stop(uint8_t channel)
{
  SCL_HIGH(channel);
  delay();
  SDA_HIGH(channel);
  delay();
}

bool iic_write(uint8_t channel, void* data, uint16_t length)
{
  uint8_t* p = (uint8_t*) data;

  while(length--)
  {
    if(writeByte(channel, *p++) == false)
    {
      return false;
    }
  }

  return true;
}

void iic_read(uint8_t channel, void* data, uint16_t length)
{
  uint8_t* p = (uint8_t*) data;

  while(length--)
  {
    *p++ = readByte(channel, length != 0);
  }
}

static uint8_t readByte(uint8_t channel, bool acknowledge)
{
  uint8_t i;
  uint8_t value = 0;

  SDA_HIGH(channel);

  delay();

  for(i = 0; i < 8; i++)
  {
    SCL_HIGH(channel);

    delay();

    value <<= 1;
    value |= SDA_IN(channel);

    SCL_LOW(channel);

    delay();
  }

  if(acknowledge)
  {
    SDA_LOW(channel);
  }

  delay();

  SCL_HIGH(channel);

  delay();

  SCL_LOW(channel);

  SDA_LOW(channel);

  return value;
}

static bool writeByte(uint8_t channel, uint8_t value)
{
  uint8_t i;

  for(i = 0; i < 8; i++)
  {
    if(value & 0x80)
    {
      SDA_HIGH(channel);
    }
    else
    {
      SDA_LOW(channel);
    }
    value <<= 1;

    delay();

    SCL_HIGH(channel);

    delay();

    SCL_LOW(channel);
  }

  delay();

  SDA_HIGH(channel);

  SCL_HIGH(channel);

  delay();

  i = SDA_IN(channel);

  SCL_LOW(channel);

  delay();

  SDA_LOW(channel);

  delay();

  return i == 0;
}
