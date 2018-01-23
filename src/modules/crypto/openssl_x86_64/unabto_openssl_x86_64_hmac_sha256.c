#include <unabto/unabto_hmac_sha256.h>

#include <string.h>

#include "unabto_openssl_x86_64_sha256.h"

static SHA256_CTX sha_ctx;
static uint8_t block_pad[SHA256_BLOCK_LENGTH];

//void print_sha256_ctx(sha256_ctx* ctx);

void unabto_hmac_sha256_buffers(const unabto_buffer keys[], uint8_t keys_size,
                                const unabto_buffer messages[], uint8_t messages_size,
                                uint8_t *mac, uint16_t mac_size)
{
    uint16_t fill = 0;
    uint16_t num;
    const uint8_t *key_used;
    uint8_t i;
    uint8_t key_temp[SHA256_BLOCK_LENGTH];
    uint8_t digest_temp[SHA256_DIGEST_LENGTH];

    uint16_t key_size = 0;
    for (i = 0; i < keys_size; i++) {
        key_size += keys[i].size;
    }


    if (key_size <= SHA256_BLOCK_LENGTH) {
        uint8_t* ptr = key_temp;
        for (i = 0; i < keys_size; i++) {
            memcpy(ptr, (const void*)keys[i].data, keys[i].size);
            ptr += keys[i].size;
        }
        key_used = (const uint8_t*) key_temp;
        num = key_size;
    } else { // key_size > SHA256_BLOCK_LENGTH
        num = SHA256_DIGEST_LENGTH;
        //NABTO_LOG_TRACE(("key size %i", key_size));
        //NABTO_LOG_BUFFER(key, key_size);
        SHA256_Init_unabto(&sha_ctx);
        for (i = 0; i < keys_size; i++) {
            SHA256_Update_unabto(&sha_ctx, keys[i].data, keys[i].size);
        }
        SHA256_Final_unabto(key_temp, &sha_ctx);
        
        //sha256(key, key_size, key_temp);
        //NABTO_LOG_BUFFER(key_temp, SHA256_DIGEST_LENGTH);
        key_used = (const unsigned char*)key_temp;
    }
    fill = SHA256_BLOCK_LENGTH - num;

    memset(block_pad + num, 0x36, fill);
    for (i = 0; i < num; i++) {
        block_pad[i] = key_used[i] ^ 0x36;
    }
//    print_sha256_ctx(sha_ctx);
    SHA256_Init_unabto(&sha_ctx);
//    print_sha256_ctx(sha_ctx);
    SHA256_Update_unabto(&sha_ctx, block_pad, SHA256_BLOCK_LENGTH);
//    print_sha256_ctx(sha_ctx);
    for (i = 0; i < messages_size; i++) {
        SHA256_Update_unabto(&sha_ctx, messages[i].data, messages[i].size);
    }
//    print_sha256_ctx(sha_ctx);
    SHA256_Final_unabto(digest_temp, &sha_ctx);
//    print_sha256_ctx(sha_ctx);
    memset(block_pad + num, 0x5c, fill);
    for (i = 0; i < num; i++) {
        block_pad[i] = key_used[i] ^ 0x5c;
    }
   
    SHA256_Init_unabto(&sha_ctx);
//    print_sha256_ctx(sha_ctx);
    SHA256_Update_unabto(&sha_ctx, block_pad, SHA256_BLOCK_LENGTH);
//    print_sha256_ctx(sha_ctx);
    SHA256_Update_unabto(&sha_ctx, digest_temp, SHA256_DIGEST_LENGTH);
//    print_sha256_ctx(sha_ctx);
    SHA256_Final_unabto(digest_temp, &sha_ctx);
    
    //UNABTO_ASSERT(mac_size <= 32 );
    memcpy(mac, (void*)digest_temp, mac_size);
}
