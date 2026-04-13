#ifndef HASH_FUNCTIONS_H_
#define HASH_FUNCTIONS_H_

#include <stdio.h>
#include <stdbool.h>

typedef unsigned int (*HashFunc)(const char *, size_t);
typedef size_t (*ProbeFunc)(size_t hash, size_t attempt, size_t capacity);

size_t TryLinear(size_t hash, size_t attempt, size_t capacity);
size_t TryQuadratic(size_t hash, size_t attempt, size_t capacity);
size_t TryDouble(size_t hash, size_t attempt, size_t capacity);

unsigned int CountHashRemainder(unsigned int x, size_t capacity);

unsigned int CountHashBits(unsigned int x, size_t capacity);
unsigned int CountHashKnuth(unsigned int x, size_t capacity);
unsigned int CountHashFloatBits(float x, size_t capacity);
unsigned int CountHashFloatBitsBits(float x, size_t capacity);
unsigned int CountHashMantissa(float x, size_t capacity);
unsigned int CountHashExponent(float x, size_t capacity);
unsigned int CountHashMantissaExponent(float x, size_t capacity);
unsigned int CountHashStrokeSize(const char* stroke, size_t capacity);
unsigned int CountHashStrokeSymbols(const char* stroke, size_t capacity);
unsigned int CountHashStrokePolinomial(const char* stroke, size_t capacity);
unsigned int CountHashcrc32(const char* stroke, size_t capacity);

#endif // HASH_FUNCTIONS_H_