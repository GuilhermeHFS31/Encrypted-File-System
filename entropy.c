#include "entropy.h"
#include <stdio.h>


int generate_random_bytes(uint8_t *buffer, size_t size){

    FILE * randomData = fopen("/dev/urandom", "rb");

    if (randomData == NULL)
    {
        perror("Error opening /dev/urandom. ");
        return -1;
    }

    size_t elements_read = fread(buffer, sizeof(uint8_t), size, randomData);

    fclose(randomData);

    if (elements_read != size)
    {
    
        fprintf(stderr, "Error: Incorrect number of bytes read (%zu of %zu).\n", elements_read, size);
        return -1;
    }

    return 0;
}

// auxiliar só para testar
void aux_hex_visualization(uint8_t *buffer, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        printf("%02x", buffer[i]);
    }
    printf("\n");
}

