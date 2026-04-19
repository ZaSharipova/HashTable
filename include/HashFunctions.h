#ifndef HASH_FUNCTIONS_H_
#define HASH_FUNCTIONS_H_

#include <stdio.h>
#include <stdbool.h>

typedef unsigned int (*HashFunc)(const char *, size_t);
typedef size_t (*ProbeFunc)(size_t hash, size_t attempt, size_t capacity);

unsigned int CountHashRemainder(unsigned int key, size_t capacity);
unsigned int CountHashBits(unsigned int key, size_t capacity);
unsigned int CountHashKnuth(unsigned int key, size_t capacity);
unsigned int CountHashFloatBits(float key, size_t capacity);
unsigned int CountHashFloatBitsBits(float key, size_t capacity);
unsigned int CountHashMantissa(float key, size_t capacity);
unsigned int CountHashExponent(float key, size_t capacity);
unsigned int CountHashMantissaExponent(float key, size_t capacity);
unsigned int CountHashStringSize(const char* string, size_t capacity);
unsigned int CountHashStringSymbols(const char* string, size_t capacity);
unsigned int CountHashStringRolXor(const char* string, size_t capacity);
unsigned int CountHashStringRorXor(const char* string, size_t capacity);
unsigned int CountHashStringPolinomial(const char* string, size_t capacity);
unsigned int CountHashcrc32(const char* string, size_t capacity);

#endif // HASH_FUNCTIONS_H_