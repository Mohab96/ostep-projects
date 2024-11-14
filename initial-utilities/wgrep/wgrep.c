#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("wgrep: searchterm [file ...]\n");
        return 1;
    } else if (argc == 2) {
        char *line;
        long unsigned int len = 1000;

        while (getline(&line, &len, stdin) != -1) {
            for (int i = 0; i < strlen(line); i++) {
                if (line[i] == argv[1][0]) {
                    int ok = 1;

                    for (int j = 0; j < strlen(argv[1]); j++) {
                        if (line[i + j] != argv[1][j]) {
                            ok = 0;
                            break;
                        }
                    }

                    if (ok == 1) {
                        printf("%s", line);
                        break;
                    }
                }
            }
        }
    } else {
        for (int k = 2; k < argc; k++) {
            FILE *file = fopen(argv[k], "r");

            if (file == NULL) {
                printf("wgrep: cannot open file\n");
                return 1;
            }

            char *line;
            long unsigned int len = 1000;

            while (getline(&line, &len, file) != -1) {
                for (int i = 0; i < strlen(line); i++) {
                    if (line[i] == argv[1][0]) {
                        int ok = 1;

                        for (int j = 0; j < strlen(argv[1]); j++) {
                            if (line[i + j] != argv[1][j]) {
                                ok = 0;
                                break;
                            }
                        }

                        if (ok == 1) {
                            printf("%s", line);
                            break;
                        }
                    }
                }
            }
        }
    }

    return 0;
}