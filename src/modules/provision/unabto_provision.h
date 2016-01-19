/*
 * Copyright (C) 2008-2015 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_PROVISION_H
#define _UNABTO_PROVISION_H

#include "unabto/unabto_common_main.h"

typedef struct {
    const uint8_t* scheme_;
    const uint8_t* host_;
    const uint8_t* api_key_;
    const uint8_t* token_;
    const uint8_t* id_;
    uint8_t* key_;
} provision_context_t;

bool unabto_provision_set_key(nabto_main_setup *nms, char *key);

// define NABTO_ENABLE_PROVISIONING to add revelvant options to default gopt arg list

/**
 * Request provisioning information from url, populate nabto_main_setup and
 * write information to persistent file.
 * If file exists and is valid the information inside is used directly.
 * @param nms         nabto_main_setup to populate
 * @param url         full url of shared secrets web service including
 *                    queries (token, mac, id)
 * @param filePath    path to persistent file for reading/writing
 *                    provisioning information
 * return             true if successful
 */
bool unabto_provision_try_existing(nabto_main_setup *nms,
                                   provision_context_t* context,
                                   const char* path);

bool unabto_provision_execute(nabto_main_setup* nms, provision_context_t* context);

#endif
