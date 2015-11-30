/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _TCP_RELAY_H_
#define _TCP_RELAY_H_

#ifdef __cplusplus
extern "C" {
#endif

/** The socket type. */
typedef uint8_t nabto_socket_t;

/** Denoting an invalid socket */
#define NABTO_INVALID_SOCKET 255

void tcp_relay_initialize(void);
void tcp_relay_tick(void);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
