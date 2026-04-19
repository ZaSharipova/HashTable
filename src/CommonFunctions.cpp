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

    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Error fopen.\n");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char* data = (char *) calloc (1, file_size + 1);
    if (!data) {
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(data, 1, file_size, file);
    if ((long)bytes_read != file_size) {
        perror("Error fread.\n");
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);

    char** keys = (char **) calloc (number_of_elements, sizeof(char*));
    if (!keys) {
        free(data);
        return NULL;
    }

    char* merger = (char *) aligned_alloc (32, (size_t)number_of_elements * 32);
    if (!merger) {
        free(keys);
        free(data);
        return NULL;
    }

    char* cursor = data;
    char* end = data + file_size;

    for (int i = 0; i < number_of_elements && cursor < end; i++) {
        char* newline = (char*) memchr (cursor, '\n', end - cursor);
        size_t len = newline ? (size_t)(newline - cursor) : (size_t)(end - cursor);

        keys[i] = merger + (size_t)i * 32;
        memcpy(keys[i], cursor, len < 31 ? len : 31);

        cursor += len + 1;
    }

    free(data);
    return keys;
}

// char** ReadString(const char* filename, int number_of_elements) {
//     assert(filename);

//     int fd = open(filename, O_RDONLY);
//     if (fd < 0) {
//         perror("Error open.\n");
//         return NULL;
//     }

//     struct stat st = {};
//     fstat(fd, &st);

//     char* data = (char*) mmap (NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
//     close(fd);

//     if (data == MAP_FAILED) {
//         perror("Error mmap.\n");
//         return NULL;
//     }

//     madvise(data, st.st_size, MADV_SEQUENTIAL | MADV_WILLNEED);

//     char** keys = (char**) calloc (number_of_elements, sizeof(char*));
//     if (!keys) {
//         munmap(data, st.st_size);
//         return NULL;
//     }

//     char* merger = (char*) aligned_alloc (32, (size_t)number_of_elements * 32);
//     if (!merger) {
//         free(keys);
//         munmap(data, st.st_size);
//         return NULL;
//     }

//     //memset(merger, 0, (size_t)number_of_elements * 32);
//     //__m256i zero = _mm256_setzero_si256(); //----------------------> поняла, что это бессмысленно

//     // for (int i = 0; i < number_of_elements; i++) {
//     //     _mm256_storeu_si256((__m256i*)(merger) + i, zero);
//     // }

//     char* cursor = data;
//     char* end = data + st.st_size;

//     for (int i = 0; i < number_of_elements && cursor < end; i++) {
//         char* newline = (char*) memchr (cursor, '\n', end - cursor);
//         size_t len = newline ? (size_t)(newline - cursor) : (size_t)(end - cursor);

//         keys[i] = merger + (size_t)i * 32;
//         //_mm256_store_si256((__m256i*)keys[i], zero); 
//         //memcpy(keys[i], cursor, len < 31 ? len : 31);

//         __asm__ volatile (
//             ".intel_syntax noprefix\n\t"
//             "vpxor      ymm0, ymm0, ymm0\n\t"
//             "vmovdqa    [%[dst]], ymm0\n\t"
//             "mov        rcx, %[len]\n\t"
//             "mov        rsi, %[src]\n\t"
//             "mov        rdi, %[dst]\n\t"
//             "rep        movsb\n\t"
//             "vzeroupper\n\t"

//             ".att_syntax prefix\n\t"
//             :
//             : [dst] "r" (keys[i]), [src] "r" (cursor), [len] "r" (len)
//             : "rcx", "rsi", "rdi", "ymm0", "memory"
//         );
//         cursor += len + 1;
//     }

//     munmap(data, st.st_size);
//     return keys;
// }