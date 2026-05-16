#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_WORD_LEN 32

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input> <output>\n", argv[0]);
        return 1;
    }

    FILE *in_file = fopen(argv[1], "r");
    if (!in_file) {
        perror("Error opening input");
        return 1;
    }

    FILE *out_file = fopen(argv[2], "a");
    if (!out_file) {
        perror("Error opening output");
        fclose(in_file);
        return 1;
    }

    char word[MAX_WORD_LEN + 1] = {};
    int len = 0;
    int symbol = 0;

    while ((symbol = fgetc(in_file)) != EOF) {
        if (isalpha(symbol)) {
            if (len < MAX_WORD_LEN) {
                word[len++] = (char)symbol;
            }
        } else {
            if (len > 0) {
                word[len] = '\0';
                fprintf(out_file, "%s\n", word);
                len = 0;
            }
        }
    }

    if (len > 0) {
        word[len] = '\0';
        fprintf(out_file, "%s\n", word);
    }

    fclose(in_file);
    fclose(out_file);
    return 0;
}