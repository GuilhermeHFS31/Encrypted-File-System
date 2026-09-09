#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <string.h>

#include "kdf.h"
#include "entropy.h"
#include "sha2.h"



int key_derivation_function(const char *password, size_t password_len, 
                   const uint8_t *salt, 
                   uint8_t *key_aes, uint8_t *key_hmac)
{
    SHA2_CTX ctx;
    uint8_t final_hash[64];

    SHA512Init(&ctx);

    SHA512Update(&ctx, (const uint8_t*) password, password_len);
    SHA512Update(&ctx, salt, 32);

    SHA512Final(final_hash, &ctx);
    memcpy(key_aes, final_hash, 32);
    memcpy(key_hmac, final_hash + 32, 32);


    memset(final_hash, 0, sizeof(final_hash));
    return 0;
}


// // só para testar as funções implementadas
// int main()
// {
//     uint8_t kdf_salt[32]; // 32 bytes 
//     uint8_t IV[16];       // 16 bytes 
    
//     // Buffers para as chaves derivadas 
//     uint8_t key_aes[32];
//     uint8_t key_hmac[32];
    
//     const char *senha_teste = "testando123";
//     size_t senha_len = strlen(senha_teste);
    
//     int rc = 0;

//     rc = generate_random_bytes(kdf_salt, 32);
//     if (rc != 0) {
//         fprintf(stderr, "Falha ao gerar o Salt.\n");
//         return 1;
//     }

//     printf("SALT gerado com sucesso:\n");
//     aux_hex_visualization(kdf_salt, 32);
//     printf("\n");

//     rc = generate_random_bytes(IV, 16);
//     if (rc != 0) {
//         fprintf(stderr, "Falha ao gerar o IV.\n");
//         return 1;
//     }
//     printf("IV gerado com sucesso:\n");
//     aux_hex_visualization(IV, 16);
//     printf("\n");

//     printf("Derivando chaves a partir da senha: \"%s\"\n", senha_teste);
//     rc = key_derivation_function(senha_teste, senha_len, kdf_salt, key_aes, key_hmac);
    
//     if (rc == 0) {
//         printf("Chave AES-256 derivada:\n");
//         aux_hex_visualization(key_aes, 32);
        
//         printf("Chave HMAC-SHA256 derivada:\n");
//         aux_hex_visualization(key_hmac, 32);
//     } else {
//         fprintf(stderr, "Erro ao derivar as chaves.\n");
//         return 1;
//     }

//     return 0;

// }