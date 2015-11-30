#include "hal.h"
#include <adc.h>
#include <device_drivers/eeprom/eeprom.h>

#define ADC_FILTERING_FACTOR                                        16

static void set_pwm(uint8_t channel, bool enable, uint8_t level);

uint16_t hardwareVersion = 0;
uint8_t hardwareVersionIndex = 0;

static const uint8_t adcChannelMap[6 + 1] = {6, 7, 8, 9, 10, 11, 4};
static uint8_t adcChannelIndex = 0;
static uint16_t adcValues[6 + 1];

void hal_initialize(void)
{
  switch(hardwareVersionIndex)
  {
    case 0:
      TRISA = 0x20;
      LATA = 0x00;
      TRISB = 0x3f;
      LATB = 0xc0;
      TRISC = 0xff;
      LATC = 0x01;
      TRISD = 0x03;
      LATD = 0x00;
      TRISE = 0x36;
      LATE = 0x00;
      TRISF = 0x7e;
      LATF = 0x00;
      TRISG = 0x00;
      LATG = 0x00;

      // Enable internal PORTB pull-ups
      INTCON2bits.RBPU = 1;

      adc_initialize(0x00 | 0b0011); // set analog input pins 0 - 11 to inputs, set Vss as Vref- and Vdd as Vref+.

      // timer2 config for PWM OUT
      PR2 = 0b11111111;
      T2CON = 0b00000111;
      break;

    case 1:
    case 2:
      TRISA = 0x28;
      LATA = 0x08;
      TRISB = 0x3f;
      LATB = 0xc0;
      TRISC = 0xff;
      LATC = 0x01;
      TRISD = 0x06;
      LATD = 0x00;
      TRISE = 0x30;
      LATE = 0x00;
      TRISF = 0x7e;
      LATF = 0x00;
      TRISG = 0x10;
      LATG = 0x00;

      // Enable internal PORTB pull-ups
      INTCON2bits.RBPU = 1;

      adc_initialize(0x00 | 0b0011); // set analog input pins 0 - 11 to inputs, set Vss as Vref- and Vdd as Vref+.

      // timer2 config for PWM OUT
      PR2 = 0b11111111;
      T2CON = 0b00000111;
      break;
  }

  adc_begin_read(adcChannelMap[adcChannelIndex]);
}

void hal_tick(void)
{
  uint16_t value;

  if(adc_end_read(&value))
  {
    if(hardwareVersionIndex < 2) // hardware version 0.4 beta and 1.2 had power supply noise issues an therefore needed heavy filtering of the analog inputs
    {
      adcValues[adcChannelIndex] *= (ADC_FILTERING_FACTOR - 1);
      adcValues[adcChannelIndex] += value;
      adcValues[adcChannelIndex] /= ADC_FILTERING_FACTOR;
    }
    else // power supply noise issue is history as of hardware version 1.3 (index 2) so no need for filtering
    {
      adcValues[adcChannelIndex] = value;
    }

    if(++adcChannelIndex >= lengthof(adcValues))
    {
      adcChannelIndex = 0;
    }

    adc_begin_read(adcChannelMap[adcChannelIndex]);
  }
}

bool ledRead(void)
{
  switch(hardwareVersionIndex)
  {
    case 0:
      return LATDbits.LATD2;

    case 1:
    case 2:
      return LATEbits.LATE0;

    default:
      return false;
  }
}

void ledWrite(bool value)
{
  switch(hardwareVersionIndex)
  {
    case 0:
      LATDbits.LATD2 = value;
      break;

    case 1:
    case 2:
      LATEbits.LATE0 = value;
      break;
  }
}

/**
 * Read the current status of the onboard button.
 * @return true if the button is currently being pressed.
 */
uint8_t buttonRead(void)
{
  return PORTCbits.RC0; // same pin for all hardware versions
}

/**
 * Read analog input channel.
 * @param channel  channel input number - 0-5 for the external pins or 6 for the onboard temperature sensor.
 * @return the raw value read from the ADC.
 */
uint16_t analogRead(uint8_t pin)
{
  if(pin > 6)
  {
    return 0;
  }
  else
  {
    return adcValues[pin];
  }
}

/**
 * Read the temperature from the onboard temperature sensor
 * @return the temperature in degrees celcius.
 */
int16_t temperatureRead(void)
{
  int32_t t = analogRead(6);

  t *= 211l;
  t -= 32768l;
  t /= 655l;

  return (int16_t) t;
}

enum
{
  HARDWARE_VERSION_0_4,
  HARDWARE_VERSION_1_2,
  HARDWARE_VERSION_1_3,
  HARDWARE_VERSIONS_SUPPORTED
};

