#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stddef.h>

typedef const char* HT_Value;
typedef unsigned int (*HashFunc)(const char* value, size_t capacity);

#define ALIGN_SIZE 32
#define HT_POISON NULL

typedef struct {
    int size;
    HT_Value *data;
    int *next;
    int *prev;
    int free;
    int number_of_elem;
} ChainList;

typedef struct {
    ChainList *buckets;
    size_t capacity;
    size_t size;
    float load_factor;
} HashTable;

HashTable *CreateTable(size_t capacity, float load_factor);
void DestroyTable(HashTable *ht);
void Insert(HashTable *ht, const char *value, HashFunc Hash);
void Delete(HashTable *ht, const char *value, HashFunc Hash);
int Contains (HashTable *ht, const char *value, HashFunc Hash);

#endif // HASH_TABLE_H