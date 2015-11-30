/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
/**
 * @file
 * The environment for the Nabto Micro Device Server, Interface.
 *
 * This file holds various logging declarations.
 */

#ifndef _UNABTO_LOGGING_H_
#define _UNABTO_LOGGING_H_

#include <unabto_platform_types.h>
#include <unabto/unabto_logging_defines.h>

// if NABTO_LOG_ALL is defined to 1 log module and level filters will be overriden and all log messages will be output
#ifndef NABTO_LOG_ALL
#define NABTO_LOG_ALL                                               0ul
#endif


#ifndef NABTO_ENABLE_LOGGING
#define NABTO_ENABLE_LOGGING                                        1ul
#endif

#ifndef NABTO_FATAL_EXIT
#include <stdlib.h>
#define NABTO_FATAL_EXIT                                            exit(1)
#endif



// Module filter - default is to show log output from all modules.
// Set NABTO_LOG_MODULE_FILTER to a combination of the NABTO_LOG_MODULE_... bit masks.
#ifndef NABTO_LOG_MODULE_FILTER
#define NABTO_LOG_MODULE_FILTER                                     NABTO_LOG_MODULE_ALL
#endif

#ifndef NABTO_LOG_MODULE_CURRENT
#define NABTO_LOG_MODULE_CURRENT                                    NABTO_LOG_MODULE_DEFAULT
#endif

// Check whether the current module should have logging enabled or not.
#ifndef NABTO_LOG_MODULE_CHECK
#define NABTO_LOG_MODULE_CHECK                                      ((NABTO_LOG_MODULE_CURRENT) & (NABTO_LOG_MODULE_FILTER))
#endif


// Severity filter
// Set NABTO_LOG_SEVERITY_FILTER to a combination of the NABTO_LOG_SEVERITY_... bit masks and/or levels.
// Default to info level if none is specified.
#ifndef NABTO_LOG_SEVERITY_FILTER
#define NABTO_LOG_SEVERITY_FILTER                                  NABTO_LOG_SEVERITY_LEVEL_INFO
#endif

// Check whether the logging at the specified severity level is enabled.
#define NABTO_LOG_SEVERITY_CHECK(severity)                          ((severity) & (NABTO_LOG_SEVERITY_FILTER))

// If fine grained log filtering is used set NABTO_LOG_ALL_FATALS to 1 to enable fatal events from all modules regardless of other filtering.
#ifndef NABTO_LOG_ALL_FATALS
#define NABTO_LOG_ALL_FATALS                                        1
#endif

// Check if logging is enabled in this module and at this severity level. Overriden by the NABTO_LOG_ALL flag.
#ifndef NABTO_LOG_CHECK
#define NABTO_LOG_CHECK(severity)                                   ((NABTO_ENABLE_LOGGING) && ((NABTO_LOG_ALL) || (NABTO_LOG_ALL_FATALS && (severity & NABTO_LOG_SEVERITY_FATAL)) || ((NABTO_LOG_MODULE_CHECK) && (NABTO_LOG_SEVERITY_CHECK(severity)))))
#endif


/**
 * The actual log output functions.
 * C versions of logging macros, should be used for crossplatform logging.
 * They work like printf exept you have to give an extra enclosing paranthesis
 * to support varargs
 * E.g. NABTO_LOG_TRACE(("hello number %i", number));
 */

#ifndef NABTO_LOG_FATAL
#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_FATAL)
#define NABTO_LOG_FATAL(message)                                    do { NABTO_LOG_BASIC_PRINT(NABTO_LOG_SEVERITY_FATAL, message); NABTO_FATAL_EXIT; } while(0)
#else
#define NABTO_LOG_FATAL(message)                                    do { NABTO_FATAL_EXIT; } while(0)
#endif
#endif

#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_ERROR)
#define NABTO_LOG_ERROR(message)                                    NABTO_LOG_BASIC_PRINT(NABTO_LOG_SEVERITY_ERROR, message)
#else
#define NABTO_LOG_ERROR(message)
#endif

#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_WARN)
#define NABTO_LOG_WARN(message)                                     NABTO_LOG_BASIC_PRINT(NABTO_LOG_SEVERITY_WARN, message)
#else
#define NABTO_LOG_WARN(message)
#endif

#ifndef NABTO_LOG_INFO
#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_INFO)
#define NABTO_LOG_INFO(message)                                     NABTO_LOG_BASIC_PRINT(NABTO_LOG_SEVERITY_INFO, message)
#else
#define NABTO_LOG_INFO(message)
#endif
#endif

#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_DEBUG)
#define NABTO_LOG_DEBUG(message)                                    NABTO_LOG_BASIC_PRINT(NABTO_LOG_SEVERITY_DEBUG, message)
#else
#define NABTO_LOG_DEBUG(message)
#endif

#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_TRACE)
#define NABTO_LOG_TRACE(message)                                    NABTO_LOG_BASIC_PRINT(NABTO_LOG_SEVERITY_TRACE, message)
#else
#define NABTO_LOG_TRACE(message)
#endif

#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_STATISTICS)
#define NABTO_LOG_STATISTIC(message)                                NABTO_LOG_BASIC_PRINT(NABTO_LOG_SEVERITY_STATISTICS, message)
#else
#define NABTO_LOG_STATISTIC(message)
#endif

/**
 * Logs a printf type string and a buffer given by a buffer and a length
 * The printf style string should be enclosed in parantheses such that the
 * logging macros doesn't have to handle variable length list of arguments.
 * E.g. NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_TRACE, ("The buffer in the state: %i", state), buf, buf_len);
 */
#ifndef NABTO_LOG_BUFFER
#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_BUFFERS)
#include <platforms/unabto_printf_logger.h>

#define NABTO_LOG_BUFFER(severity, message, buffer, length)            do { if (NABTO_LOG_CHECK(severity)) { NABTO_LOG_BASIC_PRINT(severity, message); log_buffer(buffer, length); } } while (0)
#else
#define NABTO_LOG_BUFFER(severity, message, buffer, length)
#endif
#endif

#ifndef UNABTO_ASSERT
#  if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_FATAL) || !UNABTO_PLATFORM_PIC18
#    define UNABTO_ASSERT(expr)  do { if (!(expr)) { NABTO_LOG_FATAL(("Assertion fails: %" PRItext, #expr)); } } while(0)
#  else // no room for asserts in release builds on the PIC18
#    define UNABTO_ASSERT(expr)
#  endif
#endif

#ifndef NABTO_VERIFY_COMPILE_TIME
#define NABTO_VERIFY_COMPILE_TIME(expression) typedef char NABTO_CONCATENATE(___NABTO_VERIFY_COMPILE_TIME_AT_LINE_, __LINE__)[(expression) ? 1 : -1]
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifndef PRINT_ENDPOINT
#define PRINT_ENDPOINT                                              1
#endif

#if PRINT_ENDPOINT != 1

// If end points should not be printed insert an empty string instead.

#ifdef MAKE_EP_PRINTABLE
#undef MAKE_EP_PRINTABLE
#endif
#define MAKE_EP_PRINTABLE(ep) "?"

#ifdef PRIep
#undef PRIep
#endif
#define PRIep         "%" PRItext

#endif

#endif /*DOXYGEN_SHOULD_SKIP_THIS*/

#endif
