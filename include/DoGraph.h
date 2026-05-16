#ifndef DO_GRAPH_H_
#define DO_GRAPH_H_

#include "ChainTableList.h"
#include "Structs.h"

ListErrors DumpListToGraphviz(ChangeOperationContext *Info);
ListErrors DumpHashTableToGraphviz(HashTable *hash_table, ChangeOperationContext *Info);
void DoSnprintf(ChangeOperationContext *Info);

#endif //DO_GRAPH_H_