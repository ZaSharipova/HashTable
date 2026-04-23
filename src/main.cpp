#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <immintrin.h>

#ifdef _DCRC_INTR_STRLEN
#include "CRC32_my.h"
#else
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

// void PrintChainStats(HashTable* hash_table) {
//     assert(hash_table);
//     int max_chain = 0, empty = 0;
//     unsigned long long total = 0;

//     for (int i = 0; i < hash_table->capacity; i++) {
//         int len = 0;
//         Node* cur = hash_table->table[i];
//         while (cur) {
//             len++;
//             cur = cur->next;
//         }

//         if (len == 0) empty++;
//         if (len > max_chain) max_chain = len;

//         total += len;
//     }

//     printf("buckets: %d, empty: %d, max chain: %d, avg: %.2f\n",
//         hash_table->capacity, empty, max_chain,
//         (double)total / hash_table->capacity);
// }

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
            *sink += Contains(hash_table, queries[i], hashFunc);
        }

        clock_t time_end = clock();
        unsigned long long ticks_end = __rdtsc();

        if (run == 0) continue;
        ticks_total += (ticks_end - ticks_start);
        time_total += GetTimeInMSec(time_start, time_end);
    }

    *ticks = ticks_total / RUNS;

    DestroyTable(hash_table);
    return time_total / RUNS;
}