#define NUMBER_OF_IO_PINS                                                               15

// look up tables for the io pins for the different revisions of the Nabduino board
near uint8_t * far rom ioTris[HARDWARE_VERSIONS_SUPPORTED][NUMBER_OF_IO_PINS] = {
//      0       1       2       3       4       5       6       7       8       9      10      11      12      13      14
  {&TRISC, &TRISC, &TRISB, &TRISD, &TRISB, &TRISE, &TRISE, &TRISB, &TRISB, &TRISC, &TRISC, &TRISC, &TRISC, &TRISC, &TRISD}, // 0.4 beta
  {&TRISC, &TRISC, &TRISB, &TRISC, &TRISB, &TRISC, &TRISD, &TRISB, &TRISB, &TRISD, &TRISG, &TRISC, &TRISC, &TRISC, 0 /**/}, // 1.2
  {&TRISC, &TRISC, &TRISB, &TRISC, &TRISB, &TRISC, &TRISD, &TRISB, &TRISB, &TRISD, &TRISG, &TRISC, &TRISC, &TRISC, 0 /**/} // 1.3
};
near uint8_t * far rom ioLat[HARDWARE_VERSIONS_SUPPORTED][NUMBER_OF_IO_PINS] = {
//     0      1      2      3      4      5      6      7      8      9     10     11     12     13     14
  {&LATC, &LATC, &LATB, &LATD, &LATB, &LATE, &LATE, &LATB, &LATB, &LATC, &LATC, &LATC, &LATC, &LATC, &LATD},
  {&LATC, &LATC, &LATB, &LATC, &LATB, &LATC, &LATD, &LATB, &LATB, &LATD, &LATG, &LATC, &LATC, &LATC, 0 /**/},
  {&LATC, &LATC, &LATB, &LATC, &LATB, &LATC, &LATD, &LATB, &LATB, &LATD, &LATG, &LATC, &LATC, &LATC, 0 /**/}
};
near uint8_t * far rom ioPort[HARDWARE_VERSIONS_SUPPORTED][NUMBER_OF_IO_PINS] = {
//      0       1       2       3       4       5       6       7       8       9      10      11      12      13      14
  {&PORTC, &PORTC, &PORTB, &PORTD, &PORTB, &PORTE, &PORTE, &PORTB, &PORTB, &PORTC, &PORTC, &PORTC, &PORTC, &PORTC, &PORTD},
  {&PORTC, &PORTC, &PORTB, &PORTC, &PORTB, &PORTC, &PORTD, &PORTB, &PORTB, &PORTD, &PORTG, &PORTC, &PORTC, &PORTC, 0 /**/},
  {&PORTC, &PORTC, &PORTB, &PORTC, &PORTB, &PORTC, &PORTD, &PORTB, &PORTB, &PORTD, &PORTG, &PORTC, &PORTC, &PORTC, 0 /**/}
};
far rom uint8_t ioMask[HARDWARE_VERSIONS_SUPPORTED][NUMBER_OF_IO_PINS] = {
//      0       1       2       3       4       5       6       7       8       9      10      11      12      13      14   // replace "1 << 0" with "1 /**/" to avoid compiler warnings while keeping width.
  {1 << 7, 1 << 6, 1 << 3, 1 /**/, 1 << 1, 1 << 2, 1 << 1, 1 << 2, 1 /**/, 1 << 1, 1 << 2, 1 << 5, 1 << 4, 1 << 3, 1 << 1},
  {1 << 7, 1 << 6, 1 << 3, 1 << 2, 1 << 1, 1 << 1, 1 << 1, 1 << 2, 1 /**/, 1 << 2, 1 << 4, 1 << 5, 1 << 4, 1 << 3, 0 /**/}, // only 14 IOs on version 1.2 onwards so null the last one
  {1 << 7, 1 << 6, 1 << 3, 1 << 2, 1 << 1, 1 << 1, 1 << 1, 1 << 2, 1 /**/, 1 << 2, 1 << 4, 1 << 5, 1 << 4, 1 << 3, 0 /**/}
};

void pinMode(uint8_t pin, uint8_t mode)
{
  uint8_t mask = ioMask[hardwareVersionIndex][pin];

  if(mask == 0)
  {
    return;
  }

  // automatically disable PWM when setting up pin as digital IO
  switch(pin)
  {
    case 3:
      set_pwm(0, false, 0);
      break;
    case 5:
      set_pwm(1, false, 0);
      break;
    case 6:
      set_pwm(2, false, 0);
      break;
    case 9:
      set_pwm(3, false, 0);
      break;
    case 10:
      set_pwm(4, false, 0);
      break;
  }

  switch(mode)
  {
    case INPUT:
      *ioTris[hardwareVersionIndex][pin] |= mask;
      break;

    case OUTPUT:
      *ioTris[hardwareVersionIndex][pin] &= ~mask;
      break;
  }
}

