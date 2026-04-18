#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <immintrin.h>

#ifdef _DCRC_INTR
#include "CRC32_my.h"
#else
#include "HashFunctions.h"
#endif

#ifdef _DPOOL
#include "ChainTablePool.h"
#else 
#include "ChainTable.h"
#endif

#include "CommonFunctions.h"
#include "Config.h"

static float MeasureSearch(char** keys, char** queries, int* sink, unsigned long long *ticks);

int main(void) {
    srand(SEED);

    char** keys = ReadString("data/tests_string.txt", NUMBER_KEYS);
    if (!keys) return 1;

    char** queries = ReadString("data/tests_queries.txt", NUMBER_QUERIES);
    if (!queries) {
        free(keys);
        return 1;
    }

    volatile int sink = 0;
    unsigned long long ticks = 12;
    float time = MeasureSearch(keys, queries, (int *)&sink, &ticks);

    printf("time: %f ms\nticks: %llu\n", time, ticks);
    printf("sink = %d\n\n", sink);

    free(keys);
    free(queries);
    return 0;
}

static float MeasureSearch(char** keys, char** queries, int* sink, unsigned long long *ticks) {
    assert(keys);
    assert(queries);
    assert(sink);
    assert(ticks);

    HashTable* hash_table = CreateTable(4, 10.0f);
    if (!hash_table) return -1;

    for (int i = 0; i < NUMBER_KEYS; i++) {
        Insert(hash_table, keys[i], CountHashcrc32);
    }

    unsigned long long ticks_start = __rdtsc();
    clock_t time_start = clock();
    for (int i = 0; i < 5; i++) {
        for (int i = 0; i < NUMBER_QUERIES; i++) {
            *sink += Contains(hash_table, queries[i], CountHashcrc32);
        }
    }
    clock_t time_end = clock();
    unsigned long long ticks_end = __rdtsc();
    *ticks = ticks_end - ticks_start;

    DestroyTable(hash_table);
    return GetTimeInMSec(time_start, time_end);
}