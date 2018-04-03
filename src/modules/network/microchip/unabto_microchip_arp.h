/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_MICROCHIP_ARP_H_
#define _UNABTO_MICROCHIP_ARP_H_

#include <unabto/unabto_env_base.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This function is called from the main loop.
 * Makes the actual arp resolution.
 */
void unabto_microchip_arp_tick(void);

/**
 * @param ip    The ip to lookup, in network byte order since microchip uses network byte order.
 * @param node  The node info in response.
 * @return true If the ARP resolution succedes.
 */
bool unabto_microchip_arp_resolve(const uint32_t ipNetworkOrder, MAC_ADDR* node);


/**
 * This function is called when a network packets arrives to update
 * the cache with arp information about the sender ip, mac.
 * @param node  The node info to add to the cache
 */
void unabto_microchip_arp_add_resolved(NODE_INFO* node);

/**
 * reset the arp cache
 */
void unabto_microchip_arp_reset(void);


#ifdef __cplusplus
} //extern "C"
#endif

#endif
