/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_LOGGING_MCHIP_H_
#define _UNABTO_LOGGING_MCHIP_H_

#include "unabto_env_base.h"

#if UNABTO_PLATFORM_PIC18
#include <devices/pic18/debug_channel.h>
#endif

#if NABTO_ENABLE_LOGGING == 1

#if NABTO_ENABLE_LOG_TIMESTAMP == 1
#define print_timestamp() printf("%10"PRIu32" ", TickConvertToMilliseconds(TickGet()));
#else
#define print_timestamp()
#endif

#if NABTO_ENABLE_LOG_FILE_INFO == 1
#define print_filename() printf("%"PRItext"(%i) ", __FILE__, __LINE__);
#else
#define print_filename()
#endif

#define NABTO_BASIC_PRINTC(level, cmsg) do { print_timestamp(); print_filename(); printf cmsg; printf("\n"); } while(0)

#endif


#endif
