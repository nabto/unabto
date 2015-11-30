/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#ifndef _NSLP_BINARY_TRANSPORT_H_
#define _NSLP_BINARY_TRANSPORT_H_

#include <unabto_platform_types.h>

// See nslp.c for packet format.
#define NSLP_BINARY_TRANSPORT_PACKET_OVERHEAD                     ((1 + 2 + 1 + 1) + (1))

void nslp_binary_transport_initialize(void);
void nslp_binary_transport_reset(void);
void nslp_binary_transport_tick(void);
bool nslp_binary_transport_is_idle(void);

bool nslp_binary_transport_allocate_packet(uint16_t size);
void nslp_binary_transport_write_uint8(uint8_t value);
void nslp_binary_transport_write_uint16(uint16_t value);
void nslp_binary_transport_write_uint32(uint32_t value);
void nslp_binary_transport_write_buffer(const void* buffer, uint16_t length);

uint16_t nslp_binary_transport_receive(uint8_t** packet);

uint8_t nslp_binary_transport_poll_send_status(void);

#endif
