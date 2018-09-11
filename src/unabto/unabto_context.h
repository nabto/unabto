/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
/**
 * @file
 * Nabto uServer Context - Interface.
 *
 * Handling attach packets.
 */

#ifndef _UNABTO_CONTEXT_H_
#define _UNABTO_CONTEXT_H_

#include <unabto/unabto_env_base.h>
#include <unabto/unabto_crypto.h>
#include <unabto/unabto_external_environment.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Magic values */
enum {
    NONCE_SIZE     = 32,
    NONCE_SIZE_OLD = 8,
    SEED_SIZE      = 32
};


/** A set of peer endpoints */
typedef struct {
    nabto_endpoint privateEndpoint; /**< private endpoint */
    nabto_endpoint globalEndpoint; /**< global endpoint  */
} ipxdata;


/** for simple sum verification (testing structure and flow) */
typedef struct {
    uint32_t sum; /**< the verification accumulator */
} verification_t;


/** The Nabto Attach Communication Context (incl communication buffer). */
typedef struct nabto_context_s {
    nabto_stamp_t         timestamp;               /**< time of next relevant action    */
    int                   counter;                 /**< event counter                   */
    int                   errorCount;              /**< error transitions since success */
    nabto_state           state;                   /**< the state                       */

    size_t                nonceSize;               /**< current nonce size              */
    uint8_t               nonceMicro[NONCE_SIZE];  /**< my own attach nonce             */
    uint8_t               natType;                 /**< The type of nat we are behind.  */
    bool                  clearTextData;           /**< true if data is sent unecrypted */

    verification_t        verif1;                  /**< dummy verification UD->GSP      */
    verification_t        verif2;                  /**< dummy verification UD->GSP->UD  */

    uint32_t              gspnsi;                  /**< the NSI iID of the GSP,
                                                             identifies the attachment       */
    nabto_endpoint        gsp;                     /**< the GSP endpoint                */
    nabto_endpoint        globalAddress;           /**< our global IP address           */

    unabto_buffer*        piggyBuffer;             /**< Buffer to store piggyback data
                                                             if a resend is required.        */
    uint16_t              piggyOldHeaderSequence;  /**< The old hdr seq number to
                                                             determine if the message should
                                                             be sent again                   */
    bool                  useDnsFallback;          /**< True if packets should be sent via dns fallback */
    bool                  hasDnsFallbackSocket;    /**< True if a dns fallback socket has been opened */
    bool                  keepAliveReceived;       /**< True if a keepalive has been received */

    /*****************************************************************************************/
    /* fields below has specific init, reinit and release methods.                           */

#if NABTO_ENABLE_REMOTE_ACCESS || NABTO_ENABLE_UCRYPTO
    nabto_crypto_context* cryptoAttach;            /**< context for U_ATTACH packets    */
    nabto_crypto_context* cryptoConnect;           /**< context for U_CONNECT + U_ALIVE */
#endif
} nabto_context;

/******************************************************************************/
/**
 * Initialise the Nabto Communication Context
 * @return false if buffers cannot be allocated.
 */
bool nabto_context_init(void);

/******************************************************************************/
/**
 * Re-initialise the Nabto Communication Context
 */
void nabto_context_reinit(void);


/**
 * Re-initialise the crypto relevant part of the attach context.
 */
void nabto_context_reinit_crypto(void);
    
/******************************************************************************/
/**
 * Release and reset the Nabto Communication Context
 */
void nabto_context_release(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
