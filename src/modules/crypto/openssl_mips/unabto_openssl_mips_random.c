#include <unabto/unabto_util.h>
#include <unabto/unabto_external_environment.h>
#include <unabto/unabto_logging.h>

#include "unabto_openssl_mips_aes.h"

enum {
    UNABTO_PRNG_COUNTER_BYTE_LENGTH = 16
};

typedef struct {
    AES_KEY key;
    uint8_t counter[UNABTO_PRNG_COUNTER_BYTE_LENGTH];
} unabto_prng_state;

static unabto_prng_state prng_state;

void unabto_prng_inc_counter(unabto_prng_state* state);
void unabto_prng_init_state(unabto_prng_state* state);
void unabto_prng_generate_block(unabto_prng_state* state, uint8_t* buf, size_t bytes);

void unabto_prng_random_seed(uint8_t* buf, size_t bytes)
{
    unsigned long x;
    FILE *f;
    f = fopen("/dev/urandom", "rb");
    if (f == NULL)
       f = fopen("/dev/random", "rb");
    if (f == NULL) {
        NABTO_LOG_FATAL(("Could not open random source tried /dev/urandom and /dev/random"));
        return;
    }

    /* disable buffering */
    if (setvbuf(f, NULL, _IONBF, 0) != 0) {
       fclose(f);
       NABTO_LOG_FATAL(("Could not disable buffering"));
       return;
    }   
 
    x = (unsigned long)fread(buf, 1, bytes, f);
    fclose(f);

    if (x < bytes) {
        NABTO_LOG_FATAL(("Could not get enough random entropy"));
        return;
    }
}

void nabto_random(uint8_t* buf, size_t bytes) {
    static bool unabto_prng_is_initialized = false;
    
    if (!unabto_prng_is_initialized) {
        unabto_prng_init_state(&prng_state);
        unabto_prng_is_initialized = true;
    }

    while (bytes > 0) {
        size_t bytesToGenerate = MIN(bytes, 16);
        unabto_prng_generate_block(&prng_state, buf, bytesToGenerate);
        buf += bytesToGenerate;
        bytes -= bytesToGenerate;
    }
}

/**
 * PRECONDITION bytes <= 16
 */
void unabto_prng_generate_block(unabto_prng_state* state, uint8_t* buf, size_t bytes) {
    unabto_prng_inc_counter(state);

    uint8_t r[16];
    AES_encrypt(state->counter, r, &state->key);
    
    memcpy(buf, r, bytes);
}

void unabto_prng_init_state(unabto_prng_state* state) {
    uint8_t aeskey[16];
    unabto_prng_random_seed(aeskey, 16);
    unabto_prng_random_seed(state->counter, UNABTO_PRNG_COUNTER_BYTE_LENGTH);

    private_AES_set_encrypt_key(aeskey, 128, &state->key);
    
}

void unabto_prng_inc_counter(unabto_prng_state* state) {
    int i = 0;
    for (i = 0; i < UNABTO_PRNG_COUNTER_BYTE_LENGTH; i++) {
        state->counter[i]++;
        if (state->counter[i] != 0) {
            break;
        }
    }
}
