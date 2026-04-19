#include "CRC32_my.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include <immintrin.h>

unsigned int CountHashcrc32(const char* string, size_t capacity) {
    assert(string);

    unsigned int hash = 0xFFFFFFFF;
    while (*string) {
        hash = _mm_crc32_u8(hash, (unsigned char)*string);
        string++;
    }

    return hash % capacity;
}