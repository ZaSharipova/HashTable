#ifndef HASH_FUNCTIONS_H_
#define HASH_FUNCTIONS_H_

#include <stdio.h>
#include <stdbool.h>

typedef unsigned int (*HashFunc)(const char *);
typedef size_t (*ProbeFunc)(size_t hash, size_t attempt, size_t capacity);

unsigned int CountHashRemainder(unsigned int key);
unsigned int CountHashBits(unsigned int key);
unsigned int CountHashKnuth(unsigned int key, size_t capacity);
unsigned int CountHashFloatBits(float key);
unsigned int CountHashFloatBitsBits(float key);
unsigned int CountHashMantissa(float key);
unsigned int CountHashExponent(float key);
unsigned int CountHashMantissaExponent(float key);
unsigned int CountHashStringSize(const char *string);
unsigned int CountHashStringSymbols(const char *string);
unsigned int CountHashStringRolXor(const char *string);
unsigned int CountHashStringRorXor(const char *string);
unsigned int CountHashStringPolinomial(const char *string);
unsigned int CountHashcrc32(const char *string);
unsigned int CountHashcrc32_Intr(const char *string);

#endif // HASH_FUNCTIONS_H_