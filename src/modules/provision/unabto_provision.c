/*
 * Copyright (C) 2008-2015 Nabto - All Rights Reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unabto_config.h"

#if NABTO_ENABLE_PROVISIONING

#include "unabto_provision.h"
#include "unabto_provision_http.h"
#include "unabto_provision_file.h"

bool unabto_provision_new(nabto_main_setup* nms, provision_context_t* context) {
    // fail early (prior to invoking service) if filesystem trouble
    if (!unabto_provision_test_create_file(context->file_)) {
        NABTO_LOG_FATAL(("Error creating provisioning file '%s'", context->file_));
        return false;
    }
    
    char key[KEY_BUFFER_SIZE];
    unabto_provision_status_t status = unabto_provision_http(nms, context, key);
    if (status == UPS_OK) {
        return unabto_provision_set_key(nms, key) &&
            unabto_provision_write_file(context->file_, nms);
    } else {
        switch (status) {
        case UPS_PROV_ALREADY_PROVISIONED:
            NABTO_LOG_FATAL(("Device is already provisioned - use service administration interface to re-open for new attempt"));
            break;
        case UPS_PROV_INVALID_TOKEN:
            NABTO_LOG_FATAL(("Invalid user token specified to provisioning service - check token or contact customer service"));
            break;
        default:
            NABTO_LOG_FATAL(("Provisoning failed with status [%d]", status));
            break;
        }
        return false;
    }
}

bool unabto_provision_set_key(nabto_main_setup *nms, char *key)
{
    size_t i;
    size_t pskLen = strlen(key);

    if (!key || pskLen != PRE_SHARED_KEY_SIZE * 2) {
        NABTO_LOG_ERROR(("Invalid key: %s", key));
        return false;
    }

    // Read the pre shared key as a hexadecimal string.
    for (i = 0; i < pskLen/2 && i < 16; i++) {
        sscanf(key+(2*i), "%02hhx", &nms->presharedKey[i]);
    }
    nms->secureAttach= true;
    nms->secureData = true;
    nms->cryptoSuite = CRYPT_W_AES_CBC_HMAC_SHA256;
    return true;
}

bool unabto_provision_try_existing(nabto_main_setup *nms, provision_context_t* context)
{
    char text[256];
    if (!unabto_provision_read_file(context->file_, text, sizeof(text))) {
        return false;
    }
    char key[PRE_SHARED_KEY_SIZE * 2];
    if (!unabto_provision_parse_data(nms, text, key)) {
        return false;
    }
    if (context->id_ && strcmp(nms->id, context->id_) != 0) {
        NABTO_LOG_WARN(("Specified id is different than existing id - ignoring existing and treating as new"));
        return false;
    }
    return unabto_provision_set_key(nms, key);
}

#endif
