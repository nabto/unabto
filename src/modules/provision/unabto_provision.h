/*
 * Copyright (C) 2008-2015 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_PROVISION_H
#define _UNABTO_PROVISION_H

#include "unabto/unabto_common_main.h"

bool unabto_provision_set_key(nabto_main_setup *nms, char *key);


// define NABTO_ENABLE_PROVISIONING to add revelvant options to default gopt arg list

bool unabto_provision_parse(nabto_main_setup *nms, char *text);
bool unabto_provision_parse_json(nabto_main_setup *nms, char *json);

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
bool unabto_provision_persistent(nabto_main_setup *nms, char *url, char *filePath);

/**
 * Definition of read and write function handles
 * @param text        fill with characters / write content to persistent storage
 * @param size        max size / size of text
 * return             true if successful
 */
typedef bool (*unabtoPersistentFunction)(char *text, size_t size);

/**
 * Request provisioning information from url, populate nabto_main_setup and
 * write information using function handles.
 * If persistent values exists this information is used directly.
 * @param nms         nabto_main_setup to populate
 * @param url         full url of shared secrets web service including
 *                    queries (token, mac, id)
 * @param readFunc    pointer to function that reads value from persistent storage
 * @param writeFunc   pointer to function that writes to persistent storage
 * return             true if successful
 */
bool unabto_provision_persistent_using_handles(nabto_main_setup *nms,
                                               char *url,
                                               unabtoPersistentFunction readFunc,
                                               unabtoPersistentFunction writeFunc);

#endif
