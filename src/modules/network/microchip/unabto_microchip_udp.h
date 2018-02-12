/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_MICROCHIP_UDP_H_
#define _UNABTO_MICROCHIP_UDP_H_

#include <TCPIP Stack/TCPIP.h>
#include <unabto_platform_types.h>

#ifdef __cplusplus
extern "C" {
#endif

ssize_t microchip_udp_write(UDP_SOCKET socket,
                            const uint8_t* buf,
                            size_t         len,
                            uint32_t       addr,
                            uint16_t       port);

ssize_t microchip_udp_read(UDP_SOCKET socket,
                           uint8_t*       buf,
                           size_t         len,
                           uint32_t*      addr,
                           uint16_t*      port);

void microchip_udp_close(UDP_SOCKET* socketDescriptor);

bool microchip_udp_open(uint32_t localAddr, uint16_t* localPort, UDP_SOCKET* socketDescriptor);

#ifdef __cplusplus
} //extern "C"
#endif

#endif
