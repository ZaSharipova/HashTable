#include "HashFunctions.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <time.h>
// #include <immintrin.h>

#include "CommonFunctions.h"
#include "Config.h"

static void SaveCSV(const char *filename, int *buckets);
static void RunStr(const char *name, char** keys, unsigned int (*fn)(const char *), FILE *timing_file);
static char** ReadStr(const char *filename);

int main(void) {
    system("mkdir -p csv");

    FILE *timing_file = fopen("csv/timing.csv", "w");
    CHECK_NULL(timing_file, ERROR_FILE, 1);
    fprintf(timing_file, "group,name,time_ms,time_ticks\n");

    char** str_keys = ReadStr("data/tests_string.txt");
    if (!str_keys) return 1;

    printf("\n=== strings ===\n");
    RunStr("length", str_keys, CountHashStringSize, timing_file);
    RunStr("symbols", str_keys, CountHashStringSymbols, timing_file);
    RunStr("rol+xor", str_keys, CountHashStringRolXor, timing_file);
    RunStr("ror+xor", str_keys, CountHashStringRorXor, timing_file);
    RunStr("polynomial", str_keys, CountHashStringPolinomial, timing_file);
    RunStr("crc32", str_keys, CountHashcrc32, timing_file);

    for (int i = 0; i < N; i++) {
        free(str_keys[i]);
    }

    free(str_keys);

    fclose(timing_file);

    return 0;
}

static void SaveCSV(const char *filename, int *buckets) {
    assert(filename);
    assert(buckets);

    FILE *file = fopen(filename, "w");
    CHECK_NULL_VOID(file, ERROR_FILE);

    fprintf(file, "bucket,count\n");
    for (int i = 0; i < BUCKETS; i++) {
        fprintf(file, "%d,%d\n", i, buckets[i]);
    }

    fclose(file);
}

float GetVariance(int *buckets, int n) {
    assert(buckets);

    float mean = (float)N / n;
    float var = 0;
    for (int i = 0; i < n; i++) {
        float diff = buckets[i] - mean;
        var += diff * diff;
    }

    return var / n;
}

static void RunFinish(const char *name, const char *prefix, int *buckets, clock_t time_start, clock_t time_end, 
        unsigned long long time_ticks, FILE *timing_file) {
    assert(name);
    assert(prefix);
    assert(buckets);
    assert(timing_file);

    float time_ms = GetTimeInMSec(time_start, time_end);
    float variance = GetVariance(buckets, BUCKETS);

    printf("%-30s  time: %7.2f ms, ticks: %llu, variance: %.2f\n", name, time_ms, time_ticks, variance);
    fprintf(timing_file, "%s,%s,%.2f,%llu\n", prefix, name, time_ms, time_ticks);

    char fname[DEFAULT_SIZE] = {};
    snprintf(fname, sizeof(fname), "csv/%s_%s.csv", prefix, name);
    SaveCSV(fname, buckets);
}

static void RunStr(const char *name, char** keys, unsigned int (*fn)(const char *, size_t), FILE *timing_file) {
    assert(name);
    assert(keys);
    assert(fn);
    assert(timing_file);

    int buckets[BUCKETS] = {};
    unsigned long long ticks_start = __rdtsc();
    clock_t time_start = clock();
    for (int i = 0; i < N; i++) {
        buckets[fn(keys[i], BUCKETS)]++;
    }
    unsigned long long ticks_end = __rdtsc();
    clock_t time_end = clock();

    RunFinish(name, "str", buckets, time_start, time_end, ticks_end - ticks_start, timing_file);
}

static char** ReadStr(const char *filename) {
    assert(filename);

    char** keys = (char **) calloc (N, sizeof(char *));
    CHECK_NULL(keys, ERROR_ARR, NULL);

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror(ERROR_FILE);
        free(keys);
        return NULL;
    }

    for (int i = 0; i < N; i++) {
        keys[i] = (char *) calloc (DEFAULT_SIZE, sizeof(char));
        if (!keys[i]) {
            perror(ERROR_ARR);
            fclose(file);
            return NULL;
        }

        fscanf(file, "%s", keys[i]);
    }

    fclose(file);
    return keys;
}