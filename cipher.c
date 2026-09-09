#include "cipher.h"

#include <string.h>

#include "aes.h"

void crypto_encrypt_aes256_ctr(const uint8_t *key_aes,
                               const uint8_t *iv,
                               uint8_t *buffer,
                               size_t size)
{
    struct AES_ctx ctx;

    AES_init_ctx_iv(&ctx, key_aes, iv);

    AES_CTR_xcrypt_buffer(&ctx, buffer, size);

    memset(&ctx, 0, sizeof(ctx));

}


void crypto_decrypt_aes256_ctr(const uint8_t *key_aes,
                               const uint8_t *iv,
                               uint8_t *buffer,
                               size_t size)
{
    // No CTR, o encrypt e o decrypt são iguais
    crypto_encrypt_aes256_ctr(key_aes, iv, buffer, size);
}

