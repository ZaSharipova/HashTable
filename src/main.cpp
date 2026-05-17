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

    char** keys = ReadString("out_small.txt", NUMBER_KEYS);
    if (!keys) return 1;

    char** queries = ReadString("in_small.txt", NUMBER_QUERIES);
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

static void RunWarmup(HashTable *hash_table, char **queries, HashFunc hashFunc, int *sink);
static void RunQueryBatch(HashTable *hash_table, char **queries, HashFunc hashFunc,
        int *sink, unsigned long long *out_ticks, double *out_time);
static void ComputeStats(unsigned long long *tick_runs, double *time_runs, int runs, double *out_ticks_mean,
        double *out_ticks_sigma, double *out_time_mean,  double *out_time_sigma);
static void WriteRawCSV(unsigned long long *tick_runs, double *time_runs, int runs);
static void PrintStats(double time_mean, double time_sigma, double ticks_mean, double ticks_sigma);

static float MeasureSearch(char **keys, char **queries, int *sink, unsigned long long *ticks) {
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

    RunWarmup(hash_table, queries, hashFunc, sink);

    for (int run = 0; run < RUNS; run++) {
        RunQueryBatch(hash_table, queries, hashFunc, sink, &tick_runs[run], &time_runs[run]);
#ifdef _DVERIFY
        if (time_runs[run] < 0) {
            DestroyTable(hash_table);
            return -1;
        }
#endif
    }

    double ticks_mean = 0.0, ticks_sigma = 0.0, time_mean = 0.0, time_sigma = 0.0;
    ComputeStats(tick_runs, time_runs, RUNS, &ticks_mean, &ticks_sigma, &time_mean, &time_sigma);

    *ticks = (unsigned long long)ticks_mean;

    WriteRawCSV(tick_runs, time_runs, RUNS);
    PrintStats(time_mean, time_sigma, ticks_mean, ticks_sigma);

    DestroyTable(hash_table);
#ifdef _DVERIFY
    fclose(Info.file);
#endif
    return (float)time_mean;
}

static void RunWarmup(HashTable *hash_table, char **queries, HashFunc hashFunc, int *sink) {
    assert(hash_table);
    assert(queries);
    assert(sink);

    int answer = 0;
    for (int i = 0; i < NUMBER_QUERIES; i++) {
        ListErrors err = Contains(hash_table, queries[i], hashFunc, &answer);
        (void)err;
        *sink += answer;
    }
}

static void RunQueryBatch(HashTable *hash_table, char **queries, HashFunc hashFunc,
        int *sink, unsigned long long *out_ticks, double *out_time) {
    assert(hash_table);
    assert(queries);
    assert(sink);
    assert(out_ticks);
    assert(out_time);
    int answer = 0;

    unsigned long long ticks_start = __rdtsc();
    clock_t time_start = clock();

    for (int i = 0; i < NUMBER_QUERIES; i++) {
        ListErrors err = Contains(hash_table, queries[i], hashFunc, &answer);
#ifdef _DVERIFY
        if (err != kSuccess) {
            *out_ticks = 0;
            *out_time  = -1.0;
            return;
        }
#else
        (void)err;
#endif
        *sink += answer;
    }

    clock_t time_end = clock();
    unsigned long long ticks_end = __rdtsc();

    *out_ticks = ticks_end - ticks_start;
    *out_time  = GetTimeInMSec(time_start, time_end);
}

static void ComputeStats(unsigned long long *tick_runs, double *time_runs, int runs, double *out_ticks_mean,
        double *out_ticks_sigma, double *out_time_mean,  double *out_time_sigma) {
    assert(tick_runs);
    assert(time_runs);
    assert(out_time_mean);
    assert(out_ticks_sigma);
    assert(out_time_mean);
    assert(out_time_sigma);

    unsigned long long ticks_total = 0;
    double time_total = 0;
    for (int i = 0; i < runs; i++) {
        ticks_total += tick_runs[i];
        time_total += time_runs[i];
    }

    *out_ticks_mean = (double)ticks_total / runs;
    *out_time_mean = time_total / runs;

    double ticks_var = 0, time_var = 0;
    for (int i = 0; i < runs; i++) {
        double diff_ticks = (double)tick_runs[i] - *out_ticks_mean;
        double diff_mean = time_runs[i] - *out_time_mean;

        ticks_var += diff_ticks * diff_ticks;
        time_var += diff_mean * diff_mean;
    }

    *out_ticks_sigma = sqrt(ticks_var / (runs - 1));
    *out_time_sigma = sqrt(time_var / (runs - 1));
}

static void WriteRawCSV(unsigned long long *tick_runs, double *time_runs, int runs) {
    assert(time_runs);
    assert(time_runs);

    FILE *timing_file = fopen("csv/timing_raw.csv", "a");
    if (!timing_file) {
        perror("Error opening timing file.\n");
        return;
    }

    for (int i = 0; i < runs; i++) {
        fprintf(timing_file, "%s,%llu,%.4f\n", HASH_NAME, tick_runs[i], time_runs[i]);
    }

    fclose(timing_file);
}

static void PrintStats(double time_mean, double time_sigma, double ticks_mean, double ticks_sigma) {
    printf("time:  %.4f ± %.4f ms\n", time_mean, time_sigma);
    printf("ticks: %.0f ± %.0f\n", ticks_mean, ticks_sigma);
    printf("95%% CI time:  [%.4f, %.4f] ms\n", time_mean - 2 * time_sigma, time_mean + 2 * time_sigma);
}