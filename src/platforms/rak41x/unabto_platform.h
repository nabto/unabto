/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_PLATFORM_H_
#define _UNABTO_PLATFORM_H_

// Define the symbol that can be used to determine what platform uNabto is currently being built for.
#ifndef UNABTO_PLATFORM_RAK
#define UNABTO_PLATFORM_RAK 1
#endif

#include <unabto_platform_types.h>
#include <os.h>
#include <rak_api.h>
#include <unabto_platform_logging.h>

// This macro is missing in the MPLAB C18 compiler and so is provided by the platform instead.
#define lengthof(x) (sizeof((x)) / sizeof((x)[0]))

// Enter a critical section.
#define critical_section_enter() __disable_irq()
// Exit a critical section.
#define critical_section_exit() __enable_irq()

#define NABTO_INVALID_SOCKET        255
//typedef uint8_t nabto_socket_t;
typedef struct nabto_socket_t nabto_socket_t;

enum nabto_socket_type {
    NABTO_SOCKET_IP_V4,
    NABTO_SOCKET_IP_V6
};

struct nabto_socket_t {
    uint8_t sock;
    enum nabto_socket_type type;
};

typedef uint32_t nabto_stamp_t;
typedef int32_t nabto_stamp_diff_t;

extern char* wifiSsid;
extern char* wifiKey;

#define nabtoMsec2Stamp(milliseconds)   (milliseconds)

void platform_initialize_pre(void);
bool platform_initialize(void* tcb);
void platform_tick(void);

#endif
