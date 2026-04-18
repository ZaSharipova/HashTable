#ifndef CHAIN_TABLE_POOL_H
#define CHAIN_TABLE_POOL_H

#include <stddef.h>

#include "HashFunctions.h"

typedef struct Node {
    const char* value;
    struct Node* next;
} Node;

typedef struct Pool {
    Node* node_pool;
    size_t pool_capacity;
    size_t pool_used;
} Pool;

typedef struct HashTable {
    Node** table;
    size_t capacity;
    size_t size;
    float load_factor;

    Pool pool;
} HashTable;

HashTable* CreateTable(size_t capacity, float load_factor);
void DestroyTable(HashTable* hash_table);

void Insert(HashTable* hash_table, const char* value, HashFunc hash);
void Delete(HashTable* hash_table, const char* value, HashFunc hash);
int Contains(HashTable* hash_table, const char* value, HashFunc hash);

#endif // CHAIN_TABLE_POOL_H