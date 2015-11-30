#include "unabto/unabto_env_base.h"

#if TUNNEL_USE_OPENSSL_SHA256

#include <openssl/sha.h>
#define TUNNEL_SHA256_DIGEST_LENGTH SHA256_DIGEST_LENGTH
#define tunnel_sha256_ctx SHA256_CTX
#define tunnel_sha256_init SHA256_Init
#define tunnel_sha256_update SHA256_Update
#define tunnel_sha256_final(a,b) SHA256_Final(b,a)

#elif TUNNEL_USE_GENERIC_SHA256

#include <modules/crypto/generic/unabto_sha256.h>
#define TUNNEL_SHA256_DIGEST_LENGTH SHA256_DIGEST_LENGTH
#define tunnel_sha256_ctx sha256_ctx
#define tunnel_sha256_init unabto_sha256_init
#define tunnel_sha256_update unabto_sha256_update
#define tunnel_sha256_final unabto_sha256_final

#else

#error Either set TUNNEL_USE_OPENSSL_SHA256 or TUNNEL_USE_GENERIC_SHA256

#endif
