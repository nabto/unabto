/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
/**
 * @file
 * The Basic environment for the Nabto Micro Device Server, Interface.
 *
 * This file holds definitions, declarations and prototypes to be supplied by the host.
 */

#ifndef _UNABTO_ENV_BASE_H_
#define _UNABTO_ENV_BASE_H_

#include <stddef.h>

#include <unabto_config.h>                   // user supplied config file, defaults are provided below
#include <unabto/unabto_include_platform.h>  // device detection (if not set by user) and device dependant includes
#include <unabto/unabto_config_defaults.h>   // default configuration values
#include <unabto/unabto_config_derived.h>    // calculation of constants based on the configuration values
#include <unabto/unabto_config_check.h>      // Check that the configuration seems valid.

#include <unabto/unabto_logging.h>

/// Avoid compiler warning. @param x  The not used parameter. @return
#define NABTO_NOT_USED(x) do { (void)(x); } while (0) // if (x) { /* avoid compiler warning */ }

/// Asserts expr is true, expr is only evaluated when compiling.
#define COMPILE_TIME_ASSERT(expr) switch(0){case 0:case expr:;}

/* Utilities for time handling. */
/* The utilities are used in a "poll-driven" system. */

/**
 * Add interval (in milleseconds) to timestamp
 * @param stamp  pointer to the time stamp
 * @param msec   interval to add to the time stamp, in milliseconds
 * void nabtoAddStamp(nabto_stamp_t* stamp, uint32_t msec);
 */
#ifndef nabtoAddStamp
#define nabtoAddStamp(stamp, msec)    do { *(stamp) += nabtoMsec2Stamp(msec); } while (0)
#endif

/**
 * Add interval (in nanoseconds) to timestamp
 * @param stamp  pointer to the time stamp
 * @param msec   interval to add to the time stamp, in nanoseconds
 * void nabtoAddStamp(nabto_stamp_t* stamp, uint64_t nsec);
 */
#ifndef nabtoAddStampNs
#define nabtoAddStampNs(stamp, nsec) do { *(stamp) += nabtoMsec2Stamp(nsec/1000000); } while (0)
#endif

/**
 * Set timestamp to a future time.
 * @param stamp  pointer to the time stamp
 * @param msec   interval to add to the current time stamp, in milliseconds
 * void nabtoSetFutureStamp(nabto_stamp_t* stamp, uint32_t msec);
 */
#ifndef nabtoSetFutureStamp
#define nabtoSetFutureStamp(stamp, msec) do { *(stamp) = nabtoGetStamp(); nabtoAddStamp(stamp, msec); } while (0)
#endif

/**
 * Set timestamp to a future time.
 * @param stamp  pointer to the time stamp
 * @param msec   interval to add to the current time stamp, in nanoseconds
 * void nabtoSetFutureStamp(nabto_stamp_t* stamp, uint64_t nsec);
 */
#ifndef nabtoSetFutureStampNs
#define nabtoSetFutureStampNs(stamp, nsec) do { *(stamp) = nabtoGetStamp(); nabtoAddStampNs(stamp, nsec); } while (0)
#endif


/**
 * if (*s1 < *s2) return -1;
 * if (*s1 > *s2) return 1;
 * return 0;
 */
#ifndef nabtoCompareStamp
#define nabtoCompareStamp(s1, s2) (nabtoStampLess(s1, s2) ? -1 : (nabtoStampLess(s2, s1) ? 1 : 0))
#endif

/** @return (*s1 < *s2) */
#ifndef nabtoStampLess
#define nabtoStampLess(s1,s2) (*s1 < *s2)
#endif


#ifndef WIN32
#ifndef UNABTO_INADDR_NONE
#define UNABTO_INADDR_NONE (0xffffffffu)
#endif
#ifndef UNABTO_INADDR_ANY
#define UNABTO_INADDR_ANY 0x00000000u
#endif
#endif

//#include <unabto/unabto_external_environment.h>


typedef char _nabtoDummyType; // Some compilers (e.g. the MPLAB C18 compiler for PIC18) don't accept C files with no statements. This typedef solves this.

/**
 * Compare a constant string (possibly in ROM) to the contents of a buffer
 * @param romString  the string
 * @param ramString  the buffer
 * @return           depending on the implementation (don't use)
 */
#ifndef textcmp
#   define textcmp(romString, ramString) strcmp(romString, ramString)
#endif

/**
 * Copy a constant string (possibly in ROM) to a buffer
 * @param destination         the destination buffer
 * @param destinationSize     size of the destination buffer
 * @param source              the source buffer
 * @return                    depending on the implementation (don't use)
 */
#ifndef textcpy
#   define textcpy(destination, source, destinationSize) strncpy(destination, source, destinationSize)
#endif

#define NABTO_STRINGIFY(x) _NABTO_STRINGIFY(x)
#define _NABTO_STRINGIFY(x) #x

#define NABTO_CONCATENATE(x, y) _NABTO_CONCATENATE(x, y)
#define _NABTO_CONCATENATE(x, y) x ## y

#endif
