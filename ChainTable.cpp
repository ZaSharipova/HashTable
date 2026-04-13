#include "ChainTable.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "HashFunctions.h"
#include "CommonFunctions.h"

static void RehashChain(ChainHashTable* hash_table, HashFunc Hash) {
    assert(hash_table);
    assert(Hash);

    size_t new_capacity = hash_table->capacity * 2;

    Node** new_table = (Node**) calloc (new_capacity, sizeof(Node*));
    CHECK_NULL_VOID(new_table, ERROR_ARR);

    for (size_t i = 0; i < hash_table->capacity; i++) {
        Node* cur = hash_table->table[i];

        while (cur) {
            Node* next = cur->next;

            unsigned int new_hash = Hash(cur->value, new_capacity);
            cur->next = new_table[new_hash];
            new_table[new_hash] = cur;
            cur = next;
        }
    }

    free(hash_table->table);
    hash_table->table = new_table;
    hash_table->capacity = new_capacity;
}

ChainHashTable* CreateChainTable(size_t capacity, float load_factor) {
    ChainHashTable* hash_table = (ChainHashTable *) calloc (1, sizeof(ChainHashTable));
    CHECK_NULL(hash_table, ERROR_ARR, NULL);

    hash_table->table = (Node **) calloc (capacity, sizeof(Node*));
    if (!hash_table->table) {
        perror(ERROR_ARR);
        free(hash_table);
        return NULL;
    }

    hash_table->capacity = capacity;
    hash_table->size = 0;
    hash_table->load_factor = load_factor;

    return hash_table;
}

void DestroyChainTable(ChainHashTable* hash_table) {
    assert(hash_table);

    for (size_t i = 0; i < hash_table->capacity; i++) {
        Node* cur = hash_table->table[i];

        while (cur) {
            Node* next = cur->next;
            free(cur);
            cur = next;
        }
    }

    free(hash_table->table);
    free(hash_table);
}

void InsertChain(ChainHashTable* hash_table, const char* value, HashFunc Hash) {
    assert(hash_table);
    assert(Hash);

    if ((double)hash_table->size / hash_table->capacity > hash_table->load_factor) {
        RehashChain(hash_table, Hash);
    }

    unsigned int hash = Hash(value, hash_table->capacity);
    Node* cur = hash_table->table[hash];

    while (cur) {
        if (strcmp(cur->value, value) == 0) {
            return;
        }
        cur = cur->next;
    }

    Node* node = (Node *) calloc (1, sizeof(Node));
    CHECK_NULL_VOID(node, ERROR_ARR);

    node->value = value;
    node->next = hash_table->table[hash];
    hash_table->table[hash] = node;

    hash_table->size++;
}

void DeleteChain(ChainHashTable* hash_table, const char * value, HashFunc Hash) {
    assert(hash_table);
    assert(Hash);

    unsigned int hash = Hash(value, hash_table->capacity);

    Node* cur = hash_table->table[hash];
    Node* prev = NULL;

    while (cur) {
        if (cur->value == value) {
            if (prev) {
                prev->next = cur->next;
            } else {
                hash_table->table[hash] = cur->next;
            }

            free(cur);
            hash_table->size--;
            return;
        }

        prev = cur;
        cur = cur->next;
    }
}

int ContainsChain(ChainHashTable* hash_table, const char * value, HashFunc Hash) {
    assert(hash_table);
    assert(Hash);

    unsigned int hash = Hash(value, hash_table->capacity);
    Node* cur = hash_table->table[hash];

    while (cur) {
        if (strcmp(cur->value, value) == 0)  {
            return 1;
        }
        cur = cur->next;
    }

    return 0;
}