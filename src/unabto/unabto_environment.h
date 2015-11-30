/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
/**
 * @file
 * The environment for the Nabto Micro Device Server, Interface.
 *
 * This file holds definitions, declarations and prototypes to be supplied by the host.
 */

#ifndef _UNABTO_ENVIRONMENT_H_
#define _UNABTO_ENVIRONMENT_H_

#include <unabto/unabto_env_base.h>

#ifdef __cplusplus
extern "C" {
#endif
//#include <unabto/unabto_external_environment.h>

/******************************************************************************/
/******************************************************************************/

/** test endpoints for equality. @param ep1 endpoint @param ep2 endpoint @return true if equal */
#define EP_EQUAL(ep1, ep2)  (((ep1).addr == (ep2).addr) && ((ep1).port == (ep2).port))

#ifdef __cplusplus
} //extern "C"
#endif

#endif
