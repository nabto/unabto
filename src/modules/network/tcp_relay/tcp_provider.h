/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _TCP_PROVIDER_H_
#define _TCP_PROVIDER_H_

#include <unabto/unabto_env_base.h>

#if MATCHPORT
#include "matchport_bg_tcp_provider.h"
#else
#include "winsock_tcp_provider.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

void tcp_provider_initialize(void);
void tcp_provider_tick(void);
void tcp_provider_shutdown(void);
bool tcp_provider_connect(tcp_socket* tcpSocket, text host, uint16_t port);
void tcp_provider_disconnect(tcp_socket* tcpSocket);
uint16_t tcp_provider_read(tcp_socket* tcpSocket, uint8_t* buffer, uint16_t maximumLength);
void tcp_provider_write(tcp_socket* tcpSocket, uint8_t* buffer, uint16_t length);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
