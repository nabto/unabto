#ifndef _UNABTO_TYPES_H_
#define _UNABTO_TYPES_H_

#include <unabto/unabto_env_base.h>

enum {
    FINGERPRINT_LENGTH = 16,
    PSK_ID_LENGTH = 16,
    PSK_LENGTH = 16,
    FCM_TOKEN_LENGTH = 255
};

struct unabto_fingerprint {
    uint8_t data[FINGERPRINT_LENGTH];
};

// fingerprint of a public key
struct unabto_optional_fingerprint {
    struct unabto_fingerprint value;
    bool hasValue;
};

struct unabto_psk_id {
    uint8_t data[PSK_ID_LENGTH];
};

// ID of a psk.
struct unabto_optional_psk_id {
    struct unabto_psk_id value;
    bool hasValue;
};

// PSK
struct unabto_psk {
    uint8_t data[PSK_LENGTH];
};

struct unabto_optional_psk {
    struct unabto_psk value;
    bool hasValue;
};

struct unabto_fcm_token {
    char data[FCM_TOKEN_LENGTH];
};

struct unabto_optional_fcm_token {
    struct unabto_fcm_token value;
    bool hasValue;
};

#endif
