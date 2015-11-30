/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
/**
 * @file
 * Includes device based environment after possibly detecting and setting the device.
 */

#ifndef _UNABTO_INCLUDE_PLATFORM_H_
#define _UNABTO_INCLUDE_PLATFORM_H_

/**
 * Include definitions for the target platform.
 * Platform is determined either by manually defining the appropriate symbol (UNABTO_PLATFORM_<platform_name>) or by using auto detection based on compiler flags.
 */

#if UNABTO_PLATFORM_CUSTOM
#    include <unabto_platform.h>
#elif UNABTO_PLATFORM_UNABTO_APP
#    include <platforms/unabto_app/unabto_platform.h>
#elif UNABTO_PLATFORM_WIN32 || WIN32
#    include <platforms/win32/unabto_platform.h>
#elif UNABTO_PLATFORM_ARDUINO
#    include <platforms/arduino/unabto_env_base_arduino.h>
#elif UNABTO_PLATFORM_PIC32_FREERTOS
#    include <platforms/pic32_freertos/unabto_env_base_pic32_freertos.h>
#elif UNABTO_PLATFORM_PIC32 || __XC32
#    include <platforms/pic32/unabto_platform.h>
#elif UNABTO_PLATFORM_PIC18 || __18CXX
#    include <platforms/pic18/unabto_platform.h>
#elif UNABTO_PLATFORM_COLDFIRE
#    include <platforms/coldfire/unabto_env_base_coldfire.h>
#elif UNABTO_PLATFORM_UNIX || __unix || __unix__ || __APPLE__
#    include <platforms/unix/unabto_platform.h>
#elif UNABTO_PLATFORM_MQX
#    include <unabto_env_base_mqx.h>
#elif UNABTO_PLATFORM_LWIP
#    include <platforms/freertos_lwip/unabto_env_base_freertos_lwip.h>
#elif UNABTO_PLATFORM_RTX4100
#    include <platforms/rtx4100/unabto_env_base_rtx4100.h>
#elif UNABTO_PLATFORM_GAINSPAN
#    include <platforms/gainspan/unabto_env_base_gainspan.h>
#elif UNABTO_PLATFORM_RX600
#    include <platforms/renesas_rx600/unabto_env_base_renesas_rx600.h>
#elif UNABTO_PLATFORM_FREERTOS_NET
#    include <platforms/freertos_net/unabto_env_base_freertos_net.h>
#elif UNABTO_PLATFORM_RENESAS_RL78
#    include <platforms/renesas_rl78/unabto_env_base_renesas_rl78.h>
#elif UNABTO_PLATFORM_EVOLUTION
#    include <platforms/evolution/unabto_env_base_evolution.h>
#else
#    include <unabto_platform.h>
#endif

#endif
