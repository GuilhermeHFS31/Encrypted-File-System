#ifndef HMAC_H
#define HMAC_H

#include <stddef.h>
#include <stdint.h>

#define HMAC_SHA512_BLOCK_LENGTH 128
#define HMAC_SHA512_HASH_LENGTH 64

void hmac_sha512(const uint8_t *key, size_t key_len,
                 const uint8_t *data, size_t data_len,
                 uint8_t *mac_out);

int verify_hmac(const uint8_t *key, size_t key_len,
                const uint8_t *data, size_t data_len,
                const uint8_t *stored_mac);

#endif 
