#include "ChainTableList.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "HashFunctions.h"
#include "CommonFunctions.h"
#include <immintrin.h>

#include "Config.h"

#define DEFAULT_CHAIN_SIZE  4
#define INCREASE_FACTOR     2

#ifdef _DSIMD
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

static void* AlignedCalloc(size_t count, size_t elem_size) {
    size_t total = count * elem_size;
    size_t aligned = (total + ALIGN_SIZE - 1) & ~((size_t)ALIGN_SIZE - 1);
    if (aligned == 0) aligned = ALIGN_SIZE;

    void* ptr = aligned_alloc(ALIGN_SIZE, aligned);
    if (ptr) memset(ptr, 0, aligned);

    return ptr;
}

static void* AlignedRealloc(void* old_ptr, size_t old_count, size_t new_count, size_t elem_size) {
    assert(old_ptr);

    size_t new_total = new_count * elem_size;
    size_t aligned = (new_total + ALIGN_SIZE - 1) & ~((size_t)ALIGN_SIZE - 1);
    if (aligned == 0) aligned = ALIGN_SIZE;

    void* new_ptr = aligned_alloc(ALIGN_SIZE, aligned);
    if (!new_ptr) return NULL;

    memset(new_ptr, 0, aligned);

    size_t copy_bytes = old_count * elem_size;
    if (copy_bytes > aligned) copy_bytes = aligned;
    memcpy(new_ptr, old_ptr, copy_bytes);

    free(old_ptr);
    return new_ptr;
}

static int ChainList_Ctor(ChainList* list) {
    assert(list);

    list->size = DEFAULT_CHAIN_SIZE;

    list->data = (HT_Value*) AlignedCalloc((size_t) list->size, sizeof(HT_Value));
    if (!list->data) return -1;

    list->next = (int*) AlignedCalloc((size_t) list->size, sizeof(int));
    if (!list->next) {
        free(list->data);
        return -1;
    }

    list->prev = (int*) AlignedCalloc((size_t) list->size, sizeof(int));
    if (!list->prev) {
        free(list->data);
        free(list->next);
        return -1;
    }

    list->data[0] = HT_POISON;
    list->next[0] = 0;
    list->prev[0] = 0;

    for (int i = 1; i < list->size; i++) {
        list->data[i] = HT_POISON;
        list->next[i] = (i + 1 < list->size) ? i + 1 : 0;
        list->prev[i] = -1;
    }

    list->free = 1;
    list->number_of_elem = 0;
    return 0;
}

static void ChainList_Dtor(ChainList* list) {
    if (!list) return;

    free(list->data);
    free(list->next);
    free(list->prev);

    list->data = NULL;
    list->next = NULL;
    list->prev = NULL;
    list->size = 0;
    list->free = 0;
    list->number_of_elem = 0;
}

static int ChainList_Grow(ChainList* list) {
    assert(list);

    int old_size = list->size;
    int new_size = old_size * INCREASE_FACTOR;

    HT_Value* new_data = (HT_Value*) AlignedRealloc (list->data, (size_t) old_size, (size_t) new_size, sizeof(HT_Value));
    if (!new_data) return -1;

    int* new_next = (int*) AlignedRealloc (list->next, (size_t) old_size, (size_t) new_size, sizeof(int));
    if (!new_next) {
        free(new_data);
        return -1;
    }

    int* new_prev = (int*) AlignedRealloc (list->prev, (size_t) old_size, (size_t) new_size, sizeof(int));
    if (!new_prev) {
        free(new_data);
        free(new_next);
        return -1;
    }

    list->data = new_data;
    list->next = new_next;
    list->prev = new_prev;
    list->size = new_size;

    for (int i = old_size; i < new_size; i++) {
        list->data[i] = HT_POISON;
        list->next[i] = (i + 1 < new_size) ? i + 1 : 0;
        list->prev[i] = -1;
    }

    if (list->free == 0) {
        list->free = old_size;
    } else {
        int tail = list->free;
        while (list->next[tail] != 0) {
            tail = list->next[tail];
        }

        list->next[tail] = old_size;
    }

    return 0;
}

static int ChainList_Insert(ChainList* list, const char* value) {
    assert(list);

    if (list->free == 0) {
        if (ChainList_Grow(list) != 0) return -1;
    }

    int idx = list->free;
    list->free = list->next[idx];
    list->data[idx] = value;

    int after_head = list->next[0];
    list->prev[idx] = 0;
    list->next[idx] = after_head;
    list->prev[after_head] = idx;
    list->next[0] = idx;

    list->number_of_elem++;
    return 0;
}

