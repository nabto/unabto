/*
 * Copyright (C) 2008-2015 Nabto - All Rights Reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include "unabto_provision.h"

struct curl_fetch_st {
    char *payload;
    size_t size;
};

char *filePathPtr;

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

bool set_unabto_id(nabto_main_setup *nms, char *id)
{
    char *nabtoId = malloc(sizeof(char) * strlen(id));
    if (!nabtoId) {
        NABTO_LOG_ERROR(("Failed to allocate id"));
        return false;
    }

    sprintf(nabtoId, "%s", id);
    nms->id = nabtoId;
    return true;
}

bool validate_string(char delim, char *string) {
    size_t i, count = 0;
    for (i = 0; i < strlen(string); i++) {
        count += (string[i] == delim);
    }
    return count == 1 && string[i-1] != delim && string[0] != delim;
}

size_t curl_callback (void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct curl_fetch_st *p = (struct curl_fetch_st *) userp;

    // Expand buffer and check
    p->payload = (char *) realloc(p->payload, p->size + realsize + 1);
    if (p->payload == NULL) {
        NABTO_LOG_ERROR(("Failed to expand buffer in curl_callback"));
        return -1;
    }

    // Copy contents to buffer and ensure null termination
    memcpy(&(p->payload[p->size]), contents, realsize);
    p->size += realsize;
    p->payload[p->size] = 0;

    return realsize;
}

bool read_file(char *text, size_t size)
{
    FILE *fp = fopen(filePathPtr, "r");
    if (!fp) {
        return false;
    }
    fgets(text, size, fp);
    fclose(fp);
    return true;
}

bool write_file(char *text, size_t size)
{
    FILE *fp = fopen(filePathPtr, "wb");
    if (!fp) {
        return false;
    }
    fprintf(fp, "%.*s", (int)size, text);
    fclose(fp);
    return true;
}

bool unabto_provision_persistent(nabto_main_setup *nms, char *url, char *filePath)
{
    filePathPtr = filePath;
    return unabto_provision_persistent_using_handles(nms,
                                                     url,
                                                     (unabtoPersistentFunction)read_file,
                                                     (unabtoPersistentFunction)write_file);
}

bool unabto_provision_persistent_using_handles(nabto_main_setup *nms,
                                               char *url,
                                               unabtoPersistentFunction readFunc,
                                               unabtoPersistentFunction writeFunc)
{
    char *ch;
    char text[128] = {0};
    size_t i = 0, len = 0;

    // If persistent file exists, use it
    if (readFunc(text, sizeof(text))) {
        return unabto_provision_parse(nms, text);
    }

    NABTO_LOG_INFO(("Provisioning file not found. Creating one..."));

    if (!unabto_provision(nms, url)) {
        return false;
    }

    len = sprintf(text, "%s%s", nms->id, ":");
    for (i = 0; i < sizeof(nms->presharedKey)/sizeof(nms->presharedKey[0]); i++) {
        len += sprintf(text + len, "%02x", nms->presharedKey[i]);
    }

    if (!writeFunc(text, sizeof(text))) {
        NABTO_LOG_ERROR(("Unable to write persistent data"));
        return false;
    }
    return true;
}
