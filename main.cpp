#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <immintrin.h>

#include "HashFunctions.h"
#include "ChainTable.h"
#include "CommonFunctions.h"
#include "Config.h"

static float MeasureChainSearch(char** keys, char** queries, int* sink, unsigned long long *ticks);

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
    float time = MeasureChainSearch(keys, queries, (int *)&sink, &ticks);

    printf("time: %f ms\nticks: %llu\n", time, ticks);
    printf("sink = %d\n\n", sink);

    free(keys);
    free(queries);
    return 0;
}

static float MeasureChainSearch(char** keys, char** queries, int* sink, unsigned long long *ticks) {
    assert(keys);
    assert(queries);
    assert(sink);
    assert(ticks);

    ChainHashTable* chain = CreateChainTable(NUMBER_KEYS, 10.0f);
    if (!chain) return -1;

    for (int i = 0; i < NUMBER_KEYS; i++) {
        InsertChain(chain, keys[i], CountHashcrc32);
    }

    unsigned long long ticks_start = __rdtsc();
    clock_t time_start = clock();
    for (int i = 0; i < NUMBER_QUERIES; i++) {
        *sink += ContainsChain(chain, queries[i], CountHashcrc32);
    }
    clock_t time_end = clock();
    unsigned long long ticks_end = __rdtsc();
    *ticks = ticks_end - ticks_start;

    DestroyChainTable(chain);
    return GetTimeInMSec(time_start, time_end);
}