void digitalWrite(uint8_t pin, uint8_t value)
{
  uint8_t mask = ioMask[hardwareVersionIndex][pin];

  if(mask == 0)
  {
    return;
  }

  if(value == LOW)
  {
    *ioLat[hardwareVersionIndex][pin] &= ~mask;
  }
  else
  {
    *ioLat[hardwareVersionIndex][pin] |= mask;
  }
}

uint8_t digitalRead(uint8_t pin)
{
  uint8_t mask = ioMask[hardwareVersionIndex][pin];

  if(mask == 0)
  {
    return LOW;
  }

  return (*ioPort[hardwareVersionIndex][pin] & mask) != 0;
}

void analogWrite(uint8_t pin, uint16_t value)
{
  set_pwm(pin, true, value);
}

#define IIC_CHANNEL_INTERNAL        0
#define IIC_CHANNEL_EXTERNAL        1

#if EEPROM_24xx256 || EEPROM_24xx08

bool eepromRead(uint32_t address, void* data, uint16_t length)
{
  return eeprom_read(IIC_CHANNEL_INTERNAL, address, data, length);
}

bool eepromWrite(uint32_t address, const void* data, uint16_t length)
{
  return eeprom_write(IIC_CHANNEL_INTERNAL, address, data, length);
}

#endif

static void set_pwm(uint8_t channel, bool enable, uint8_t level)
{
  switch(hardwareVersionIndex)
  {
    case 0:
      switch(channel)
      {
        case 3: // RC1/ECCP2
          if(enable)
          {
            TRISCbits.TRISC1 = 0;
            LATCbits.LATC1 = 0;
            CCP2CON = 0x0c;
            CCPR2L = level;
          }
          else
          {
            CCP2CON = 0x00;
            CCPR2L = 0;
          }
          break;

        case 4: // RC2/ECCP1
          if(enable)
          {
            TRISCbits.TRISC2 = 0;
            LATCbits.LATC2 = 0;
            CCP1CON = 0x0c;
            CCPR1L = level;
          }
          else
          {
            CCP1CON = 0x00;
            CCPR1L = 0;
          }
          break;

        case 5: // RD1/ECCP3
          if(enable)
          {
            TRISDbits.TRISD1 = 0;
            LATDbits.LATD1 = 0;
            CCP3CON = 0x0c;
            CCPR3L = level;
          }
          else
          {
            CCP3CON = 0x00;
            CCPR3L = 0;
          }
          break;
      }
      break;

    case 1:
    case 2:
      switch(channel)
      {
        case 0: // RC2/ECCP1
          if(enable)
          {
            TRISCbits.TRISC2 = 0;
            LATCbits.LATC2 = 0;
            CCP1CON = 0x0c;
            CCPR1L = level;
          }
          else
          {
            CCP1CON = 0x00;
            CCPR1L = 0;
          }
          break;

        case 1: // RC1/ECCP2
          if(enable)
          {
            TRISCbits.TRISC1 = 0;
            LATCbits.LATC1 = 0;
            CCP2CON = 0x0c;
            CCPR2L = level;
          }
          else
          {
            CCP2CON = 0x00;
            CCPR2L = 0;
          }
          break;

        case 2: // RD1/ECCP3
          if(enable)
          {
            TRISDbits.TRISD1 = 0;
            LATDbits.LATD1 = 0;
            CCP3CON = 0x0c;
            CCPR3L = level;
          }
          else
          {
            CCP3CON = 0x00;
            CCPR3L = 0;
          }
          break;

        case 3: // RD2/CCP4
          if(enable)
          {
            TRISDbits.TRISD2 = 0;
            LATDbits.LATD2 = 0;
            CCP4CON = 0x0c;
            CCPR4L = level;
          }
          else
          {
            CCP4CON = 0x00;
            CCPR4L = 0;
          }
          break;

        case 4: // RG4/CCP5
          if(enable)
          {
            TRISGbits.TRISG4 = 0;
            LATGbits.LATG4 = 0;
            CCP5CON = 0x0c;
            CCPR5L = level;
          }
          else
          {
            CCP5CON = 0x00;
            CCPR5L = 0;
          }
          break;
      }
      break;
  }
}
