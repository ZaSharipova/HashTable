#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <immintrin.h>
#include <math.h>

#define HASH_NAME "crc32"
#ifdef _DCRC_INTR_STRLEN
#include "CRC32_my.h"
#else
#include "HashFunctions.h"
#endif

// #ifdef _DPOOL
// #include "ChainTablePool.h"
// #elif defined(_DLIST_TABLE)
// #include "ChainTableList.h"
// #else
// #include "ChainTable.h"
// #endif
#include "ChainTableList.h"

#include "CommonFunctions.h"
#include "Config.h"
#include "Verify.h"
#include "DoDump.h"
#include "DumpHelper.h"

// void PrintChainStats(HashTable *hash_table) {
//     assert(hash_table);
//
//     int max_chain = 0, empty = 0;
//     unsigned long long total = 0;

//     for (int i = 0; i < hash_table->capacity; i++) {
//         int len = 0;
//         Node *cur = hash_table->table[i];
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

static float MeasureSearch(char** keys, char** queries, int *sink, unsigned long long *ticks);

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
    unsigned long long ticks = 12;
    float time = MeasureSearch(keys, queries, (int *)&sink, &ticks);

    printf("time: %f ms\nticks: %llu\n", time, ticks);
    printf("sink = %d\n\n", sink);

    free(keys);
    free(queries);
    return 0;
}

static float MeasureSearch(char** keys, char** queries, int *sink, unsigned long long *ticks) {
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

    HashTable *hash_table = CreateTable(4, 10.0f);
    if (!hash_table) return -1;

    DUMP_HELPER_DECL(Info, hash_table->buckets);

    for (int i = 0; i < NUMBER_KEYS; i++) {
        ListErrors err = Insert(hash_table, keys[i], hashFunc);
#ifdef _DVERIFY
        if (err != kSuccess) {
            DUMP_HELPER_ON_ERROR(Info, hash_table, hashFunc, keys[i], i, err, "Insert");
            DestroyTable(hash_table);
            return -1;
        }
#else
        (void)err;
#endif
    }

    const int RUNS = 20;
    unsigned long long tick_runs[RUNS] = {};
    double time_runs[RUNS] = {};
    int answer = 0;

    for (int run = 0; run < RUNS + 1; run++) {
        unsigned long long ticks_start = __rdtsc();
        clock_t time_start = clock();

        for (int i = 0; i < NUMBER_QUERIES; i++) {
            ListErrors err = Contains(hash_table, queries[i], hashFunc, &answer);
#ifdef _DVERIFY
            if (err != kSuccess) {
                DUMP_HELPER_ON_ERROR(Info, hash_table, hashFunc, queries[i], i, err, "Contains");
                DestroyTable(hash_table);
                return -1;
            }
#else
            (void)err;
#endif
        
            *sink += answer;
        }

        clock_t time_end = clock();
        unsigned long long ticks_end = __rdtsc();

        if (run == 0) continue;
        tick_runs[run - 1] = ticks_end - ticks_start;
        time_runs[run - 1] = GetTimeInMSec(time_start, time_end);
    }

    unsigned long long ticks_total = 0;
    double time_total = 0;
    for (int i = 0; i < RUNS; i++) {
        ticks_total += tick_runs[i];
        time_total += time_runs[i];
    }
    double ticks_mean = (double)ticks_total / RUNS;
    double time_mean = time_total / RUNS;

    double ticks_var = 0, time_var = 0;
    for (int i = 0; i < RUNS; i++) {
        ticks_var += ((double)tick_runs[i] - ticks_mean) * ((double)tick_runs[i] - ticks_mean);
        time_var += (time_runs[i] - time_mean) * (time_runs[i] - time_mean);
    }
    ticks_var /= (RUNS - 1);
    time_var /= (RUNS - 1);

    double ticks_sigma = sqrt(ticks_var);
    double time_sigma  = sqrt(time_var);

    *ticks = (unsigned long long)ticks_mean;

    FILE *f = fopen("csv/timing_raw.csv", "a");
    if (f) {
        for (int i = 0; i < RUNS; i++) {
            fprintf(f, "%s,%llu,%.4f\n", HASH_NAME, tick_runs[i], time_runs[i]);
        }
        fclose(f);
    }

    printf("time:  %.4f ± %.4f ms  (σ)\n", time_mean, time_sigma);
    printf("ticks: %.0f ± %.0f     (σ)\n", ticks_mean, ticks_sigma);
    printf("95%% CI time:  [%.4f, %.4f] ms\n",
        time_mean - 2*time_sigma, time_mean + 2*time_sigma);

    DestroyTable(hash_table);
    return (float)time_mean;
}