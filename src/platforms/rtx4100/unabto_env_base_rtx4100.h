/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef UNABTO_ENV_BASE_RTX4100
#define UNABTO_ENV_BASE_RTX4100
/**
 * @file
 * The basic environment for uNabto RTX4100.
 */

#include "unabto_platform_types.h"

#include <Core/RtxCore.h>
#include <Ros/RosCfg.h>
#include <PortDef.h>
#include <Api/Api.h>

#include <stdarg.h>

#include "modules/timers/tick_timer/unabto_tick_timer.h"
#include "platforms/unabto_common_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//typedef ApiSocketHandleType nabto_socket_t;
typedef struct nabto_socket_t nabto_socket_t;

enum nabto_socket_type {
    NABTO_SOCKET_IP_V4,
    NABTO_SOCKET_IP_V6
};

struct nabto_socket_t {
    ApiSocketHandleType sock;
    enum nabto_socket_type type;
};

typedef int ssize_t;

#define NABTO_INVALID_SOCKET 0

bool platform_initialize();

void rtx4100_log(const char* fmt, ...);
void rtx4100_shell_log(const char* fmt, ...);

#define NABTO_DEFAULT_LOG_MODULE(level,cmsg) do { rtx4100_shell_log cmsg; rtx4100_shell_log("\r\n"); } while (0)

// No logging atm
#define NABTO_LOG_CHECK(level) 0

// This is a buggy implementation, fatal should restart nabto
#define NABTO_FATAL_EXIT  while(0)
#define NABTO_FATAL(cmsg)


#ifdef __cplusplus
} //extern "C"
#endif

#endif
