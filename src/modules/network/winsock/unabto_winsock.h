/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_WINSOCK_H_
#define _UNABTO_WINSOCK_H_

#include <winsock2.h>
#include <unabto_platform_types.h>
/** The socket type. */
typedef SOCKET nabto_socket_t;

/** Denoting an invalid socket */
#define NABTO_INVALID_SOCKET INVALID_SOCKET

#ifdef __cplusplus
extern "C" {
#endif

bool unabto_winsock_initialize(void);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
