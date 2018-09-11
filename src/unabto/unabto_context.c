/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/**
 * @file
 * Nabto uServer Context - Implementation.
 */

#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_UNABTO

#include "unabto_context.h"
#include "unabto_main_contexts.h"
#include "unabto_memory.h"
#include "unabto_external_environment.h"
#include "unabto_attach.h" //nabto_change_context_state

#include <string.h>

#if NABTO_ENABLE_REMOTE_ACCESS

/**
 * Initialise the Nabto Communication Context
 * @param ctx       the context
 * @param nmc       the main context
 * @param buffer    the buffer
 */
static void nabto_ctx_reset(void)
{
    memset(&nmc.context, 0, offsetof(nabto_context, cryptoAttach)); // don't touch the crypto context pointers
    nmc.context.timestamp = nabtoGetStamp();
} 

/******************************************************************************/

bool nabto_context_init(void)
{
    nabto_ctx_reset();
    nabto_context_reinit_crypto();
    return true;
}

/******************************************************************************/

void nabto_context_reinit(void)
{
    nabto_change_context_state(NABTO_AS_IDLE);
    nabto_ctx_reset();
    nabto_context_reinit_crypto();
}

void nabto_context_reinit_crypto(void)
{
    nabto_crypto_release(nmc.context.cryptoConnect);
    nabto_random(nmc.context.nonceMicro, sizeof(nmc.context.nonceMicro));
    NABTO_LOG_INFO(("SECURE ATTACH: %i, DATA: %i", (int)nmc.nabtoMainSetup.secureAttach, (int)nmc.nabtoMainSetup.secureData));
    if (nmc.nabtoMainSetup.secureAttach && nmc.nabtoMainSetup.cryptoSuite != CRYPT_W_NULL_DATA) {
        nmc.context.nonceSize      = NONCE_SIZE;
        nmc.context.clearTextData = !nmc.nabtoMainSetup.secureData;
    } else {
        nmc.context.nonceSize      = NONCE_SIZE_OLD;
        nmc.context.clearTextData = true;
    }
    NABTO_LOG_INFO(("NONCE_SIZE: %u, CLEAR_TEXT: %i", (unsigned int) nmc.context.nonceSize, (int)nmc.context.clearTextData));
    nabto_crypto_reinit_a();
}

/******************************************************************************/

void nabto_context_release(void)
{
    nabto_change_context_state(NABTO_AS_IDLE);
    memset(&nmc.context, 0, offsetof(nabto_context, cryptoAttach));
    nabto_crypto_release(nmc.context.cryptoAttach);
    nabto_crypto_release(nmc.context.cryptoConnect);
}

const char* nabto_context_ip_to_string(const struct nabto_ip_address* addr)
{
    static char output[20]; //0000:1111:2222:3333:4444:5555:6666:7777:8888\0"
    memset(output, 0, 20);
    if (addr->type == NABTO_IP_NONE) {
        sprintf(output, "NABTO_IP_NONE");
    } else if (addr->type == NABTO_IP_V4) {
        uint32_t ip = addr->addr.ipv4;
        sprintf(output, "%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8, (uint8_t)(ip >> 24), (uint8_t)(ip >> 16), (uint8_t)(ip >> 8), (uint8_t)(ip));
    } else {
        const uint8_t* ip;
        sprintf(output, "%02X%02X:" "%02X%02X:" "%02X%02X:" "%02X%02X:" "%02X%02X:" "%02X%02X:" "%02X%02X:" "%02X%02X", ip[0], ip[1], ip[2], ip[3], ip[4], ip[5], ip[6], ip[7], ip[8], ip[9], ip[10], ip[11], ip[12], ip[13], ip[14], ip[15]);
    }
    return output;
}

#endif
