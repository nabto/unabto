/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_PROTOCOL_EXCEPTIONS_H_
#define _UNABTO_PROTOCOL_EXCEPTIONS_H_

/**
 * @file
 * Nabto protocol exceptions - error codes in exception packets.
 */

/*********************** WARNING WARNING WARNING ******************************
** This file is included in both the micro world and the big nabto world.    **
** That is the content of this file is used by all compilers for the micro   **
** devices and the compilers for the base station and client software.       **
**
** The definitions should mainly consist of defines for the C preprocessor.  **
******************************************************************************/

/* Exception codes - always positive - must *never* be changed! */
#define NP_E_NOT_READY         3  ///< the application isn't ready yet (still initialising)
#define NP_E_NO_ACCESS         4  ///< the application doesn't allow this request to be answered
#define NP_E_TOO_SMALL         5  ///< the request is too small, i.e. required fields aren't present
#define NP_E_TOO_LARGE         6  ///< the request is too large
#define NP_E_INV_QUERY_ID      7  ///< the request queryId is invalid
#define NP_E_RSP_TOO_LARGE     8  ///< the response is larger than the space allocated
#define NP_E_OUT_OF_RESOURCES  9  ///< no ressources available (out of memory)
#define NP_E_SYSTEM_ERROR     10  ///< internal error
#define NP_E_NO_QUERY_ID      11  ///< the request is so small that no queryId is present

#endif
