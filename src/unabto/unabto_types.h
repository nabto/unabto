#ifndef _UNABTO_TYPES_H_
#define _UNABTO_TYPES_H_

#include <unabto/unabto_env_base.h>

// Truncated sha256 fingerprint of a public key.
typedef uint8_t unabto_public_key_fingerprint[16];

// ID of a psk.
typedef uint8_t unabto_psk_id[16];

// PSK
typedef uint8_t unabto_psk[16];

#endif
