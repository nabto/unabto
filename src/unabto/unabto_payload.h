#ifndef _NABTO_PAYLOAD_H_
#define _NABTO_PAYLOAD_H_

#include <unabto/unabto_env_base.h>

// Payload decoded from a packet
struct unabto_payload_packet {
    uint8_t  type;
    uint8_t  flags;
    uint16_t length;
    uint8_t* dataBegin;
    uint8_t* dataEnd;
};

// capabilities structure which can be sent.
struct unabto_capabilities {
    uint8_t  type;
    uint32_t flags;
    uint32_t mask;
    uint16_t noCodes;
    uint16_t code;
};

// structure which can be written or read from a packet.  The
// encrypotion codes is stored elsewhere. On the stack or in the
// received buffer.
struct unabto_payload_capabilities {
    uint8_t  type;
    uint32_t flags;
    uint32_t mask;
    uint16_t noCodes;
    uint16_t *codes;
};

/**
 * read a single payload from the packet. return the pointer to where
 * the next payload begins. On error return NULL.
 */
uint8_t* unabto_read_payload(uint8_t* begin, uint8_t* end, struct unabto_payload_packet* payload);

void unabto_capabilities_add_mask(struct unabto_payload_capabilities* capabilities, uint32_t mask);
void unabto_capabilities_add_bits(struct unabto_payload_capabilities* capaboilites, uint32_t bits);

#endif
