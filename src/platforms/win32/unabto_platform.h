/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_PLATFORM_WIN32_H_
#define _UNABTO_PLATFORM_WIN32_H_

/**
 * @file
 * The Basic environment for the Nabto Micro Device Server (PC device), Interface.
 *
 * This file holds definitions, declarations and prototypes to be supplied by the host.
 */

// Define the symbol that can be used to determine what platform uNabto is currenlty being built for.
#ifndef UNABTO_PLATFORM_WIN32
#define UNABTO_PLATFORM_WIN32 1
#endif

#include <WinSock2.h>
#include <Windows.h>
#include <platforms/win32/unabto_platform_types.h>
#include <platforms/win32/unabto_logging_win32.h>

#if NABTO_TCP_RELAY_ENABLE
#include <modules/network/tcp_relay/tcp_relay.h>
#else
#include <modules/network/winsock/unabto_winsock.h>
#endif

/************ The Time Handling basics (PC Device), Interface *************/

/** The timestamp definition. @return */
typedef UINT64  nabto_stamp_t;
typedef int64_t nabto_stamp_diff_t;

/**
 * Convert milleseconds to nabto_stamp_t difference (duration)
 * @param msec  number of milliseconds
 * @return      the stamp difference
 */
#define nabtoMsec2Stamp(msec)   (msec)

#ifndef lengthof
#define lengthof(x) (sizeof((x)) / sizeof((x)[0]))
#endif

//#define copy_string(destination, destinationSize, source) strcpy_s(destination, destinationSize, source)
//#define copy_text(destination, destinationSize, source) strcpy_s(destination, destinationSize, source)

//#define textcmp(romString, ramString) strcmp(romString, ramString)
//#define textcpy(destination, destinationSize, source) strcpy_s(destination, destinationSize, source)

#endif
