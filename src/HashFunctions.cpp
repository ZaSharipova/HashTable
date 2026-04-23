#include "HashFunctions.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <immintrin.h>

#include "crc32.h"

unsigned int CountHashRemainder(unsigned int key, size_t capacity) {
    return ((unsigned int)key) % capacity;
}

unsigned int CountHashBits(unsigned int key, size_t capacity) {
    key ^= key >> 16;
    key ^= key >> 8;
    key ^= key >> 4;

    return key % capacity;
}

unsigned int CountHashKnuth(unsigned int key, size_t capacity) {
    double A = 0.6180339887498948;
    return (unsigned int)(capacity * fmod(key * A, 1.0)) % capacity;
}

unsigned int CountHashFloatBits(float key, size_t capacity) {
    return CountHashBits((int)key, capacity);
}

unsigned int CountHashFloatBitsBits(float key, size_t capacity) {
    unsigned int bits = 0;
    memcpy(&bits, &key, sizeof(bits));

    bits ^= bits >> 16;
    bits ^= bits >> 8;
    bits ^= bits >> 4;

    return bits % capacity;
}

unsigned int CountHashMantissa(float key, size_t capacity) {
    if (capacity == 0) return 0;

    unsigned int bits = 0;
    memcpy(&bits, &key, sizeof(key));

    unsigned int mantissa = bits & 0x7FFFFF;

    return (unsigned int)(mantissa % capacity);
}

unsigned int CountHashExponent(float key, size_t capacity) {
    if (capacity == 0) return 0;

    unsigned int bits = 0;
    memcpy(&bits, &key, sizeof(key));

    unsigned int exponent = (bits >> 23) & 0xFF;

    return exponent % capacity;
}

unsigned int CountHashMantissaExponent(float key, size_t capacity) {
    unsigned int bits = 0;
    memcpy(&bits, &key, sizeof(bits));

    unsigned int mantissa = bits & 0x7FFFFF;
    unsigned int exponent = (bits >> 23) & 0xFF;
    unsigned int result = mantissa * exponent;

    return result % capacity;
}

unsigned int CountHashStringSize(const char* string, size_t capacity) {
    assert(string);

    return strlen(string) % capacity;
}

unsigned int CountHashStringSymbols(const char* string, size_t capacity) {
    assert(string);

    int size = strlen(string);
    unsigned int result = 0;
    for (int i = 0; i < size; i++) {
        result += string[i];
    }

    return result % capacity;
}

unsigned int CountHashStringRolXor(const char* string, size_t capacity) {
    assert(string);
    int size = strlen(string);
    if (size == 0) return 0;

    unsigned int result = (unsigned char)string[0];
    for (int i = 1; i < size; i++) {
        result = (result << 1) | (result >> 31);
        result ^= (unsigned char)string[i];
    }

    return result % capacity;
}

unsigned int CountHashStringRorXor(const char* string, size_t capacity) {
    assert(string);
    int size = strlen(string);
    if (size == 0) return 0;

    unsigned int result = (unsigned char)string[0];
    for (int i = 1; i < size; i++) {
        result = (result >> 1) | (result << 31);
        result ^= (unsigned char)string[i];
    }

    return result % capacity;
}

unsigned int CountHashStringPolinomial(const char* string, size_t capacity) {
    assert(string);

    int size = strlen(string);
    unsigned int result = 0;
    unsigned int P = 1027;

    for (int i = 0; i < size; i++) {
        result = (P * result + string[i]) % capacity;
    }

    return result % capacity;
}

unsigned int CountHashcrc32(const char* string, size_t capacity) {
    assert(string);

    return xcrc32((const unsigned char *)string, strlen(string), 0) % capacity;
}

unsigned int CountHashcrc32_Intr(const char* string, size_t capacity) {
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