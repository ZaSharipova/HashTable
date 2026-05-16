#ifndef DUMP_HELPER_H
#define DUMP_HELPER_H

#include "Structs.h"
#include "Verify.h"
#include "HashFunctions.h"

#ifdef _DDUMP
#include "DoDump.h"

void DumpHelperInit (ChangeOperationContext *info, ChainList *initial_list);
void DumpHelperError(ChangeOperationContext *info, HashTable *hash_table, HashFunc Hash,
    const char *key, int index, ListErrors err, const char *op_name);

#define DUMP_HELPER_DECL(name, list_ptr) \
    ChangeOperationContext name = {};    \
    DumpHelperInit(&name, (list_ptr))

#define DUMP_HELPER_ON_ERROR(info, hash_table, hash_func, key, idx, err, op)        \
    DumpHelperError(&(info), (hash_table), (hash_func), (key), (idx), (err), (op))

#else

#define DUMP_HELPER_DECL(name, list_ptr) ((void)0)
#define DUMP_HELPER_ON_ERROR(info, hash_table, hash_func, key, idx, err, op)  ((void)0)

#endif

#endif // DUMP_HELPER_H