#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <immintrin.h>
#include <string.h>

#ifdef _DCRC_INTR_STRLEN
#include "CRC32_my.h"
#include "HashFunctions.h"
#else
#include "CRC32_my.h"
#include "HashFunctions.h"
#endif

#ifdef _DPOOL
#include "ChainTablePool.h"
#elif defined(_DLIST_TABLE)
#include "ChainTableList.h"
#else
#include "ChainTable.h"
#endif

#include "CommonFunctions.h"
#include "Config.h"

#define NUMBER_OF_FUNCS 4
#define INIT_HASH_FUNC(func) {func, #func}

static float MeasureSearch(const char *name, char** keys, char** queries, int* sink, unsigned long long *ticks, HashFunc hashfunc);

struct FuncInfo {
    HashFunc func;
    const char *name;
};

int main(void) {
    srand(SEED);

    char** keys = ReadString("out.txt", NUMBER_KEYS);
    if (!keys) return 1;

    char** queries = ReadString("in.txt", NUMBER_QUERIES);
    if (!queries) {
        free(keys);
        return 1;
    }

    volatile int sink = 0;
    unsigned long long ticks = 0;
    FuncInfo func_arr[NUMBER_OF_FUNCS] = {
        INIT_HASH_FUNC(CountHashcrc32),
        INIT_HASH_FUNC(CountHashStringPolinomial),
        INIT_HASH_FUNC(CountHashStringRolXor),
        INIT_HASH_FUNC(CountHashStringRorXor),
    };

    for (int i = 0; i < NUMBER_OF_FUNCS; i++) {
        float time = MeasureSearch(func_arr[i].name, keys, queries, (int *)&sink, &ticks, func_arr[i].func);
        printf("func: %s %f ms\n", func_arr[i].name, time);
    }

    printf("time: %f ms\nticks: %llu\n", time, ticks);
    printf("sink = %d\n\n", sink);

    free(keys);
    free(queries);
    return 0;
}

static float MeasureSearch(const char *name, char** keys, char** queries, int* sink, unsigned long long *ticks, HashFunc hashfunc) {
    assert(keys);
    assert(queries);
    assert(sink);
    assert(ticks);

    char *filename = (char *) calloc (128, sizeof(char));
    snprintf(filename, 128, "%s.txt", name);
    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "ERROR opening file %s", name);
        return -1;
    }

#ifdef _DCRC_INTR
    HashFunc hashFunc = CountHashcrc32_Intr;
#elif defined(_DCRC_INTR_STRLEN)
    HashFunc hashFunc = CountHashcrc32_Strlen;
#else
    HashFunc hashFunc = CountHashcrc32;
#endif

    HashTable* hash_table = CreateTable(4, 10.0f);
    if (!hash_table) return -1;

    for (int i = 0; i < NUMBER_KEYS; i++) {
        Insert(hash_table, keys[i], hashFunc);
    }

    unsigned long long ticks_total = 0;
    double time_total = 0;
    const int RUNS = 5;

    for (int run = 0; run < RUNS + 1; run++) {
        unsigned long long ticks_start = __rdtsc();
        clock_t time_start = clock();

        for (int i = 0; i < NUMBER_QUERIES; i++) {
            *sink += Contains(hash_table, queries[i], hashfunc);
        }

        clock_t time_end = clock();
        unsigned long long ticks_end = __rdtsc();

        if (run == 0) continue;
        ticks_total += (ticks_end - ticks_start);
        time_total += GetTimeInMSec(time_start, time_end);
    }

    *ticks = ticks_total / RUNS;

    DestroyTable(hash_table);
    fclose(file);
    return time_total / RUNS;
}