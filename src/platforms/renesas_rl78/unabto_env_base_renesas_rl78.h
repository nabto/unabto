/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_ENV_BASE_RENESAS_RL78_H_
#define _UNABTO_ENV_BASE_RENESAS_RL78_H_
/**
 * @file
 * The basic environment for uNabto Renesas RL78 - Interface.
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <platforms/unabto_common_types.h>

typedef long ssize_t;                   ///< ssize_t definition
//typedef uint8_t nabto_socket_t;         ///< Handle for UDP stack
typedef struct nabto_socket_t nabto_socket_t;

enum nabto_socket_type {
    NABTO_SOCKET_IP_V4,
    NABTO_SOCKET_IP_V6
};

struct nabto_socket_t {
    uint8_t sock;
    enum nabto_socket_type type;
};
typedef long long nabto_stamp_t;        ///< Nabto time stamp
typedef long long nabto_stamp_diff_t;

#define NABTO_INVALID_SOCKET    NULL
#define nabtoMsec2Stamp(msec)   (msec)
#define NABTO_FATAL_EXIT        exit(1);


#endif
