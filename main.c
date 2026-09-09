#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <string.h>

#include "kdf.h"
#include "entropy.h"
#include "sha2.h"
#include "cipher.h"
#include "hmac.h"

// arquivo inicial de testes
int main()
{
    uint8_t kdf_salt[32]; // 32 bytes 
    uint8_t IV[16]; // 16 bytes 
    
    // buffers para as chaves derivadas 
    uint8_t key_aes[32];
    uint8_t key_hmac[32];
    
    const char *senha_teste = "senhasupersecreta";
    size_t senha_len = strlen(senha_teste);
    
    int rc = 0;

    rc = generate_random_bytes(kdf_salt, 32);
    if (rc != 0) {
        fprintf(stderr, "Falha ao gerar o Salt.\n");
        return 1;
    }

    printf("SALT gerado com sucesso:\n");
    aux_hex_visualization(kdf_salt, 32);
    printf("\n");

    rc = generate_random_bytes(IV, 16);
    if (rc != 0) {
        fprintf(stderr, "Falha ao gerar o IV.\n");
        return 1;
    }
    printf("IV gerado com sucesso:\n");
    aux_hex_visualization(IV, 16);
    printf("\n");

    printf("Derivando chaves a partir da senha: \"%s\"\n", senha_teste);
    rc = key_derivation_function(senha_teste, senha_len, kdf_salt, key_aes, key_hmac);
    
    if (rc == 0) {
        printf("Chave AES-256 derivada:\n");
        aux_hex_visualization(key_aes, 32);
        
        printf("Chave HMAC-SHA512 derivada:\n");
        aux_hex_visualization(key_hmac, 32);
    } else {
        fprintf(stderr, "Erro ao derivar as chaves.\n");
        return 1;
    }


    const char *mensagem_teste = "mensagemsupersecreta";
    size_t mensagem_len = strlen(mensagem_teste);
    printf("Mensagem Original: %s\n", mensagem_teste);
    uint8_t buffer_cifra[128];
    memcpy(buffer_cifra, mensagem_teste, mensagem_len);
    uint8_t hmac_stored[HMAC_SHA512_HASH_LENGTH];


    crypto_encrypt_aes256_ctr(key_aes, IV,buffer_cifra,mensagem_len);

    printf("Mensagem Cifrada: ");
    aux_hex_visualization(buffer_cifra, mensagem_len);

    hmac_sha512(key_hmac,32, buffer_cifra, mensagem_len, hmac_stored);

    buffer_cifra[0] ^= 0x01; 
    if (verify_hmac(key_hmac, 32, buffer_cifra, mensagem_len, hmac_stored))
    {
        crypto_decrypt_aes256_ctr(key_aes, IV, buffer_cifra, mensagem_len);
        buffer_cifra[mensagem_len] = '\0';
        printf("Mensagem Decifrada: %s\n", (char *)buffer_cifra);
    } else 
    {
        printf("Erro na validação do HMAC.\n");
    }

    return 0;

}