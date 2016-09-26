#ifndef UNABTO_CONFIG_H
#define UNABTO_CONFIG_H

/**
 * This header contains the specific Nabto settings for this project.
 * All available settings can be read about in "unabto_config_defaults.h".
 * The default value will be used if a specific setting is not set here.
 */

#define NABTO_ENABLE_STREAM               0
#define NABTO_CONNECTIONS_SIZE            4
#define NABTO_SET_TIME_FROM_ALIVE         0
#if CRYPTO_DEMO
#define NABTO_ENABLE_UCRYPTO              1
#else
#define NABTO_ENABLE_UCRYPTO              0
#endif
#define NABTO_ENABLE_LOCAL_ACCESS_LEGACY_PROTOCOL    0
#define NABTO_ENABLE_LOGGING              0

#if __XC32
#include "modules/network/microchip/unabto_microchip_time.h"
//#include "modules/timers/freertos/unabto_time_freertos.h"
#endif

#endif
