#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "HashFunctions.h"
#include "ChainTable.h"
#include "CommonFunctions.h"
#include "Config.h"

#define NUMBER_KEYS 100000
#define NUMBER_QUERIES 10000000

static float MeasureChainSearch(char** keys, char** queries, int* sink);

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
    // float methods[4] = {};
    // methods[0] = MeasureChainSearch(keys, queries, (int *)&sink);
    printf("time: %f ms\n", MeasureChainSearch(keys, queries, (int *)&sink));

    printf("sink = %d\n\n", sink);
    // PrintResults(methods, names);
    // int result = SaveResults(methods, names);

    free(keys);
    free(queries);
    return 0;
}

static float MeasureChainSearch(char** keys, char** queries, int* sink) {
    assert(keys);
    assert(queries);
    assert(sink);

    ChainHashTable* chain = CreateChainTable(NUMBER_KEYS, 10.0f);
    if (!chain) return -1;

    for (int i = 0; i < NUMBER_KEYS; i++) {
        InsertChain(chain, keys[i], CountHashcrc32);
    }

    clock_t time_start = clock();
    for (int i = 0; i < NUMBER_QUERIES; i++) {
        *sink += ContainsChain(chain, queries[i], CountHashcrc32);
    }
    clock_t time_end = clock();

    DestroyChainTable(chain);
    return GetTimeInMSec(time_start, time_end);
}