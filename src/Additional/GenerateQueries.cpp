#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CommonFunctions.h"
#include "Config.h"

#define MIN_LEN 5
#define MAX_LEN 20

int main(void) {
    srand(SEED);

    char** keys = ReadString("data/tests_string.txt", NUMBER_KEYS);
    if (!keys) return 1;

    FILE *file = fopen("data/tests_queries.txt", "w");
    if (!file) {
        perror(ERROR_FILE);
        free(keys);
        return 1;
    }

    for (int i = 0; i < NUMBER_QUERIES; i++) {
        if (rand() % 2 == 0) {
            fprintf(file, "%s\n", keys[rand() % NUMBER_KEYS]);

        } else {
            int len = MIN_LEN + rand() % (MAX_LEN - MIN_LEN + 1);
            for (int j = 0; j < len; j++) {
                char c = rand() % 2 == 0 ? 'a' + rand() % 26 : 'A' + rand() % 26;
                fputc(c, file);
            }

            fputc('\n', file);
        }
    }

    fclose(file);
    free(keys);

    return 0;
}