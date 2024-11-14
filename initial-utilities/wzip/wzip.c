#include <stdio.h>

int main(int argc, char **argv) {
    if (argc == 1) {
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    } else {
        char cur = '\0';
        int cnt = 1;

        for (int i = 1; i < argc; i++) {
            FILE *file = fopen(argv[i], "r");

            if (file == NULL) {
                printf("wzip: cannot open file\n");
                return 1;
            }

            char c = (char)fgetc(file);

            while (c != EOF) {
                if (cur == '\0') {
                    cur = c;
                } else if (cur == c) {
                    ++cnt;
                } else if (c != EOF) {
                    fwrite(&cnt, sizeof(int), 1, stdout);
                    fwrite(&cur, sizeof(char), 1, stdout);
                    cur = c;
                    cnt = 1;
                };

                c = (char)fgetc(file);
            }
        }

        fwrite(&cnt, sizeof(int), 1, stdout);
        fwrite(&cur, sizeof(char), 1, stdout);
    }

    return 0;
}