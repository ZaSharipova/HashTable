#include "HashFunctions.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <immintrin.h>

#include "crc32.h"

unsigned int CountHashRemainder(unsigned int key) {
    return ((unsigned int)key);
}

unsigned int CountHashBits(unsigned int key) {
    key ^= key >> 16;
    key ^= key >> 8;
    key ^= key >> 4;

    return key;
}

unsigned int CountHashKnuth(unsigned int key, size_t capacity) {
    double A = 0.6180339887498948;
    return (unsigned int)(capacity * fmod(key * A, 1.0));
}

unsigned int CountHashFloatBits(float key) {
    return CountHashBits((int)key);
}

unsigned int CountHashFloatBitsBits(float key) {
    unsigned int bits = 0;
    memcpy(&bits, &key, sizeof(bits));

    bits ^= bits >> 16;
    bits ^= bits >> 8;
    bits ^= bits >> 4;

    return bits;
}

unsigned int CountHashMantissa(float key) {
    unsigned int bits = 0;
    memcpy(&bits, &key, sizeof(key));

    unsigned int mantissa = bits & 0x7FFFFF;

    return (unsigned int)(mantissa);
}

unsigned int CountHashExponent(float key) {
    unsigned int bits = 0;
    memcpy(&bits, &key, sizeof(key));

    unsigned int exponent = (bits >> 23) & 0xFF;

    return exponent;
}

unsigned int CountHashMantissaExponent(float key) {
    unsigned int bits = 0;
    memcpy(&bits, &key, sizeof(bits));

    unsigned int mantissa = bits & 0x7FFFFF;
    unsigned int exponent = (bits >> 23) & 0xFF;
    unsigned int result = mantissa * exponent;

    return result;
}

unsigned int CountHashStringSize(const char* string) {
    assert(string);

    return strlen(string);
}

unsigned int CountHashStringSymbols(const char* string) {
    assert(string);

    int size = strlen(string);
    unsigned int result = 0;
    for (int i = 0; i < size; i++) {
        result += string[i];
    }

    return result;
}

unsigned int CountHashStringRolXor(const char* string) {
    assert(string);
    int size = strlen(string);
    if (size == 0) return 0;

    unsigned int result = (unsigned char)string[0];
    for (int i = 1; i < size; i++) {
        result = (result << 1) | (result >> 31);
        result ^= (unsigned char)string[i];
    }

    return result;
}

unsigned int CountHashStringRorXor(const char* string) {
    assert(string);
    int size = strlen(string);
    if (size == 0) return 0;

    unsigned int result = (unsigned char)string[0];
    for (int i = 1; i < size; i++) {
        result = (result >> 1) | (result << 31);
        result ^= (unsigned char)string[i];
    }

    return result;
}

unsigned int CountHashStringPolinomial(const char* string) {
    assert(string);

    int size = strlen(string);
    unsigned int result = 0;
    unsigned int P = 1027;

    for (int i = 0; i < size; i++) {
        result = (P * result + string[i]);
    }

    return result;
}

unsigned int CountHashcrc32(const char* string) {
    assert(string);

    return xcrc32((const unsigned char *)string, strlen(string), 0);
}

unsigned int CountHashcrc32_Intr(const char* string) {
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

    return (unsigned int)hash;
}