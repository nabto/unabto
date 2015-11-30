/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NETWORK

#include "unabto_microchip_arp.h"
#include <unabto/unabto_external_environment.h>

#define INVALID_ENTRY 255
#define ARP_CACHE_SIZE 5
#define ARP_EXPIRE_MS (180000ul)

typedef enum {
    IDLE,
    RESOLVING,
    RESOLVED
} arp_entry_state;

typedef struct {
    NODE_INFO node;
    nabto_stamp_t expire_time;
    arp_entry_state state;
} arp_entry;

static arp_entry arp_cache[ARP_CACHE_SIZE];
static uint8_t entry;

void unabto_microchip_arp_tick(void) {
    static nabto_stamp_t timeout;
    arp_entry* ptr;
    uint8_t i;

    if (entry == INVALID_ENTRY) { // if resolver logic is currently idle...
        for (i = 0; i < ARP_CACHE_SIZE; i++) {
            ptr = &arp_cache[i];
            if (ptr->state == RESOLVED && nabtoIsStampPassed(&ptr->expire_time)) { // make the cache entry timeout a period of time after being resolved
                NABTO_LOG_TRACE(("ARP: " PRIip " timed out.", MAKE_IP_PRINTABLE(swapl(ptr->node.IPAddr.Val))));
                ptr->state = IDLE;
            }

            if (ptr->state == RESOLVING) { // start resolving an address if a resolve request has been made
                NABTO_LOG_TRACE(("ARP: Starting resolve of " PRIip ".", MAKE_IP_PRINTABLE(swapl(ptr->node.IPAddr.Val))));
                ARPResolve(&ptr->node.IPAddr);
                nabtoSetFutureStamp(&timeout, 1000);
                entry = i;
                break;
            }
        }
    }

    if (entry != INVALID_ENTRY) { // if resolver logic is currently busy...
        ptr = &arp_cache[entry];

        if (ARPIsResolved(&ptr->node.IPAddr, &ptr->node.MACAddr)) { // if address has been resolved store the result
            NABTO_LOG_TRACE(("ARP: " PRIip " resolved.", MAKE_IP_PRINTABLE(swapl(ptr->node.IPAddr.Val))));
            ptr->state = RESOLVED;
            nabtoSetFutureStamp(&ptr->expire_time, ARP_EXPIRE_MS);
            entry = INVALID_ENTRY;
        } else if (nabtoIsStampPassed(&timeout)) { // if the resolve failed release the cache slot
            NABTO_LOG_TRACE(("ARP: Resolve failed for " PRIip ".", MAKE_IP_PRINTABLE(swapl(ptr->node.IPAddr.Val))));
            ptr->state = IDLE;
            entry = INVALID_ENTRY;
        }
    }
}

uint8_t find_free_slot(void) {
    //    arp_entry* ptr;
    uint8_t i;

    for (i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_cache[i].state == IDLE) {
            return i;
        }

        //        ptr = &arp_cache[i];
        //        if (ptr->state == IDLE) {
        //            return i;
        //        }
    }

    return rand() % ARP_CACHE_SIZE;
}

/**
 * Get either the ip or the gw ip
 * @param ipNetworkOrder
 * @return
 */
uint32_t commIp(uint32_t ipNetworkOrder) {
    if ((AppConfig.MyIPAddr.Val ^ ipNetworkOrder) & AppConfig.MyMask.Val) {
        return AppConfig.MyGateway.Val;
    } else {
        return ipNetworkOrder;
    }
}

bool unabto_microchip_arp_resolve(const uint32_t ipNetworkOrder, MAC_ADDR* mac) {
    arp_entry* ptr;
    uint8_t i;
    uint8_t slot;
    uint32_t gwAddr;
    gwAddr = commIp(ipNetworkOrder);

    for (i = 0; i < ARP_CACHE_SIZE; i++) {
        ptr = &arp_cache[i];
        if (gwAddr == ptr->node.IPAddr.Val && ptr->state != IDLE) { // cache hit
            if (ptr->state == RESOLVED) { // address already resolved
                *mac = ptr->node.MACAddr;
                NABTO_LOG_TRACE(("ARP: Request succeeded for " PRIip ".", MAKE_IP_PRINTABLE(swapl(ptr->node.IPAddr.Val))));
                return true;
            } else if (ptr->state == RESOLVING) { // address is currently being resolved
                NABTO_LOG_TRACE(("ARP: Request failed for " PRIip ".", MAKE_IP_PRINTABLE(swapl(ptr->node.IPAddr.Val))));
                return false;
            }
        }
    }

    // cache miss

    slot = find_free_slot(); // start new resolve by allocating a cache slot

    ptr = &arp_cache[slot];
    ptr->node.IPAddr.Val = gwAddr;
    ptr->state = RESOLVING;

    NABTO_LOG_TRACE(("ARP: Request queued for " PRIip ".", MAKE_IP_PRINTABLE(swapl(ptr->node.IPAddr.Val))));

    return false;
}

void add_resolved_entry(uint8_t index, uint32_t ip, MAC_ADDR* mac) {
    arp_entry* ptr = &arp_cache[index];
    ptr->node.MACAddr = *mac;
    ptr->node.IPAddr.Val = ip;
    ptr->state = RESOLVED;
    nabtoSetFutureStamp(&ptr->expire_time, ARP_EXPIRE_MS);
}

void unabto_microchip_arp_add_resolved(NODE_INFO* node) {
    uint8_t i;
    //    arp_entry* ptr;
    uint32_t commAddr = commIp(node->IPAddr.Val);
    bool found = false;

    for (i = 0; i < ARP_CACHE_SIZE; i++) {
        //        ptr = &arp_cache[i];
        if (arp_cache[i].node.IPAddr.Val == commAddr) {
            NABTO_LOG_TRACE(("ARP: Updating " PRIip ".", MAKE_IP_PRINTABLE(swapl(arp_cache[i].node.IPAddr.Val))));
            found = true;
            break;
        }


        //        ptr = &arp_cache[i];
        //        if (ptr->node.IPAddr.Val == gwAddr) {
        //            break;
        //        }
    }

    if (found == false) {
        i = find_free_slot();
    }

    add_resolved_entry(i, commAddr, &node->MACAddr);

    NABTO_LOG_TRACE(("ARP: Resolved " PRIip ".", MAKE_IP_PRINTABLE(swapl(arp_cache[i].node.IPAddr.Val))));
}

void unabto_microchip_arp_reset(void) {
    memset(arp_cache, 0, sizeof (arp_cache));
    entry = INVALID_ENTRY;
}
