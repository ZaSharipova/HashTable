#ifndef VERIFY_H_
#define VERIFY_H_

#include "Structs.h"

#define CHECK_ERROR_RETURN(cond)    \
    do {                            \
        err = (cond);               \
        if (err != kSuccess) {      \
            return err;             \
        }                           \
    } while (0)

#define INIT_INFO(list) \
    Info = {file, list, #list, __FILE__, 0, 0, 0, kSuccess, "\0", kDump, kDump};

ListErrors ListVerify(ChainList *list);
ListErrors HashTableVerify(HashTable *hash_table, HashFunc Hash);

#endif // VERIFY_H_