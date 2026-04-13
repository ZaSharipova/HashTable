#include "CommonFunctions.h"

#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "Config.h"

double GetTimeInMSec(clock_t start, clock_t end) {
    return (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
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

char** ReadString(const char* filename, int number_of_elements) {
    assert(filename);

    char** keys = (char **) calloc (number_of_elements, sizeof(char*));
    CHECK_NULL(keys, ERROR_ARR, NULL);

    FILE* file = fopen(filename, "r");
    if (!file) {
        perror(ERROR_FILE);
        free(keys);
        return NULL;
    }

    char buf[DEFAULT_SIZE] = {};
    for (int i = 0; i < number_of_elements; i++) {
        if (!fgets(buf, DEFAULT_SIZE, file)) break;
        buf[strcspn(buf, "\n")] = '\0';
        keys[i] = strdup(buf);
        if (!keys[i]) {
            perror(ERROR_ARR);
            fclose(file);
            return NULL;
        }
    }

    fclose(file);
    return keys;
}

int* GenerateUniqueKeys(int n) {
    int* keys = (int *) calloc (n, sizeof(int));
    CHECK_NULL(keys, ERROR_ARR, NULL);

    for (int i = 0; i < n; i++) {
        keys[i] = i + 1;
    }

    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp_key = keys[i];
        keys[i] = keys[j];
        keys[j] = tmp_key;
    }

    return keys;
}

char** GenerateQueries(char** keys, int n, int count) {
    assert(keys);
    
    char** queries = (char **) calloc (count, sizeof(char*));
    CHECK_NULL(queries, ERROR_ARR, NULL);
    
    const int MIN_LEN = 5;
    const int MAX_LEN = 20;
    
    for (int i = 0; i < count; i++) {
        if (rand() % 2 == 0) {
            queries[i] = strdup(keys[rand() % n]);
        } else {
            int length = MIN_LEN + rand() % (MAX_LEN - MIN_LEN + 1);
            queries[i] = (char*) calloc (length + 1, sizeof(char));
            CHECK_NULL(queries[i], ERROR_ARR, NULL);
            
            for (int j = 0; j < length; j++) {
                if (rand() % 2 == 0) {
                    queries[i][j] = 'a' + rand() % 26;
                } else {
                    queries[i][j] = 'A' + rand() % 26;
                }
            }
            queries[i][length] = '\0';
        }
    }
    
    return queries;
}