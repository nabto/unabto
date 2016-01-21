/*
 * Copyright (C) 2008-2015 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_PROVISION_H
#define _UNABTO_PROVISION_H

/**
 * Coarse grained facade to device provisioning functions - to have
 * cryptographic key and optionally device id issued from central
 * service at runtime (for devices capable of running a full https
 * stack).
 */

#include "unabto/unabto_common_main.h"

/**
 * Struct to be populated with project / customer specific data
 * provided by Nabto.
 */
typedef struct {
    const uint8_t* scheme_;   // http/https
    const uint8_t* host_;     // provisioning webservice hostname 
    const uint8_t* api_key_;  // apikey to obtain access to webservices
    const uint8_t* token_;    // optional one-time token (activation code)
    const uint8_t* id_;       // device id input as configured for project (e.g. mac address)
    const uint8_t* file_;     // where to store output
} provision_context_t;

/**
 * Attempt to read provisining file specified in context to setup
 * device id and crypto key in nms struct. Returns true iff both were
 * successfully read and set.
 */
bool unabto_provision_try_existing(nabto_main_setup *nms, provision_context_t* context);

/**
 * Invoke provision service as specified in context, write id and key
 * issued by service to file and insert into nms struct. Returns true
 * iff service was invoked and id and key were successfully written
 * and set.
 *
 * BUG: Truncates existing file on service error (NABTO-1170)
 *
 */
bool unabto_provision_new(nabto_main_setup* nms, provision_context_t* context);

/**
 * Insert key into nms struct.
 */
bool unabto_provision_set_key(nabto_main_setup *nms, char *key);


#endif
