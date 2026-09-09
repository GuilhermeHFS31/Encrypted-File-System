#ifndef KDF_H
#define KDF_H

#include <stdint.h>
#include <stddef.h>


int key_derivation_function(const char *password, size_t password_len, 
                            const uint8_t *salt, 
                            uint8_t *key_aes, uint8_t *key_hmac);

#endif 