#ifndef FILE_OPERATIONS_H_
#define FILE_OPERATIONS_H_

#include <stdio.h>

#include "Structs.h"

FILE *OpenFile(const char *filename, const char *mode);
ListErrors CloseFile(FILE *file);

#endif //FILE_OPERATIONS_H_