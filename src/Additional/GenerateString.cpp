#include <stdio.h>
#include <stdlib.h>

#include "Config.h"

int main(void) {
    srand(SEED);
    int n = NUMBER_KEYS;

    for (int i = 0; i < n; i++) {
        int len = 5 + rand() % 16;

        for (int j = 0; j < len; j++) {
            char letter = 'a' + rand() % 26;
            putchar(letter);
        }

        putchar('\n');
    }

    return 0;
}