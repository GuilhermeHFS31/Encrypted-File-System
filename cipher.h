#ifndef CIPHER_H
#define CIPHER_H

#include <stddef.h>
#include <stdint.h>

#define AES256_KEY_LENGTH 32u
#define AES_CTR_IV_LENGTH 16u

void crypto_encrypt_aes256_ctr(const uint8_t *key_aes,
                               const uint8_t *iv,
                               uint8_t *buffer,
                               size_t size);

void crypto_decrypt_aes256_ctr(const uint8_t *key_aes,
                               const uint8_t *iv,
                               uint8_t *buffer,
                               size_t size);

#endif
