#include "CommonFunctions.h"

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <immintrin.h>

#include "Config.h"

double GetTimeInMSec(clock_t start, clock_t end) {
    return (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
}

char** ReadString(const char* filename, int number_of_elements) {
    assert(filename);

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Error open.\n");
        return NULL;
    }

    struct stat st = {};
    fstat(fd, &st);

    char* data = (char*) mmap (NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (data == MAP_FAILED) {
        perror("Error mmap.\n");
        return NULL;
    }

    madvise(data, st.st_size, MADV_SEQUENTIAL | MADV_WILLNEED);

    char** keys = (char**) calloc (number_of_elements, sizeof(char*));
    if (!keys) {
        munmap(data, st.st_size);
        return NULL;
    }

    char* merger = (char*) aligned_alloc (32, (size_t)number_of_elements * 32);
    if (!merger) {
        free(keys);
        munmap(data, st.st_size);
        return NULL;
    }

    memset(merger, 0, (size_t)number_of_elements * 32);
    // __m256i zero = _mm256_setzero_si256(); ----------------------> поняла, что это бессмысленно

    // for (int i = 0; i < number_of_elements; i++) {
    //     _mm256_storeu_si256((__m256i*)(merger) + i, zero);
    // }

    char* cursor = data;
    char* end = data + st.st_size;

    for (int i = 0; i < number_of_elements && cursor < end; i++) {
        char* newline = (char*) memchr (cursor, '\n', end - cursor);
        size_t len = newline ? (size_t)(newline - cursor) : (size_t)(end - cursor);

        keys[i] = merger + (size_t)i * 32;
        memcpy(keys[i], cursor, len < 31 ? len : 31);

        // __asm__ volatile (
        //     ".intel_syntax noprefix\n\t"
        //     "vpxor      ymm0, ymm0, ymm0\n\t"
        //     "vmovdqa    [%[dst]], ymm0\n\t"
        //     "mov        rcx, %[len]\n\t"
        //     "mov        rsi, %[src]\n\t"
        //     "mov        rdi, %[dst]\n\t"
        //     "rep        movsb\n\t"
        //     "vzeroupper\n\t"

        //     ".att_syntax prefix\n\t"
        //     :
        //     : [dst] "r" (keys[i]), [src] "r" (cursor), [len] "r" (len)
        //     : "rcx", "rsi", "rdi", "ymm0", "memory"
        // );
        cursor += len + 1;
    }

    munmap(data, st.st_size);
    return keys;
}

// char** ReadString(const char* filename, int number_of_elements) {
//     assert(filename);

//     char** keys = (char**) calloc (number_of_elements, sizeof(char*));
//     CHECK_NULL(keys, ERROR_ARR, NULL);

//     char* merger = (char*) aligned_alloc (32, (size_t)number_of_elements * 32);
//     if (!merger) {
//         free(keys);
//         return NULL;
//     }

//     memset(merger, 0, (size_t)number_of_elements * 32);

//     FILE* file = fopen(filename, "r");
//     if (!file) {
//         perror(ERROR_FILE);
//         free(merger);
//         free(keys);
//         return NULL;
//     }

//     char buf[DEFAULT_SIZE] = {};

//     for (int i = 0; i < number_of_elements; i++) {
//         if (!fgets(buf, DEFAULT_SIZE, file)) break;
//         buf[strcspn(buf, "\n")] = '\0';

//         keys[i] = merger + (size_t)i * 32;

//         size_t len = strlen(buf);
//         len = (len > 31) ? 31 : len;
//         memcpy(keys[i], buf, len);
//     }

//     fclose(file);
//     return keys;
// }

// char** ReadString(const char* filename, int number_of_elements) {
//     assert(filename);

//     char** keys = (char **) calloc (number_of_elements, sizeof(char*));
//     CHECK_NULL(keys, ERROR_ARR, NULL);

//     char *buf = (char *) calloc (number_of_elements * DEFAULT_SIZE, sizeof(char));
//     CHECK_NULL(buf, ERROR_ARR, NULL);

//     FILE* file = fopen(filename, "r");
//     if (!file) {
//         perror(ERROR_FILE);
//         free(keys);
//         return NULL;
//     }

//     // char buf[DEFAULT_SIZE] = {};
//     char* cursor = buf;

//     for (int i = 0; i < number_of_elements; i++) {
//         if (!fgets(cursor, DEFAULT_SIZE, file)) break;
//         cursor[strcspn(cursor, "\n")] = '\0';
//         keys[i] = cursor;
//         cursor += strlen(cursor) + 1;
//     }

//     fclose(file);
//     return keys;
// }

// char** ReadString(const char* filename, int number_of_elements) {
//     assert(filename);

//     char* merger = (char *) calloc (number_of_elements * DEFAULT_SIZE, sizeof(char));
//     CHECK_NULL(merger, ERROR_ARR, NULL);

//     char** keys = (char **) calloc (number_of_elements, sizeof(char*));
//     if (!keys) {
//         perror(ERROR_ARR);
//         free(merger);
//         return NULL;
//     }

//     for (int i = 0; i < number_of_elements; i++) {
//         keys[i] = merger + i * DEFAULT_SIZE;
//     }

//     FILE* file = fopen(filename, "r");
//     if (!file) {
//         perror(ERROR_FILE);
//         free(merger);
//         free(keys);
//         return NULL;
//     }

//     for (int i = 0; i < number_of_elements; i++) {
//         if (!fgets(keys[i], DEFAULT_SIZE, file)) break;
//         keys[i][strcspn(keys[i], "\n")] = '\0';
//     }

//     fclose(file);
//     return keys;
// }