#ifndef _lzss_h
#define _lzss_h

#include <stdint.h>
#include <stdio.h>

void lzss_compress(uint8_t* data, size_t data_size, uint8_t** compressed_data, size_t* compressed_data_size);
void lzss_decompress(uint8_t* compressed_data, size_t compressed_data_size, uint8_t** data, size_t* data_size);

#endif
