#include "DoDump.h"

#include <stdio.h>

#include "Structs.h"
#include "Verify.h"
#include "Subsidiary.h"
#include "DoGraph.h"

#include <time.h>
#include <assert.h>
#include <math.h>
#include <string.h>

static void PrintChangeDescription(FILE *file, ChangeOperationContext *Info);
static void PrintCurrentTime(FILE *file, const char *label);
static void PrintListInfoItem(FILE *file, const char *label, int value);

static void PrintTableStart(FILE *file);
static void PrintTableEnd(FILE *file);
static void PrintTableHeader(FILE *file, int size);
static void PrintTableRowData(FILE *file, const char *label, int size, int *array);
static void PrintTableRowListSpec(FILE *file, const char *label, int size, ChainList *list);

static ChainList *GetFocusedChain(ChangeOperationContext *Info) {
    assert(Info);

    if (Info->hash_table == NULL) return Info->list;

    int focus = Info->focus_bucket;
    if (focus < 0 || (size_t)focus >= Info->hash_table->capacity) {
        return Info->list;
    }
    return &Info->hash_table->buckets[focus];
}

void DoDump(ChangeOperationContext *Info) {
    assert(Info);

    unsigned int bit = 1;
    fprintf(Info->file, "<h2> <font color=\"red\"> DUMP Listing Error</h2> </font>  \n");
    fprintf(Info->file, "<h2> <font color = \"red\"> %s</font> </h2> ", Info->message);
    fprintf(Info->file, "<h3> errors: </h3>");
    for (unsigned long long i = 0; i < NUMBER_OF_ERRORS; i++) {
        if (Info->error & bit) {
            fprintf(Info->file, "<h4> <font color=\"red\"> %s </font> </h4> \n", ListErrorString[i]);
        }
        bit <<= 1;
    }
    fprintf(Info->file, "<br>");

    PrintChangeDescription(Info->file, Info);
    fprintf(Info->file, "<h4 style=\"margin: 3px 0;\">%s {%s} </h4>\n", Info->var_name, Info->filename);
    PrintCurrentTime(Info->file, "made");
    fprintf(Info->file, "<br>");

    if (Info->hash_table) {
        fprintf(Info->file, "<h3 style=\"margin: 3px 0;\"> HashTable </h3>");
        PrintListInfoItem(Info->file, "Capacity",     (int)Info->hash_table->capacity);
        PrintListInfoItem(Info->file, "Size",         (int)Info->hash_table->size);
        PrintListInfoItem(Info->file, "Focus bucket", Info->focus_bucket);
        fprintf(Info->file, "<br>");
    }

    ChainList *saved_list = Info->list;
    ChainList *focus_chain = GetFocusedChain(Info);
    Info->list = focus_chain;

    if (Info->list && Info->list->data && Info->list->next && Info->list->prev) {
        if (Info->hash_table) {
            fprintf(Info->file, "<h3 style=\"margin: 3px 0;\"> Bucket [%d] </h3>", Info->focus_bucket);
        }
        PrintListInfoItem(Info->file, "Capacity", Info->list->size);
        PrintListInfoItem(Info->file, "Size",     Info->list->number_of_elem);
        PrintListInfoItem(Info->file, "Head",     Info->list->next[0]);
        PrintListInfoItem(Info->file, "Tail",     Info->list->prev[0]);
        PrintListInfoItem(Info->file, "Free",     Info->list->free);
    }

    fprintf(Info->file, "<br>");

    if (Info->list && Info->list->data && Info->list->next && Info->list->prev) {
        PrintTableStart(Info->file);
        PrintTableHeader(Info->file, Info->list->size);
        PrintTableRowListSpec(Info->file, "Data", Info->list->size, Info->list);
        PrintTableRowData(Info->file, "Next", Info->list->size, Info->list->next);
        PrintTableRowData(Info->file, "Prev", Info->list->size, Info->list->prev);
        PrintTableEnd(Info->file);
    }

    Info->list = saved_list;

    fprintf(Info->file, "<br><br>\n");

    fprintf(Info->file, "<img src = %s>", Info->image_file);
    fprintf(Info->file, "<hr style=\"height:1px; background-color:black; border:none; width:100%%;\">\n");
    fprintf(Info->file, "<br>");
}

