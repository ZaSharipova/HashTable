#include "CRC32_my.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// #include <immintrin.h>

unsigned int CountHashcrc32(const char *string, size_t capacity) {
    assert(string);

    unsigned long long hash = 0xFFFFFFFF;
    size_t len = strlen(string);
    size_t i = 0;

    for (; i + 8 <= len; i += 8) {
        unsigned long long buf = *(const unsigned long long*)(string + i);
        hash = _mm_crc32_u64(hash, buf);
    }

    for (; i < len; i++) {
        hash = _mm_crc32_u8((unsigned int)hash, (unsigned char)string[i]);
    }

    return (unsigned int)hash % capacity;
}