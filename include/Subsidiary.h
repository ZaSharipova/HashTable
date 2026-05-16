#ifndef SUBSIDIARY_H_
#define SUBSIDIARY_H_

#include <stdio.h>

#define FILE_FOR_GRAPH_TEXT "output.txt"
#define WRITE_MODE "w"
#define LIST_SPEC "%s"

#define POISON 666

typedef struct {
    const char *fillColor;
    const char *borderColor;
} FillAndBorderColor;

#define RED "\033[31m"
#define PUPRPUR "\033[35m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

#define NUMBER_OF_ERRORS sizeof(ListErrorString)/sizeof(ListErrorString[0])

static const char *ListErrorString[] = {
    "Negative Size",
    "Null Data Pointer",
    "Null Next Pointer",
    "Null Prev Pointer",
    "Negative Head Index",
    "Negative Tail Index",
    "Negative Free Index",
    "Wrong Canary Value",
    "No Memory Available",
    "Invalid Parameter",
    "Error Opening File",
    "Error Closing File",
    "Invalid Position",
    "Has Cycle in NEXT",
    "Has Cycle in PREV",
    "Has Cycle in FREE",
    "Has small Cycle in NEXT",
    "Has small Cycle in PREV",
    "Invalid Next",
    "Invalid Prev",
    "Invalid Unused Position Values",
    "Wrong Direction",
    "Failure",
};

#endif //SUBSIDIARY_H_