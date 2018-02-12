/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_PLATFORM_H_
#define _UNABTO_PLATFORM_H_

// Define the symbol that can be used to determine what platform uNabto is currently being built for.
#ifndef UNABTO_PLATFORM_PIC18
#define UNABTO_PLATFORM_PIC18 1
#endif

// Override default text functions with PIC18 specific versions (the PIC has strings stored in program memory which required special instructions to reach).
#define textcmp(romString, ramString) strcmpram2pgm(romString, ramString)
#define textcpy(destination, source, destinationSize) strncpypgm2ram(destination, source, destinationSize)

#include <unabto_platform_types.h>
#include <unabto_platform_logging.h>

#if USE_W5100
#include <modules/network/w5100/w5100_network.h>
#else
#include <modules/network/microchip/unabto_microchip_network.h>
#endif

#if NABTO_USE_DEBUG_CHANNEL
#include <debug_channel.h>
#endif

#include <modules/network/microchip/unabto_microchip_time.h>

// This macro is missing in the MPLAB C18 compiler and so is provided by the platform instead.
#define lengthof(x) (sizeof((x)) / sizeof((x)[0]))

// Determine what should happen on exit. The exit function is called when a fatal error occures in uNabto.
#define exit(exitCode) do { Reset(); } while(0)

// Enter a critical section.
#define critical_section_enter() uint8_t __gieStore = INTCON & 0xc0; INTCON &= 0x3f;
// Exit a critical section.
#define critical_section_exit() INTCON |= __gieStore;

// Override default read/write macros with PIC18 optimized version.
#define READ_U8(variable, buffer)           do { (variable) = *((uint8_t*)(buffer)); } while (0)
#define READ_U16(variable, buffer)          do { (variable) = swaps(*(uint16_t*)(buffer)); } while (0)
#define READ_U32(variable, buffer)          do { (variable) = swapl(*(uint32_t*)(buffer)); } while(0)
#define WRITE_U8(buffer, variable)          do { *((uint8_t*)(buffer)) = (variable); } while (0)
#define WRITE_U16(buffer, variable)         do { *(uint16_t*)(buffer) = swaps(variable); } while(0)
#define WRITE_U32(buffer, variable)         do { *(uint32_t*)(buffer) = swapl(variable); } while(0)
#define WRITE_32(dst, s32)                  WRITE_U32(dst, (uint32_t)(s32))

bool platform_initialize(void);
void platform_tick(void);

#define PLATFORM_LED_A                    0
#define PLATFORM_LED_B                    1
// Determines which if any Ethernet LED to control from the application. Default is none.
void platform_configure_led(uint16_t mask);
void platform_control_led(bool state);

const far rom char* clean_filename(const far rom char* path);

#endif