static void ChainList_DeleteSlot(ChainList* list, int pos) {
    assert(list);
    assert(pos > 0 && pos < list->size);

    list->next[list->prev[pos]] = list->next[pos];
    list->prev[list->next[pos]] = list->prev[pos];

    list->data[pos] = HT_POISON;
    list->next[pos] = list->free;
    list->prev[pos] = -1;
    list->free = pos;

    list->number_of_elem--;
}

static int ChainList_Find(ChainList* list, const char* value) {
    assert(list);

    int cur = list->next[0];
    while (cur != 0) {
#ifdef _DSIMD
    #ifdef _DCHECK_FIRST_CHAR
        if (list->data[cur] && list->data[cur][0] == value[0] && simd_strcmp_32(list->data[cur], value)) {
    #else
        if (list->data[cur] && simd_strcmp_32(list->data[cur], value)) {
    #endif
#else
        if (list->data[cur] && strcmp(list->data[cur], value) == 0) {
#endif
            return cur;
        }
        cur = list->next[cur];
    }
    return 0;
}

HashTable* CreateTable(size_t capacity, float load_factor) {
    HashTable* hash_table = (HashTable*) calloc (1, sizeof(HashTable));
    CHECK_NULL(hash_table, ERROR_ARR, NULL);

    hash_table->buckets = (ChainList*) calloc (capacity, sizeof(ChainList));
    if (!hash_table->buckets) {
        perror(ERROR_ARR);
        free(hash_table);
        return NULL;
    }

    for (size_t i = 0; i < capacity; i++) {
        if (ChainList_Ctor(&hash_table->buckets[i]) != 0) {
            for (size_t j = 0; j < i; j++) ChainList_Dtor(&hash_table->buckets[j]);
            free(hash_table->buckets);
            free(hash_table);
            return NULL;
        }
    }

    hash_table->capacity = capacity;
    hash_table->size = 0;
    hash_table->load_factor = load_factor;
    return hash_table;
}

void DestroyTable(HashTable* hash_table) {
    assert(hash_table);

    for (size_t i = 0; i < hash_table->capacity; i++) {
        ChainList_Dtor(&hash_table->buckets[i]);
    }

    free(hash_table->buckets);
    free(hash_table);
}

static void Rehash(HashTable* hash_table, HashFunc Hash) {
    assert(hash_table);
    assert(Hash);

    size_t new_cap = hash_table->capacity * 2;

    ChainList* new_buckets = (ChainList*) calloc (new_cap, sizeof(ChainList));
    CHECK_NULL_VOID(new_buckets, ERROR_ARR);

    for (size_t i = 0; i < new_cap; i++) {
        if (ChainList_Ctor(&new_buckets[i]) != 0) {
            for (size_t j = 0; j < i; j++) ChainList_Dtor(&new_buckets[j]);
            free(new_buckets);
            return;
        }
    }

    for (size_t i = 0; i < hash_table->capacity; i++) {
        ChainList* old = &hash_table->buckets[i];
        int cur = old->next[0];

        while (cur != 0) {
            const char* val = old->data[cur];
            unsigned int hash = Hash(val) % new_cap;
            ChainList_Insert(&new_buckets[hash], val);
            cur = old->next[cur];
        }

        ChainList_Dtor(old);
    }

    free(hash_table->buckets);
    hash_table->buckets = new_buckets;
    hash_table->capacity = new_cap;
}

void Insert(HashTable* hash_table, const char* value, HashFunc Hash) {
    assert(hash_table);
    assert(value);
    assert(Hash);

    if ((float) hash_table->size / (float) hash_table->capacity > hash_table->load_factor) {
        Rehash(hash_table, Hash);
    }

    unsigned int hash = Hash(value) % hash_table->capacity;
    ChainList* chain = &hash_table->buckets[hash];

    if (ChainList_Find(chain, value) != 0) return;

    ChainList_Insert(chain, value);
    hash_table->size++;
}

void Delete(HashTable* hash_table, const char* value, HashFunc Hash) {
    assert(hash_table);
    assert(value);
    assert(Hash);

    unsigned int hash = Hash(value) % hash_table->capacity;
    ChainList* chain = &hash_table->buckets[hash];

    int slot = ChainList_Find(chain, value);
    if (slot == 0) return;

    ChainList_DeleteSlot(chain, slot);
    hash_table->size--;
}

int Contains(HashTable* hash_table, const char* value, HashFunc Hash) {
    assert(hash_table);
    assert(value);
    assert(Hash);

    unsigned int hash = Hash(value) % hash_table->capacity;
    return ChainList_Find(&hash_table->buckets[hash], value) != 0;
}