#include <tomcrypt.h>
#include <unabto/unabto_external_environment.h>

static prng_state unabto_prng_state;

void nabto_random(uint8_t* buf, size_t len) {
    static bool isInitialized = false;
    size_t bytes;
    if (!isInitialized) {
        int wprng;
        int status;
        register_prng(&fortuna_desc);
        wprng = find_prng("fortuna");

        status = rng_make_prng(128, wprng, &unabto_prng_state, NULL);
        if (status != CRYPT_OK) {
            NABTO_LOG_FATAL(("Could not initialize random function"));
            return;
        }
        isInitialized = true;
    }
    bytes = fortuna_desc.read(buf, len, &unabto_prng_state);

    if (bytes != len) {
        NABTO_LOG_FATAL(("Random function did not give required bytes"));
    }
}
