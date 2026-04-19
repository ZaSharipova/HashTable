#ifndef CHAIN_TABLE_H
#define CHAIN_TABLE_H

#include <stddef.h>
#include "HashFunctions.h"

typedef struct Node {
    const char* value;
    struct Node* next;
} Node;

typedef struct {
    Node** table;
    size_t capacity;
    size_t size;
    float load_factor;
} HashTable;

HashTable* CreateTable(size_t capacity, float load_factor);
void DestroyTable(HashTable* hash_table);
void Insert(HashTable* hash_table, const char* value, HashFunc hash);
void Delete(HashTable* hash_table, const char* value, HashFunc hash);
int  Contains(HashTable* hash_table, const char* value, HashFunc hash);

#endif // CHAIN_TABLE_H_