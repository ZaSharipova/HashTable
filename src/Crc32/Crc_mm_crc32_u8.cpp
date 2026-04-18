#include "CRC32_my.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include <immintrin.h>

unsigned int CountHashcrc32(const char* stroke, size_t capacity) {
    assert(stroke);

    unsigned int hash = 0xFFFFFFFF;
    while (*stroke) {
        hash = _mm_crc32_u8(hash, (unsigned char)*stroke);
        stroke++;
    }

    return hash % capacity;
}