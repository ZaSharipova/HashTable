#include "ChainTablePool.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "HashFunctions.h"
#include "CommonFunctions.h"
#include <immintrin.h>

#include "Config.h"

#ifdef _DSIMD
// static inline int simd_strcmp_32(const char* a, const char* b) {
//     __m256i va = _mm256_load_si256((const __m256i*)a);
//     __m256i vb = _mm256_load_si256((const __m256i*)b);
//     __m256i cmp = _mm256_cmpeq_epi8(va, vb);
    
//     return _mm256_movemask_epi8(cmp) == (int)0xFFFFFFFF;
// }
//extern "C" int simd_strcmp_32(const char*, const char*);
static inline int simd_strcmp_32(const char* a, const char* b) {
    int mask = 0;
    __asm__ volatile (
        ".intel_syntax noprefix\n"
        "vmovdqa    ymm0, [%[a]]\n"
        "vpcmpeqb   ymm0, ymm0, [%[b]]\n"
        "vpmovmskb  %0, ymm0\n"
        "vzeroupper\n"

        ".att_syntax prefix\n"
        : "=r" (mask)
        : [a] "r" (a), [b] "r" (b)
        : "ymm0", "memory"
    );
    return mask == (int)0xFFFFFFFF;
}
#endif

HashTable* CreateTable(size_t capacity, float load_factor) {
    HashTable* hash_table = (HashTable*) calloc (1, sizeof(HashTable));
    CHECK_NULL(hash_table, ERROR_ARR, NULL);

    hash_table->table = (Node**) calloc (capacity, sizeof(Node*));
    if (!hash_table->table) {
        perror(ERROR_ARR);
        free(hash_table);
        return NULL;
    }

    hash_table->pool.pool_capacity = NUMBER_KEYS;
    hash_table->pool.node_pool = (Node *) calloc (hash_table->pool.pool_capacity, sizeof(Node));
    if (!hash_table->pool.node_pool) {
        perror(ERROR_ARR);
        free(hash_table->table);
        free(hash_table);
        return NULL;
    }
    hash_table->pool.pool_used = 0;

    hash_table->capacity = capacity;
    hash_table->size = 0;
    hash_table->load_factor = load_factor;

    return hash_table;
}

static Node* AllocNode(HashTable* hash_table) {
    assert(hash_table);

    if (hash_table->pool.pool_used >= hash_table->pool.pool_capacity) {
        size_t new_capacity = hash_table->pool.pool_capacity * 2;

        Node* new_pool = (Node*) realloc (hash_table->pool.node_pool, new_capacity * sizeof(Node));
        CHECK_NULL(new_pool, "Error realloc.\n", NULL);

        hash_table->pool.node_pool = new_pool;
        hash_table->pool.pool_capacity = new_capacity;
    }

    Node* node = &hash_table->pool.node_pool[hash_table->pool.pool_used++];
    node->next = NULL;
    node->value = NULL;

    return node;
}


static void Rehash(HashTable* hash_table, HashFunc Hash) {
    assert(hash_table);
    assert(Hash);

    size_t new_capacity = hash_table->capacity * 2;

    Node** new_table = (Node**) calloc (new_capacity, sizeof(Node*));
    CHECK_NULL_VOID(new_table, ERROR_ARR);

    for (size_t i = 0; i < hash_table->capacity; i++) {
#ifdef _DPREFETCH
        if (i + 8 < hash_table->capacity) {
            __asm__ volatile ("prefetcht0 (%0)" :: "r"(&hash_table->table[i + 8]));
        }
#endif
        Node* cur = hash_table->table[i];

        while (cur) {
#ifdef _DPREFETCH
            if (cur->next) {
                __asm__ volatile ("prefetcht0 (%0)" :: "r"(cur->next));
            }
#endif

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

void DestroyTable(HashTable* hash_table) {
    assert(hash_table);

    free(hash_table->pool.node_pool);
    free(hash_table->table);
    free(hash_table);
}

void Insert(HashTable* hash_table, const char* value, HashFunc Hash) {
    assert(hash_table);
    assert(Hash);

    if ((double)hash_table->size / hash_table->capacity > hash_table->load_factor) {
        Rehash(hash_table, Hash);
    }

    unsigned int hash = Hash(value, hash_table->capacity);
    Node* cur = hash_table->table[hash];

    while (cur) {
#ifdef _DPREFETCH
        if (cur->next) {
            __asm__ volatile ("prefetcht0 (%0)" :: "r"(cur->next));
            __asm__ volatile ("prefetcht0 (%0)" :: "r"(cur->next->value));
        }
#endif

#ifdef _DSIMD
        if (simd_strcmp_32(cur->value, value)) return;
#else 
        if (strcmp(cur->value, value) == 0) return;
#endif
        cur = cur->next;
    }

    Node* node = (Node *) AllocNode (hash_table);
    CHECK_NULL_VOID(node, ERROR_ARR);

    node->value = value;
    node->next = hash_table->table[hash];
    hash_table->table[hash] = node;

    hash_table->size++;
}

void Delete(HashTable* hash_table, const char* value, HashFunc Hash) {
    assert(hash_table);
    assert(Hash);

    unsigned int hash = Hash(value, hash_table->capacity);

    Node* cur = hash_table->table[hash];
    Node* prev = NULL;

    while (cur) {
#ifdef _DPREFETCH
        if (cur->next) {
            __asm__ volatile ("prefetcht0 (%0)" :: "r"(cur->next));
            __asm__ volatile ("prefetcht0 (%0)" :: "r"(cur->next->value));
        }
#endif

#ifdef _DSIMD
        if (simd_strcmp_32(cur->value, value)) {
#else 
        if (strcmp(cur->value, value) == 0) {
#endif
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

int Contains(HashTable* hash_table, const char* value, HashFunc Hash) {
    assert(hash_table);
    assert(Hash);

    unsigned int hash = Hash(value, hash_table->capacity);
    Node* cur = hash_table->table[hash];

    while (cur) {
#ifdef _DPREFETCH
        if (cur->next) {
            __asm__ volatile ("prefetcht0 (%0)" :: "r"(cur->next));
            __asm__ volatile ("prefetcht0 (%0)" :: "r"(cur->next->value));
        }
#endif

#ifdef _DSIMD
        if (simd_strcmp_32(cur->value, value)) {
#else 
        if (strcmp(cur->value, value) == 0) {
#endif
            return 1;
        }

        cur = cur->next;
    }

    return 0;
}