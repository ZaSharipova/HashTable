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
} ChainHashTable;

ChainHashTable* CreateChainTable(size_t capacity, float load_factor);
void DestroyChainTable(ChainHashTable* hash_table);

void InsertChain(ChainHashTable* hash_table, const char* value, HashFunc hash);
void DeleteChain(ChainHashTable* hash_table, const char* value, HashFunc hash);
int  ContainsChain(ChainHashTable* hash_table, const char* value, HashFunc hash);

#endif // CHAIN_TABLE_H_