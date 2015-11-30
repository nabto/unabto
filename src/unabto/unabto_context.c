/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
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

#endif
