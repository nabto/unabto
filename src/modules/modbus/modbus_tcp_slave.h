/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _MODBUS_TCP_SLAVE_H_
#define _MODBUS_TCP_SLAVE_H_

#include <unabto/unabto_env_base.h>

#ifdef __cplusplus
extern "C" {
#endif

bool modbus_tcp_slave_initialize(void);
void modbus_tcp_slave_tick(void);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
