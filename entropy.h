#ifndef ENTROPY_H
#define ENTROPY_H

#include <stdint.h>
#include <stddef.h>

int generate_random_bytes(uint8_t *buffer, size_t size);

void aux_hex_visualization(uint8_t *buffer, size_t size);

#endif 
