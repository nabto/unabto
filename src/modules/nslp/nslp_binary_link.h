/*
 * Copyright (C) 2008-2014 Nabto - All Rights Reserved.
 */
#ifndef _NSLP_BINARY_LINK_H_
#define _NSLP_BINARY_LINK_H_

#include <unabto_platform_types.h>

void nslp_binary_link_start_frame(void);
void nslp_binary_link_write_to_frame(uint8_t value);
void nslp_binary_link_end_frame(void);
uint16_t nslp_binary_link_receive_frame(uint8_t** frame);

#endif
