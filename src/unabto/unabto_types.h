#ifndef _UNABTO_TYPES_H_
#define _UNABTO_TYPES_H_

#include <unabto/unabto_env_base.h>

enum {
    FINGERPRINT_LENGTH = 16,
    PSK_ID_LENGTH = 16,
    PSK_LENGTH = 16
};

// fingerprint of a public key
struct unabto_fingerprint {
    uint8_t value[FINGERPRINT_LENGTH];
    bool hasValue;
};

// ID of a psk.
struct unabto_psk_id {
    uint8_t value[PSK_ID_LENGTH];
    bool hasValue;
};

// PSK
struct unabto_psk {
    uint8_t value[PSK_LENGTH];
    bool hasValue;
};

#endif
