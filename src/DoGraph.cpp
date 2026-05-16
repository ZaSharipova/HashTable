#include "DoGraph.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "Structs.h"
#include "FileOperations.h"
// #include "Canary.h"
#include "Subsidiary.h"

#define DUMP_BUCKET_RADIUS 2

static void PrintGraphHeader(FILE *file);
static FillAndBorderColor GetFillColors(ChangeOperationContext *Info, int pos);

static void PrintNodes(ChangeOperationContext *Info, FILE *file, int bucket_id);
static void PrintInvisibleEdges(ChangeOperationContext *Info, FILE *file, int bucket_id);

static void FillFree(ChangeOperationContext *Info, FILE *file, int bucket_id);

static void PrintEdges(ChangeOperationContext *Info, FILE *file, int bucket_id);

ListErrors DumpListToGraphviz(ChangeOperationContext *Info) {
    assert(Info);

    FILE *file = OpenFile(FILE_FOR_GRAPH_TEXT, WRITE_MODE);
    if (!file) {
        return kErrorOpening;
    }

    if (Info->list->data && Info->list->next && Info->list->prev) {
        PrintGraphHeader(file);
        PrintNodes(Info, file, 0);

        PrintInvisibleEdges(Info, file, 0);

        PrintEdges(Info, file, 0);
        FillFree(Info, file, 0);

        fprintf(file, "}\n");
    }

    printf("Graphviz dump saved to %s\n", FILE_FOR_GRAPH_TEXT);

    return CloseFile(file);
}

ListErrors DumpHashTableToGraphviz(HashTable *hash_table, ChangeOperationContext *Info) {
    assert(hash_table);
    assert(Info);

    FILE *file = OpenFile(FILE_FOR_GRAPH_TEXT, WRITE_MODE);
    if (!file) {
        return kErrorOpening;
    }

    int capacity = (int)hash_table->capacity;
    int focus = Info->focus_bucket;

    int lo = 0;
    int hi = 0;
    if (focus < 0 || focus >= capacity) {
        lo = 0;
        hi = (capacity < 2 * DUMP_BUCKET_RADIUS + 1) ? capacity - 1 : 2 * DUMP_BUCKET_RADIUS;
    } else {
        lo = focus - DUMP_BUCKET_RADIUS;
        hi = focus + DUMP_BUCKET_RADIUS;
        if (lo < 0) lo = 0;
        if (hi >= capacity) hi = capacity - 1;
    }

    fprintf(file, "digraph HashTable {\n");
    fprintf(file, "    rankdir=TB;\n");
    fprintf(file, "    splines=ortho;\n");
    fprintf(file, "    nodesep=0.3;\n");
    fprintf(file, "    node [shape=record];\n");
    fprintf(file, "    compound=true;\n");
    fprintf(file, "    labelloc=\"t\";\n");
    fprintf(file, "    label=\"capacity=%zu  size=%zu  focus=%d  window=[%d..%d]\";\n\n",
            hash_table->capacity, hash_table->size, focus, lo, hi);

    fprintf(file, "    table [shape=record, style=filled, fillcolor=\"#F5F5DC\", label=\"");

    int first = 1;
    if (lo > 0) {
        fprintf(file, "<lo> ... %d skipped ...", lo);
        first = 0;
    }
    for (int i = lo; i <= hi; i++) {
        if (!first) fprintf(file, " | ");
        first = 0;

        const char *highlight = (i == focus) ? "  <-- focus" : "";
        fprintf(file, "<b%d> [%d]\\nn=%d%s",
                i, i, hash_table->buckets[i].number_of_elem, highlight);
    }
    if (hi < capacity - 1) {
        fprintf(file, " | <hi> ... %d skipped ...", capacity - 1 - hi);
    }
    fprintf(file, "\"];\n\n");

    ChainList *saved = Info->list;

    for (int i = lo; i <= hi; i++) {
        ChainList *chain = &hash_table->buckets[i];
        if (chain->number_of_elem == 0) continue;

        const char *cluster_color = (i == focus) ? "#B22222" : "#888888";
        const char *cluster_pen   = (i == focus) ? "\"2.5\"" : "\"1\"";

        fprintf(file, "    subgraph cluster_b%d {\n", i);
        fprintf(file, "        label = \"bucket %d%s\";\n",
                i, (i == focus) ? " (focus)" : "");
        fprintf(file, "        style    = \"rounded\";\n");
        fprintf(file, "        color    = \"%s\";\n", cluster_color);
        fprintf(file, "        penwidth = %s;\n", cluster_pen);

        Info->list = chain;
        PrintNodes(Info, file, i);
        PrintInvisibleEdges(Info, file, i);
        PrintEdges(Info, file, i);
        FillFree(Info, file, i);

        fprintf(file, "    }\n");

        int head = chain->next[0];
        fprintf(file, "    table:b%d -> node_b%d_%d [lhead=cluster_b%d, style=dashed, color=\"%s\"];\n\n",
                i, i, head, i, cluster_color);
    }

    Info->list = saved;

    fprintf(file, "}\n");

    printf("Graphviz dump saved to %s\n", FILE_FOR_GRAPH_TEXT);

    return CloseFile(file);
}

