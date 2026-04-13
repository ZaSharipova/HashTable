#ifndef COMMON_FUNCTIONS_H_
#define COMMON_FUNCTIONS_H_

#include <stdio.h>
#include <time.h>

double GetTimeInMSec(clock_t start, clock_t end);
float GetVariance(int* buckets, int n);
char** ReadString(const char* filename, int number_of_elements);
// int* GenerateUniqueKeys(int n);
char** GenerateQueries(char** keys, int n, int count);

#define CHECK_NULL(ptr, message, ret) \
    if (!(ptr)) {                     \
        perror(message);              \
        return ret;                   \
    }

#define CHECK_NULL_VOID(ptr, message) \
    if (!(ptr)) {                     \
        perror(message);              \
        return;                       \
    }

#define ERROR_ARR "Error calloc.\n"
#define ERROR_FILE "Error opening file.\n"

#endif // COMMON_FUNCTIONS_H_