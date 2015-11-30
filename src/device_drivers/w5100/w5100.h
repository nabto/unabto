/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _W5100_H_
#define _W5100_H_

#include <unabto_platform_types.h>

#define W5100_INVALID_SOCKET        0xff
typedef uint8_t w5100_socket;

//#include <unabto/unabto_external_environment.h>

//#define NABTO_INVALID_SOCKET W5100_INVALID_SOCKET
//#define nabto_socket_t w5100_socket

#ifdef __cplusplus
extern "C" {
#endif

void w5100_initialize(void);
void w5100_set_mac_address(const uint8_t* mac);
void w5100_set_local_address(uint32_t ip);
void w5100_set_gateway_address(uint32_t ip);
void w5100_set_netmask(uint32_t mask);
bool w5100_udp_open(w5100_socket* socket, uint16_t* sourcePort);
void w5100_udp_close(w5100_socket* socket);
bool w5100_udp_send(w5100_socket* socket, const uint8_t* buffer, uint16_t length, uint32_t ip, uint16_t port);
uint16_t w5100_udp_receive(w5100_socket* socket, uint8_t* buffer, uint16_t maximumLength, uint32_t* sourceIp, uint16_t* sourcePort);

// uNabto compatible UDP socket operations
bool w5100_nabto_init_socket(uint32_t localAddr, uint16_t* localPort, w5100_socket* socket);
void w5100_nabto_close_socket(w5100_socket* socket);
ssize_t w5100_nabto_write(w5100_socket socket, const uint8_t* buf, size_t len, uint32_t addr, uint16_t port);
ssize_t w5100_nabto_read(w5100_socket socket, uint8_t* buf, size_t len, uint32_t* addr, uint16_t* port);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
