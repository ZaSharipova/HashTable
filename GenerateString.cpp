#include <stdio.h>
#include <stdlib.h>

int main(void) {
    srand(42);
    int n = 1000000;

    for (int i = 0; i < n; i++) {
        int length = 5 + rand() % 16;

        for (int j = 0; j < length; j++) {
            char letter = 'a' + rand() % 26;
            putchar(letter);
        }

        putchar('\n');
    }

    return 0;
}