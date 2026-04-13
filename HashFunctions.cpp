#include "HashFunctions.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "crc32.h"

unsigned int CountHashRemainder(unsigned int x, size_t capacity) {
    return ((unsigned int)x) % capacity;
}

unsigned int CountHashBits(unsigned int x, size_t capacity) {
    x ^= x >> 16;
    x ^= x >> 8;
    x ^= x >> 4;

    return x % capacity;
}

unsigned int CountHashKnuth(unsigned int x, size_t capacity) {
    double A = 0.6180339887498948;
    return (unsigned int)(capacity * fmod(x * A, 1.0)) % capacity;
}

unsigned int CountHashFloatBits(float x, size_t capacity) {
    return CountHashBits((int)x, capacity);
}

unsigned int CountHashFloatBitsBits(float x, size_t capacity) {
    unsigned int bits = 0;
    memcpy(&bits, &x, sizeof(bits));

    bits ^= bits >> 16;
    bits ^= bits >> 8;
    bits ^= bits >> 4;

    return bits % capacity;
}

unsigned int CountHashMantissa(float x, size_t capacity) {
    if (capacity == 0) return 0;

    unsigned int bits = 0;
    memcpy(&bits, &x, sizeof(x));

    unsigned int mantissa = bits & 0x7FFFFF;

    return (unsigned int)(mantissa % capacity);
}

unsigned int CountHashExponent(float x, size_t capacity) {
    if (capacity == 0) return 0;

    unsigned int bits = 0;
    memcpy(&bits, &x, sizeof(x));

    unsigned int exponent = (bits >> 23) & 0xFF;

    return exponent % capacity;
}

unsigned int CountHashMantissaExponent(float x, size_t capacity) {
    unsigned int bits = 0;
    memcpy(&bits, &x, sizeof(bits));

    unsigned int mantissa = bits & 0x7FFFFF;
    unsigned int exponent = (bits >> 23) & 0xFF;
    unsigned int result = mantissa * exponent;

    return result % capacity;
}

unsigned int CountHashStrokeSize(const char* stroke, size_t capacity) {
    assert(stroke);

    return strlen(stroke) % capacity;
}

unsigned int CountHashStrokeSymbols(const char* stroke, size_t capacity) {
    assert(stroke);

    int size = strlen(stroke);
    unsigned int result = 0;
    for (int i = 0; i < size; i++) {
        result += stroke[i];
    }

    return result % capacity;
}

unsigned int CountHashStrokePolinomial(const char* stroke, size_t capacity) {
    assert(stroke);

    int size = strlen(stroke);
    unsigned int result = 0;
    unsigned int P = 1027;

    for (int i = 0; i < size; i++) {
        result = (P * result + stroke[i]) % capacity;
    }

    return result % capacity;
}

unsigned int CountHashcrc32(const char* stroke, size_t capacity) {
    assert(stroke);

    return xcrc32((const unsigned char *)stroke, sizeof(stroke), 0) % capacity;
}

size_t TryLinear(size_t hash, size_t attempt, size_t capacity) {
    return (hash + attempt) % capacity;
}

size_t TryQuadratic(size_t hash, size_t attempt, size_t capacity) {
    return (hash + attempt * attempt) % capacity;
}

size_t TryDouble(size_t hash, size_t attempt, size_t capacity) {
    size_t hash2 = 1 + CountHashKnuth(hash, capacity - 1);
    return (CountHashRemainder(hash, capacity) + attempt * hash2) % capacity;
}