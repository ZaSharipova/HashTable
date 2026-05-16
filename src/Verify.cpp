#include "Verify.h"

#include <stdio.h>
#include <assert.h>

#include "ChainTableList.h"
#include "Structs.h"

int GetListHeadPos(ChainList *list) {
    assert(list);

    return list->next[0];
}

int GetListTailPos(ChainList *list) {
    assert(list);

    return list->prev[0];
}

// static Realloc_Mode CheckSize(List *list) {
//     assert(list);

//     if (list->number_of_elem * INCREASE_VALUE >= list->size) {
//         return kIncrease;
//     }

//     else if (list->size == 0) {
//         return kIncreaseZero;
//     }

//     return kNoChange;
// }

ListErrors ListVerify(ChainList *list) {
    assert(list);
    unsigned int error = 0;
    if (list->size < 0) {
    error |= kNegativeSize;
    return kNegativeSize;
    }
    if (list->data == NULL) {
    error |= kNullData;
    return kNullData;
    }
    if (list->next == NULL) {
    error |= kNullNext;
    return kNullNext;
    }
    if (list->prev == NULL) {
    error |= kNullPrev;
    return kNullPrev;
    }
    if (list->next && (GetListHeadPos(list) < 0 || GetListHeadPos(list) >= list->size)) {
    error |= kInvalidHead;
    }
    if (list->prev && (GetListTailPos(list) < 0 || GetListTailPos(list) >= list->size)) {
    error |= kInvalidTail;
    }
    if (list->free < 0 || list->free >= list->size) {
    error |= kInvalidFree;
    }
    for (int i = 0; i < list->size; i++) {
    if (list->data[i] == HT_POISON && list->prev[i] != -1 && i != list->size - 1 && i != 0) {
    error |= kInvalidUnusedPos;
    return (ListErrors)error;
    }
    if (list->data[i] != HT_POISON && list->prev[i] == -1) {
    error |= kInvalidUnusedPos;
    return (ListErrors)error;
    }
    }
    for (int i = 0; i < list->size; i++) {
    int next = list->next[i];
    if ((next >= 0 && next <= list->size - 1 && list->data[i] != HT_POISON && list->data[next] != HT_POISON) 
    && list->prev[next] != i && i != list->size - 1) {
    printf("%d ", i);
    error |= kWrongDirection;
    return kWrongDirection;
    }
    }
    #ifdef _DEBUG
    if (list->data[0] != (List_t)canary_left || list->data[list->size - 1] != (List_t)canary_right) {
            error |= kErrorWrongCanary;
    }
    #endif
    for (int i = 0; i < list->size; i++) {
    int next = list->next[i];
    int prev = list->prev[i];
    if (!(next == -1 || (next >= 0 && next < list->size))) {
    error |= kInvalidNext;
    fprintf(stderr, "Invalid next: idx=%d next=%d\n", i, next);
    return (ListErrors)error;
    }
    if (!(prev == -1 || (prev >= 0 && prev < list->size))) {
    error |= kInvalidPrev;
    fprintf(stderr, "Invalid prev: idx=%d prev=%d\n", i, prev);
    return (ListErrors)error;
    }
    }
    int cnt = 1;
    for (int i = list->free; ((i == 0 && list->next[i] == list->free) || i != 0) && i != -1; i = list->next[i], cnt++) {
            //printf("%d ", i);
    if (cnt > list->size - list->number_of_elem) {
    error |= kHasCycleFree;
    break;
    }
    }
    cnt = 0;
    for (int i = GetListHeadPos(list); list->next[i] != 0; i = list->next[i], cnt++) {
    if (cnt > list->number_of_elem) {
    error |= kHasCycleNext;
    }
    }
    if (cnt < list->number_of_elem - 1) {
    error |= kHasSmallCycleNext;
    }
    cnt = 0;
    for (int i = GetListTailPos(list); list->prev[i] != 0; i = list->prev[i], cnt++) {
    if (cnt > list->number_of_elem) {
    error |= kHasCyclePrev;
    }
    }
    if (cnt < list->number_of_elem - 1) {
    error |= kHasSmallCyclePrev;
    }
        // if (error != 0) {
        //     ListDump(list, error);
        // }
    return (ListErrors)error;
}

ListErrors HashTableVerify(HashTable *hash_table, HashFunc Hash) {
    if (hash_table == NULL) {
        return kNullHashTable;
    }

    if (hash_table->buckets == NULL) {
        return kNullBuckets;
    }

    unsigned int error = 0;

    if (hash_table->capacity == 0) {
        error |= kInvalidCapacity;
        return (ListErrors)error;
    }

    if (!(hash_table->load_factor > 0.0f)) {
        error |= kInvalidLoadFactor;
    }

    size_t counted = 0;

    for (size_t i = 0; i < hash_table->capacity; i++) {
        ChainList *chain = &hash_table->buckets[i];

        ListErrors list_error = ListVerify(chain);
        if (list_error != kSuccess) {
            error |= kBucketCorrupt;
            fprintf(stderr, "HashTableVerify: bucket %zu is corrupt (ListVerify=0x%x)\n",
                i, (unsigned)list_error);
            continue;
        }

        if (chain->next == NULL || chain->data == NULL) {
            error |= kBucketCorrupt;
            continue;
        }

        int cur = chain->next[0];
        size_t bucket_count = 0;
        size_t max_iters = (size_t)chain->size + 1;

        while (cur != 0 && bucket_count < max_iters) {
            const char *value = chain->data[cur];

            if (value != HT_POISON && value != NULL) {
                unsigned int hash = Hash(value) % hash_table->capacity;
                if ((size_t)hash != i) {
                    error |= kWrongBucket;
                    fprintf(stderr, "HashTableVerify: value in bucket %zu hashes to %u\n", i, hash);
                }
            }

            bucket_count++;
            cur = chain->next[cur];
        }

        if (bucket_count != (size_t)chain->number_of_elem) {
            error |= kSizeMismatch;
            fprintf(stderr, "HashTableVerify: bucket %zu walk=%zu vs number_of_elem=%d\n",
                i, bucket_count, chain->number_of_elem);
        }

        counted += bucket_count;
    }

    if (counted != hash_table->size) {
        error |= kSizeMismatch;
        fprintf(stderr, "HashTableVerify: total counted=%zu vs hash_table->size=%zu\n",
            counted, hash_table->size);
    }

    return (ListErrors)error;
}