static void PrintEdges(ChangeOperationContext *Info, FILE *file, int bucket_id) {
    assert(Info);
    assert(file);

    int size = Info->list->size;

    for (int i = 0; i < size; i++) {
        int next = Info->list->next[i];

        int prev_pos = Info->list->prev[i];
        if (prev_pos >= 0 && prev_pos < size && ((Info->list->data[prev_pos] == HT_POISON && prev_pos != 0) || Info->list->next[prev_pos] != i)) {
            fprintf(file, "    node_b%d_%d -> node_b%d_%d [color=\"orange\", penwidth=\"3\"];\n", bucket_id, i, bucket_id, prev_pos);
        }

        if (next >= 0 && next < size) {
            if (Info->list->data[i] == HT_POISON && i != 0) {
                if (next > 0 && next < size) {
                    fprintf(file, "    node_b%d_%d -> node_b%d_%d [color=\"pink\"];\n", bucket_id, i, bucket_id, next);
                }
            }
            else if (Info->list->prev[next] == i) {
                if (next < i && Info->list->number_of_elem == 1)
                    continue;
                fprintf(file, "    node_b%d_%d -> node_b%d_%d [color=\"purple2\", dir=both];\n", bucket_id, i, bucket_id, next);
            }

            else {
                fprintf(file, "    node_b%d_%d -> node_b%d_%d [color=\"orange\", penwidth=\"3\"];\n", bucket_id, i, bucket_id, next);

                if (Info->list->prev[next] >= 0 && Info->list->prev[next] < size && Info->list->number_of_elem != 1) {
                    //fprintf(file, "    node_b%d_%d -> node_b%d_%d [color=\"orange\", penwidth=\"3\"];\n", bucket_id, next, bucket_id, Info->list->prev[next]);
                }
                else if (Info->list->prev[next] >= 0 && Info->list->prev[next] < size && Info->list->number_of_elem != 1) {
                    fprintf(file, "    node_b%d_%d [shape=octagon, fillcolor=\"firebrick1\", style=filled];\n", bucket_id, Info->list->prev[next]);
                }
                else if (Info->list->number_of_elem != 1 && Info->list->prev[next] >= 0) {
                    fprintf(file, "    node_b%d_%d [shape=octagon, fillcolor=\"firebrick1\", style=filled];\n", bucket_id, Info->list->prev[next]);
                    fprintf(file, "    node_b%d_%d -> node_b%d_%d [color=\"firebrick2\", penwidth=\"3\"];\n", bucket_id, Info->list->prev[next], bucket_id, next);
                }
            }
        }
        else {
            if (Info->list->data[i] == HT_POISON && i != 0) {
                if (next > 0 && next < size) {
                    fprintf(file, "    node_b%d_%d -> node_b%d_%d [color=\"pink\"];\n", bucket_id, i, bucket_id, next);
                }
            }
            else {
                fprintf(file, "    node_b%d_%d -> node_b%d_%d [color=\"firebrick2\", penwidth=\"3\"];\n", bucket_id, i, bucket_id, next);

                if (next >= 0 && next < size && !(Info->list->prev[next] >= 0 && Info->list->prev[next] < size)) {
                    fprintf(file, "    node_b%d_%d [shape=octagon, fillcolor=\"firebrick1\", style=filled];\n", bucket_id, Info->list->prev[next]);
                }
                fprintf(file, "    node_b%d_%d [shape=octagon, fillcolor=\"firebrick1\", style=filled];\n", bucket_id, next);
            }
        }


    }
}

static void PrintGraphHeader(FILE *file) {
    assert(file);

    fprintf(file, "digraph List {\n");
    fprintf(file, "    splines = ortho;\n");
    fprintf(file, "    rankdir=LR;\n");
    fprintf(file, "    node [shape=record];\n\n");
    fprintf(file, "    nodesep=0.3;\n");
    // fprintf(file, "    ranksep=1.0;\n");
}

