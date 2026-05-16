#ifndef STRUCTS_H_
#define STRUCTS_H_

#include <stdio.h>
// #include "Verify.h"

#define ALIGN_SIZE 32
#define HT_POISON NULL
#define MAX_STRING_SIZE 64

typedef const char *HT_Value;
typedef unsigned int (*HashFunc)(const char *value);

typedef enum {
    kNegativeSize       = 1 << 0,
    kNullData           = 1 << 1,
    kNullNext           = 1 << 2,
    kNullPrev           = 1 << 3,
    kInvalidHead        = 1 << 4,
    kInvalidTail        = 1 << 5,
    kInvalidFree        = 1 << 6,
    kErrorWrongCanary   = 1 << 7,

    kNoMemory           = 1 << 8,
    kInvalidParam       = 1 << 9,
    kErrorOpening       = 1 << 10,
    kErrorClosing       = 1 << 11,
    kInvalidPos         = 1 << 12,

    kHasCycleNext       = 1 << 13,
    kHasCyclePrev       = 1 << 14,
    kHasCycleFree       = 1 << 15,

    kHasSmallCycleNext  = 1 << 16,
    kHasSmallCyclePrev  = 1 << 17,

    kInvalidNext        = 1 << 18,
    kInvalidPrev        = 1 << 19,

    kInvalidUnusedPos   = 1 << 20,
    kWrongDirection     = 1 << 21,
    kFailure            = 1 << 22,

    kNullHashTable      = 1 << 23,
    kNullBuckets        = 1 << 24,
    kInvalidCapacity    = 1 << 25,
    kInvalidLoadFactor  = 1 << 26,
    kSizeMismatch       = 1 << 27,
    kWrongBucket        = 1 << 28,
    kBucketCorrupt      = 1 << 29, 

    kSuccess            =  0,
} ListErrors;

typedef enum {
    kInsertBefore,
    kInsertAfter,
    kDelete,
    kPopBack,
    kPopFront,
    kPushBack,
    kPushFront,
    kDumpBefore,
    kDump,
    kDumpErrors,
} ListCommands;

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

typedef struct ChangeOperationContext {
    FILE *file;
    ChainList *list;
    const char *var_name;
    const char *filename;      

    int pos;
    int number;
    int graph_counter;
    ListErrors error;

    char message[MAX_STRING_SIZE];
    ListCommands type_of_command_before;
    ListCommands type_of_command_after;

    char image_file[MAX_STRING_SIZE];

    HashTable *hash_table;
    int focus_bucket;
} ChangeOperationContext;

#endif // STRUCTS_H_