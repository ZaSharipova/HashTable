#include "DumpHelper.h"

#ifdef _DVERIFY

#include <stdio.h>
#include <assert.h>

#include "DoDump.h"

void DumpHelperInit(ChangeOperationContext *info, ChainList *initial_list) {
    assert(info);

    INIT_INFO(initial_list);
    info->hash_table = NULL;
    info->focus_bucket = -1;
    
    FILE *file = fopen("log.html", "w");
    info->file = file;
}

void DumpHelperError(ChangeOperationContext *info, HashTable *hash_table, HashFunc Hash,
    const char *key, int index, ListErrors err, const char *op_name) {
    assert(info);
    assert(hash_table);
    assert(Hash);
    assert(key);
    assert(op_name);

    info->hash_table = hash_table;
    info->focus_bucket = (int)(Hash(key) % hash_table->capacity);
    info->error = err;

    snprintf(info->message, sizeof(info->message), "%s failed on [%d]: %s", op_name, index, key);
    DoAllDumpHashTable(info);
}

#endif // _DDUMP