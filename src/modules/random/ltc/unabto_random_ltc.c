#include <tomcrypt.h>
#include <unabto/unabto_external_environment.h>

static prng_state unabto_prng_state;

void nabto_random(uint8_t* buf, size_t len) {
    static bool isInitialized = false;
    if (!isInitialized) {
        register_prng(&fortuna_desc);
        int wprng = find_prng("fortuna");
        
        int status = rng_make_prng(128, wprng, &unabto_prng_state, NULL);
        if (status != CRYPT_OK) {
            NABTO_LOG_FATAL(("Could not initialize random function"));
            return;
        }
        isInitialized = true;
    }
    size_t bytes = fortuna_desc.read(buf, len, &unabto_prng_state);

    if (bytes != len) {
        NABTO_LOG_FATAL(("Random function did not give required bytes"));
    }
}
