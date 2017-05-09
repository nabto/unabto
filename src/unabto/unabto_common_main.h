/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_COMMON_MAIN_H_
#define _UNABTO_COMMON_MAIN_H_

#include <unabto/unabto_main_contexts.h>

/**
 * General unabto usage from an applications perspective.
 *
 * 1. Initialize the context of unabto and get a pointer to the
 * setup structure.
 * nabto_main_setup* nms = unabto_init_context();
 *
 * 2. Make the neccessary setup of unabto.
 * nms->id = strdup("testdevice.tld");
 *
 * 3. Initialize unabto by opening sockets etc.
 * unabto_init();
 *
 * 4. Here you have two choices either run unabto polled or run unabto
 * event driven. The two approaches is decribed by pseudo code below.
 * 
 * The polled approach which is the simplest would be implemented like
 *
 * while(true) {
 *     unabto_tick();
 * }
 * 
 * The event driven method waits for events. Once an event occurs the
 * correct part of uNabto is called.
 *
 * while(true) {
 *     nabto_stamp_t next_event;
 *     unabto_next_event(&next_event);
 *     wait for which ever comes first data on a socket or the next event.
 *     if (data was received on the local socket) {
 *         unabto_read_socket(local_socket);
 *     }
 *     if (data was received on the remote socket) {
 *         unabto_read_socket(remote_socket);
 *     }
 *     if (the next event should occur) {
 *         unabto_time_event();
 *     }
 * }
 *
 * 5. If you want to close unabto call unabto_close()
 */

/**
 * If you need encrypotion in your application you need to set it up
 * in step 2.
 * 
 * nms->cryptoSuite = CRYPT_W_AES_CBC_HMAC_SHA256
 * nms->secureAttach = 1;
 * nms->secureData = 1;
 * memcpy(&nms->presharedKey, yourKey, PRE_SHARED_KEY_SIZE);
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the unabto context and get a pointer
 * back to the setup structure where device name and other
 * uNabto options can be set.
 * @return a pointer to the setup context.
 */
nabto_main_setup* unabto_init_context(void);

/**
 * Main initializer for nabto.
 * This both initializes nabto main context and the nabto subsystem.
 * @return true if ok.
 */
bool unabto_init(void);
/**
 * Fully closes nabto
 * @param nmc  the nabto main context
 */
void unabto_close(void);

/**
 * Give nabto a tick.
 * Nabto should have ticks fairly often to ensure timers and messages
 * are handled properly. Fairly often is every 10ms for most applications.
 */
void unabto_tick(void);

bool unabto_init_nms_crypto(nabto_main_setup* nms, bool secureAttach, bool secureData, crypto_suite crypt, uint8_t* preSharedKey, size_t pskLength);

/**
 * Read and handle data on a single socket.
 * called from an event driven implementation to inform that
 * nabto_read should be called on the specific socket.
 */
bool unabto_read_socket(nabto_socket_t socket);

/**
 * When a timed event or polled data is ready
 * call this function
 */
void unabto_time_event(void);

/**
 * Initialize the nabto_main_setup to default values
 */
void unabto_init_default_values(nabto_main_setup* nms);

/**
 * Is uNabto connected to the gsp
 * @return true if connected
 */
bool unabto_is_connected_to_gsp(void);

/**
 * @return a reference to the nabto main context.
 */
nabto_main_context* unabto_get_main_context(void);

/**
 * Notify uNabto about changes in the network configuration.
 * Unabto will use this information to reset certain timers
 * if we are currently in the initial connect to the gsp etc.
 * @param ip  Ip address in hostbyte order.
 */
void unabto_notify_ip_changed(uint32_t ip);


#if NABTO_ENABLE_NEXT_EVENT
/**
 * return stamp for next event.
 */
void unabto_next_event(nabto_stamp_t* stamp);
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif
