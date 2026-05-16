#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stddef.h>
#include "Structs.h"

HashTable* CreateTable(size_t capacity, float load_factor);
void DestroyTable(HashTable* hash_table);
ListErrors Insert(HashTable* hash_table, const char *value, HashFunc Hash);
ListErrors Delete(HashTable *hash_table, const char *value, HashFunc Hash);
ListErrors Contains(HashTable *hash_table, const char *value, HashFunc Hash, int *answer);

#endif // HASH_TABLE_H