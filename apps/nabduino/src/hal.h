#ifndef _HAL_H_
#define _HAL_H_

#include <unabto/unabto_env_base.h>

#define LOW         0
#define HIGH        1

#define INPUT       0
#define OUTPUT      1

extern uint16_t hardwareVersion;
extern uint8_t hardwareVersionIndex;

void hal_initialize(void);
void hal_tick(void);

bool ledRead(void);
void ledWrite(bool value);

uint8_t buttonRead(void);

uint16_t analogRead(uint8_t pin);
void analogWrite(uint8_t pin, uint16_t value);

int16_t temperatureRead(void);

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t value);
uint8_t digitalRead(uint8_t pin);

#if EEPROM_24xx256 || EEPROM_24xx08

bool eepromRead(uint32_t address, void* data, uint16_t length);
bool eepromWrite(uint32_t address, const void* data, uint16_t length);

#endif

#endif
