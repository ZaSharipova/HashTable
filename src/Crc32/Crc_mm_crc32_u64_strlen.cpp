#include "CRC32_my.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <immintrin.h>
extern "C" size_t my_strlen(const char *stroke);

unsigned int CountHashcrc32(const char* stroke, size_t capacity) {
    assert(stroke);

    unsigned long long hash = 0xFFFFFFFF;
    size_t len = my_strlen(stroke);
    size_t i = 0;

    for (; i + 8 <= len; i += 8) {
        unsigned long long buf = *(const unsigned long long*)(stroke + i);
        hash = _mm_crc32_u64(hash, buf);
    }

    for (; i < len; i++) {
        hash = _mm_crc32_u8((unsigned int)hash, (unsigned char)stroke[i]);
    }

    return (unsigned int)hash % capacity;
}