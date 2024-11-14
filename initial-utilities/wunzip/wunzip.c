#include <stdio.h>

int main(int argc, char **argv) {
    if (argc == 1) {
        printf("wunzip: file1 [file2 ...]\n");
        return 1;
    } else {
        for (int i = 1; i < argc; i++) {
            FILE *file = fopen(argv[i], "r");

            if (file == NULL) {
                printf("wunzip: cannot open file\n");
                return 1;
            }

            int count;
            char c;

            while (fread(&count, sizeof(int), 1, file) != 0) {
                fread(&c, sizeof(char), 1, file);

                for (int j = 0; j < count; j++) {
                    printf("%c", c);
                }
            }
        }
    }

    return 0;
}