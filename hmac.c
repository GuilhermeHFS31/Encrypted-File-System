#include "hmac.h"

#include <string.h>

#include "sha2.h"

static int constant_time_compare(const uint8_t *a, const uint8_t *b, size_t length)
{
    int result = 0;

    for (size_t i = 0; i < length; i++) {
        result |= a[i] ^ b[i];
    }

    return result == 0;
}

int verify_hmac(const uint8_t *key, size_t key_len,
                const uint8_t *data, size_t data_len,
                const uint8_t *stored_mac)
{
    uint8_t calculated_mac[HMAC_SHA512_HASH_LENGTH];

    hmac_sha512(key, key_len, data, data_len, calculated_mac);

    const int is_valid = constant_time_compare(stored_mac, calculated_mac, HMAC_SHA512_HASH_LENGTH);
    
    memset(calculated_mac, 0, sizeof(calculated_mac));

    return is_valid;
}



void hmac_sha512(const uint8_t *key, size_t key_len, 
                 const uint8_t *data, size_t data_len, 
                 uint8_t *mac_out)
{
    uint8_t key_pad[HMAC_SHA512_BLOCK_LENGTH] = {0};

    if (key_len > HMAC_SHA512_BLOCK_LENGTH) {
        SHA2_CTX key_ctx;

        SHA512Init(&key_ctx);
        SHA512Update(&key_ctx, key, key_len);
        SHA512Final(key_pad, &key_ctx);
        memset(&key_ctx, 0, sizeof(key_ctx));
    } else {
        memcpy(key_pad, key, key_len);
    }

    uint8_t k_ipad[HMAC_SHA512_BLOCK_LENGTH];
    uint8_t k_opad[HMAC_SHA512_BLOCK_LENGTH];

    for (size_t i = 0; i < HMAC_SHA512_BLOCK_LENGTH; i++)
    {
        k_ipad[i] = key_pad[i] ^ 0x36;
        k_opad[i] = key_pad[i] ^ 0x5c;
    }

    SHA2_CTX ctx;
    uint8_t temp_hash[64];

    //H(k_ipad || data)
    SHA512Init(&ctx);
    SHA512Update(&ctx, k_ipad, HMAC_SHA512_BLOCK_LENGTH);
    SHA512Update(&ctx, data, data_len);
    SHA512Final(temp_hash, &ctx);

    //H(k_opad || temp_hash)
    SHA512Init(&ctx);        
    SHA512Update(&ctx, k_opad, HMAC_SHA512_BLOCK_LENGTH);
    SHA512Update(&ctx, temp_hash, HMAC_SHA512_HASH_LENGTH);
    SHA512Final(mac_out, &ctx);            

    memset(key_pad, 0, sizeof(key_pad));
    memset(k_ipad, 0, sizeof(k_ipad));
    memset(k_opad, 0, sizeof(k_opad));
    memset(temp_hash, 0, sizeof(temp_hash));
    memset(&ctx, 0, sizeof(ctx));

}
