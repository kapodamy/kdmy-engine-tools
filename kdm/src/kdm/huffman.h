#ifndef _huffman_h
#define _huffman_h

#include <stdint.h>
#include <stdlib.h>

uint8_t* huffman_compress(uint8_t* data, size_t data_size, size_t* compressed_size);
uint8_t* huffman_decompress(uint8_t* compressed, size_t compressed_size, size_t* data_size);

#endif