static FillAndBorderColor GetFillColors(ChangeOperationContext *Info, int pos) {
    assert(Info);

    if (pos == 0) {
        return (FillAndBorderColor){"#20B2AA", "#008B8B"};

    } else if (Info->type_of_command_before == kDelete && pos == Info->list->free && Info->pos == Info->list->free) {
        return (FillAndBorderColor){"indianred1", "#8B0000"};

    } else if ((Info->type_of_command_before == kPopBack || Info->type_of_command_before == kPopFront) && pos == Info->pos) {
        return (FillAndBorderColor){"indianred1", "#8B0000"};

    } else if ((Info->type_of_command_before == kPushBack || Info->type_of_command_before == kPushFront) && pos == Info->pos) {
        return (FillAndBorderColor){"#7FFF00", "#32CD32"};

    } else if (Info->list->data[pos] == HT_POISON && (Info->list->prev[pos] == -1 || pos == Info->list->size - 1)
#ifdef _DEBUG
           || Info->list->data[pos] == (List_t)canary_right
#endif
    ) {
        return (FillAndBorderColor){"#fbf5eef2", "#CD853F"};

    } else if ((Info->type_of_command_before == kInsertAfter  && pos == Info->list->next[Info->pos])
    || (Info->type_of_command_before == kInsertBefore && pos == Info->list->prev[Info->pos])) {
        return (FillAndBorderColor){"#7FFF00", "#32CD32"};

    } else {
        if ((Info->list->data[pos] == HT_POISON && Info->list->prev[pos] != -1 && pos != Info->list->size - 1)
        || (Info->list->data[pos] != HT_POISON && Info->list->prev[pos] <= -1 && pos != Info->list->size - 1)
        || (Info->list->prev[pos] >= 0 && Info->list->prev[pos] < Info->list->size && Info->list->data[Info->list->prev[pos]] == HT_POISON && Info->list->prev[pos] != 0)
        || ( Info->list->next[pos] >= 0 && Info->list->next[pos] < Info->list->size && Info->list->data[Info->list->next[pos]] != HT_POISON && Info->list->next[pos] != 0 && Info->list->data[pos] == HT_POISON)) {
            return (FillAndBorderColor){"#FF0000", "#8B0000"};
        }
        return (FillAndBorderColor){"#FFE4B5", "#CD853F"};
    }
}

static void PrintNodes(ChangeOperationContext *Info, FILE *file, int bucket_id) {
    assert(Info);
    assert(file);

    if (Info->list->data && Info->list->next && Info->list->prev) {
    for (int i = 0; i < Info->list->size; i++) {
        FillAndBorderColor colors = GetFillColors(Info, i);

#ifdef _DEBUG
        const char *data_label = (i == 0 || i == Info->list->size - 1) ? "data (canary)" : "data";
#else
        const char *data_label = "data";
#endif
        fprintf(file, "    node_b%d_%d [label=\"idx: %d | %s: " LIST_SPEC " | next: %d | prev: %d\"; shape=Mrecord; style=filled; ",
                bucket_id, i, i, data_label, Info->list->data[i], Info->list->next[i], Info->list->prev[i]);

        fprintf(file, "fillcolor = \"%s\"; color = \"%s\"];\n", colors.fillColor, colors.borderColor);
    }
    }
}

static void PrintInvisibleEdges(ChangeOperationContext *Info, FILE *file, int bucket_id) {
    assert(Info);
    assert(file);

    fprintf(file, "    node_b%d_0 -> node_b%d_1", bucket_id, bucket_id);
    for (int i = 2; i < Info->list->size; i++) {
        fprintf(file, " -> node_b%d_%d", bucket_id, i);
    }
    fprintf(file, " [style=invis, weight=1000];\n\n");
}

static void FillFree(ChangeOperationContext *Info, FILE *file, int bucket_id) {
    assert(Info);
    assert(file);


    fprintf(file, "    head_b%d [shape=ellipse fillcolor=\"#DCDCDC\" style=filled label=\"head = %d\"];\n", bucket_id, Info->list->next[0]);
    fprintf(file, "    tail_b%d [shape=ellipse fillcolor=\"#DCDCDC\" style=filled label=\"tail = %d\"];\n", bucket_id, Info->list->prev[0]);

    fprintf(file, "    {rank=same; head_b%d; node_b%d_%d; }", bucket_id, bucket_id, Info->list->next[0]);
    fprintf(file, "    {rank=same; tail_b%d; node_b%d_%d; }", bucket_id, bucket_id, Info->list->prev[0]);
    fprintf(file, "    head_b%d -> node_b%d_%d [color=brown, maxlen=1];\n", bucket_id, bucket_id, Info->list->next[0]);
    fprintf(file, "    tail_b%d -> node_b%d_%d [color=brown, maxlen=1];\n", bucket_id, bucket_id, Info->list->prev[0]);

    //fprintf(file, "  { rank = same; head; tail; free; }");
    fprintf(file, "    free_b%d [shape=ellipse fillcolor=\"#DCDCDC\" style=filled label=\"free = %d\"];\n", bucket_id, Info->list->free);
    fprintf(file, "    {rank=same; free_b%d; node_b%d_%d; }", bucket_id, bucket_id, Info->list->free);
    fprintf(file, "    free_b%d -> node_b%d_%d [color=\"#8B4513\"];\n", bucket_id, bucket_id, Info->list->free);
}

void DoSnprintf(ChangeOperationContext *Info) {
    assert(Info);

    snprintf(Info->image_file, sizeof(Info->image_file), "Images/graph_%d.svg", Info->graph_counter);
    (Info->graph_counter)++;
    char cmd[40] = {};
    snprintf(cmd, sizeof(cmd), "dot output.txt -T svg -o %s", Info->image_file);

    system(cmd);
}