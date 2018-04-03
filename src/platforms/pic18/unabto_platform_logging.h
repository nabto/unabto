/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/*
 * Provides the low level output functions used by the logging system.
 * Supports either "default printf" functionality (using UART) or the debug channel.
 */

#ifndef _UNABTO_PLATFORM_LOGGING_H_
#define _UNABTO_PLATFORM_LOGGING_H_

#define NABTO_FATAL_EXIT do { Reset() } while(0)

#if NABTO_ENABLE_LOGGING

#include <debug_channel.h>

#ifndef NABTO_ENABLE_LOG_TIMESTAMP
#define NABTO_ENABLE_LOG_TIMESTAMP                                  0
#endif

#ifndef NABTO_ENABLE_LOG_FILENAME
#define NABTO_ENABLE_LOG_FILENAME                                   0
#endif

#ifndef NABTO_ENABLE_LOG_MESSAGE
#define NABTO_ENABLE_LOG_MESSAGE                                    1
#endif

// determine what the log output preamble looks like
#if NABTO_ENABLE_LOG_TIMESTAMP && NABTO_ENABLE_LOG_FILENAME
#  define print_log_preamble() do { printf("\n%10" PRIu32 " %" PRItext ":%u  ", TickConvertToMilliseconds(TickGet()), clean_filename(__FILE__), __LINE__); } while(0)
#elif NABTO_ENABLE_LOG_TIMESTAMP
#  define print_log_preamble() do { printf("\n%10" PRIu32 "  ", TickConvertToMilliseconds(TickGet())); } while(0)
#elif NABTO_ENABLE_LOG_FILENAME
#  if NABTO_USE_DEBUG_CHANNEL
#    define print_log_preamble() do { debug_channel_write('\n'); debug_channel_write_string_pgm(clean_filename(__FILE__ ":")); debug_channel_write_int32(__LINE__); debug_channel_write_string_pgm("  "); } while(0)
#  else
#    define print_log_preamble() do { printf("\n%" PRItext ":%u  ", clean_filename(__FILE__), __LINE__); } while(0)
#  endif
#else
#  define print_log_preamble() do { debug_channel_write('\n'); } while(0)
#endif


// determine if the log message should be printed or not

#if NABTO_ENABLE_LOG_MESSAGE
#define print_log_message(message) do { printf message; } while(0)
#else
#define print_log_message(message)
#endif


// create the most efficient way of outputting the log message depending on whether the debug channel is selected or not.

#if NABTO_USE_DEBUG_CHANNEL
#define NABTO_LOG_BASIC_PRINT(severity, message) do { print_log_preamble(); print_log_message(message); } while(0)
#else
#define NABTO_LOG_BASIC_PRINT(severity, message) do { print_log_preamble(); print_log_message(message); } while(0)
#endif

#endif

#endif
