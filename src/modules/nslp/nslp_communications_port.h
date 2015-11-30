/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#ifndef _NSLP_COMMUNICATIONS_PORT_H_
#define _NSLP_COMMUNICATIONS_PORT_H_

#include <unabto_platform_types.h>
#include <uart.h>

#if _WIN32

#define nslp_communications_port_can_read()                         uart_can_read(0)
#define nslp_communications_port_can_write()                        uart_can_write(0)
#define nslp_communications_port_read()                             uart_read(0)
#define nslp_communications_port_read_buffer(buffer, length)        uart_read_buffer(0, buffer, length)

// use caching write on Win32
//#define nslp_communications_port_write(value)                       uart_write(0, value)
//#define nslp_communications_port_write_buffer(buffer, length)       uart_write_buffer(0, buffer, length)
void nslp_communications_port_write(uint8_t value);
void nslp_communications_port_write_buffer(const void* buffer, uint16_t length);

#else 

#define nslp_communications_port_can_read()                         uart_can_read()
#define nslp_communications_port_can_write()                        uart_can_write()
#define nslp_communications_port_read()                             uart_read()
#define nslp_communications_port_read_buffer(buffer, length)        uart_read_buffer(buffer, length)
#define nslp_communications_port_write(value)                       uart_write(value)
#define nslp_communications_port_write_buffer(buffer, length)       uart_write_buffer(buffer, length)

#endif

extern bool nslpCommunicationsPortLock;

#endif
