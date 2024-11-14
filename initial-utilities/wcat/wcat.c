#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc == 1) {
        return 0;
    } else {
        for (int i = 1; i < argc; i++) {
            FILE *stream = fopen(argv[i], "r");
            if (stream == NULL) {
                printf("wcat: cannot open file\n");
                return 1;
            } else {
                char line[50];
                while (fgets(line, sizeof line, stream) != NULL) {
                    printf("%s", line);
                }
            }

            if (fclose(stream) != 0)
                return 1;
        }
    }

    return 0;
}