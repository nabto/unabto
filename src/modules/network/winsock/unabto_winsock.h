/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_WINSOCK_H_
#define _UNABTO_WINSOCK_H_

#include <winsock2.h>

/** The socket type. */
typedef SOCKET nabto_socket_t;

/** Denoting an invalid socket */
#define NABTO_INVALID_SOCKET INVALID_SOCKET

#ifdef __cplusplus
extern "C" {
#endif

static bool unabto_winsock_initialize(void);

uint16_t nabto_read_events(nabto_socket_t* sockets, uint16_t maxSockets, int timeout);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