static void PrintChangeDescription(FILE *file, ChangeOperationContext *Info) {
    assert(file);
    assert(Info);

    bool is_before = (Info->type_of_command_before == kDumpBefore);
    int command_type = is_before ? Info->type_of_command_after : Info->type_of_command_before;
    
    if (Info->type_of_command_before == kDumpErrors) {
        //fprintf(file, "<h3> Error Listing </h3>");
        return;
    }

    if (Info->type_of_command_before == kDump) {
        return;
    }

    const char *time_label = is_before ? "BEFORE" : "AFTER";
    const char *color = is_before ? "#8B4513" : 
        (command_type == kDelete || command_type == kPopBack || command_type == kPopFront) ? "#DC143C" : "#7FFF00";

    fprintf(file, " <font color=\"%s\"> %s </font> ", color, time_label);
    
    if (command_type == kInsertAfter || command_type == kInsertBefore) {
        const char *position = (command_type == kInsertAfter) ? "AFTER" : "BEFORE";
        fprintf(file, "<font color=\"%s\"> %s </font> value <font color=\"%s\"> %d </font> %s position <font color=\"%s\">%d</font></h3>\n",
                color, "INSERT", color, Info->number, position, color, Info->pos);
    } else if (command_type == kDelete || command_type == kPopBack || command_type == kPopFront) {
        fprintf(file, "<font color=\"%s\"> %s </font> value from position <font color=\"%s\">%d</font></h3>\n",
                color, "DELETE", color, Info->pos);
    } else if (command_type == kPushBack || command_type == kPushFront) {
            fprintf(file, "<font color=\"%s\"> %s </font> value <font color=\"%s\"> %d </font> %s position <font color=\"%s\">%d</font></h3>\n",
                color, "INSERT", color, Info->number, "AFTER", color, Info->pos);
    }
}

static void PrintCurrentTime(FILE *file, const char *label) {
    assert(file);
    assert(label);

    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);

    char buf_time[MAX_STRING_SIZE] = {};
    strftime(buf_time, sizeof(buf_time), "%Y-%m-%d %H:%M:%S\n", tm_now);

    fprintf(file, "<h4 style=\"margin: 3px 0;\">%s: %s</h4>\n", label, buf_time);
}

static void PrintListInfoItem(FILE *file, const char *label, int value) {
    assert(file);
    assert(label);

    fprintf(file, "<h4 style=\"margin: 3px 0;\"> %s: %d </h4>\n", label, value);
}

static void PrintTableStart(FILE *file) {
    assert(file);

    fprintf(file, "<table border=\"2\" style=\"border-radius: 5px; overflow: hidden; border-collapse: separate; border-spacing: 0; width: 30%%; font-size: 18px; text-align: center; margin-left: 0px;\">\n");
}

static void PrintTableEnd(FILE *file) {
    assert(file);

    fprintf(file, "</table>\n");
}

static void PrintTableHeader(FILE *file, int size) {
    assert(file);

    fprintf(file, "<tr>");
    fprintf(file, "<th style=\"padding: 10px 24px; align: left; text-align: center; background-color: #fbf5eef2;\">Index</th>");
    for (int i = 0; i < size; i++) {
        fprintf(file, "<th style=\"padding: 10px 24px; text-align: center; background-color: #fbf5eef2\">%d</th>", i);
    }
    fprintf(file, "</tr>\n");
}

static void PrintTableRowData(FILE *file, const char *label, int size, int *array) {
    assert(file);
    assert(label);
    assert(array);

    fprintf(file, "<tr>");
    fprintf(file, "<td style=\"padding: 10px 24px; text-align: center; background-color: #fbf5eef2;\">%s</td>", label);
    for (int i = 0; i < size; i++) {
        fprintf(file, "<td>%d</td>", array[i]);
    }
    fprintf(file, "</tr>\n");
}

static void PrintTableRowListSpec(FILE *file, const char *label, int size, ChainList *list) {
    assert(file);
    assert(label);
    assert(list);

    fprintf(file, "<tr>");
    fprintf(file, "<td style=\"padding: 10px 24px; text-align: center; background-color: #fbf5eef2;\">%s</td>", label);

    for (int i = 0; i < size; i++) {
#ifdef _DEBUG
        if (i == 0 || i == list->size - 1) {
            fprintf(file, "<td> " LIST_SPEC " (canary) </td>", abs(list->data[i]));
        } else {
            fprintf(file, "<td> " LIST_SPEC " </td>", list->data[i]);
        }
#else
        fprintf(file, "<td> " LIST_SPEC " </td>", list->data[i]);
#endif
    }
    fprintf(file, "</tr>\n");
}

void DoAllDump(ChangeOperationContext *Info) {
    assert(Info);

    DumpListToGraphviz(Info);
    DoSnprintf(Info);

    DoDump(Info);
}

void DoAllDumpHashTable(ChangeOperationContext *Info) {
    assert(Info);

    DumpHashTableToGraphviz(Info->hash_table, Info);
    DoSnprintf(Info);

    DoDump(Info);
}
