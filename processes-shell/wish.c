#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
const char *PATH = "/home/mohab/Documents/Coding/Projects/ostep-projects/"
                   "processes-shell/path.txt";

void error() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

int get_path(char *command, char **path) {
    // Check if the command is in the PATH environment variable and return 1 if
    // it is, otherwise return 0

    FILE *file = fopen(PATH, "a+");
    if (file == NULL)
        return 1;

    char *temp_path = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&temp_path, &len, file)) != -1) {
        // Check if the command exists in the path
        char *full_path = malloc(strlen(temp_path) + strlen(command) + 2);

        if (full_path == NULL)
            return 1;

        // Remove all trailing newlines
        size_t temp_len = strlen(temp_path);
        while (temp_len > 0 && temp_path[temp_len - 1] == '\n') {
            temp_path[temp_len - 1] = '\0';
            temp_len--;
        }

        strcpy(full_path, temp_path);
        strcat(full_path, "/");
        strcat(full_path, command);

        if (access(full_path, F_OK) != -1 && access(full_path, X_OK) != -1) {
            *path = full_path;
            free(temp_path);
            fclose(file);
            return 0;
        }
        free(full_path);
    }

    free(temp_path);
    fclose(file);

    return 1;
}

int redirect_output(char *filename) {
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1)
        return 1;

    // Redirect stdout to the file
    if (dup2(fd, STDOUT_FILENO) == -1) {
        close(fd);
        return 1;
    }

    // Redirect stderr to the same file
    if (dup2(fd, STDERR_FILENO) == -1) {
        close(fd);
        return 1;
    }

    return 0;
}

int needs_redirection(int argc, char *argv[]) {
    // rc = -1 if there's an error/failure
    // rc = -2 if there's no need for redirection
    // otherwise, rc = the index of '>'

    int redirections = 0;
    for (int i = 0; i < argc; i++)
        if (strcmp(argv[i], ">") == 0)
            if (++redirections > 1 || i == argc - 1)
                return -1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], ">") == 0) {
            if (i == argc - 1 || i + 1 != argc - 1)
                return -1;

            char *filename = argv[i + 1];
            if (redirect_output(filename) == 1)
                return -1;

            return i;
        }
    }

    return -2;
}

int execute(int argc, char *argv[]) {
    char *path = NULL;
    if (get_path(argv[0], &path) == 1)
        return 1;

    int redirection_rc = needs_redirection(argc, argv);

    if (redirection_rc == -1) {
        free(path);
        return 1;
    } else if (redirection_rc == -2) {
        // No redirection needed
        execv(path, argv);
        return 2;
    } else {
        // Redirection needed
        // redirection_rc here is the index of '>'
        argv[redirection_rc] = NULL;
        if (redirect_output(argv[redirection_rc + 1]) == 1) {
            free(path);
            return 1;
        }
        execv(path, argv);
        return 2;
    }
}

char *append(char *a, char b) {
    size_t len = (a != NULL) ? strlen(a) : 0;

    // One for new char, one for null terminator
    char *result = malloc(len + 2);

    if (result == NULL)
        return NULL;

    if (a != NULL)
        strcpy(result, a);
    result[len] = b;
    result[len + 1] = '\0';

    if (a != NULL)
        free(a);
    return result;
}

char ***parse(char *line, char sep) {
    char ***commands = calloc(64, sizeof(char **));

    if (commands == NULL)
        return NULL;

    int cidx = 0;  // Command index
    int aidx = 0;  // Argument index

    // Initialize first command and its first argument

    // Pre-allocate space for 64 arguments
    commands[cidx] = calloc(64, sizeof(char *));

    if (commands[cidx] == NULL) {
        free(commands);
        return NULL;
    }

    commands[cidx][aidx] = NULL;  // Initialize first argument as empty string

    size_t line_len = strlen(line);
    for (size_t i = 0; i < line_len; i++) {
        char c = line[i];

        if (c == sep) {
            // Move to next command if current argument is not empty
            if (commands[cidx][aidx] != NULL) {
                aidx++;
                commands[cidx][aidx] = NULL;
            }
            cidx++;    // Move to next command
            aidx = 0;  // The first argument of the new command

            commands[cidx] = calloc(64, sizeof(char *));
            if (commands[cidx] == NULL)
                return NULL;
            commands[cidx][aidx] = NULL;

        } else if (c == ' ' || c == '\n' || c == '\t') {
            // Move to next argument if current is not empty
            if (commands[cidx][aidx] != NULL &&
                strlen(commands[cidx][aidx]) > 0) {
                aidx++;
                commands[cidx][aidx] = NULL;
            }
        } else if (c == '>') {
            // Add redirection as a separate argument
            // to avoid that case (e.g. ls>file)
            if (commands[cidx][aidx] != NULL &&
                strlen(commands[cidx][aidx]) > 0) {
                aidx++;
            }
            commands[cidx][aidx] = append(commands[cidx][aidx], c);
            if (commands[cidx][aidx] == NULL)
                return NULL;
            aidx++;
            commands[cidx][aidx] = NULL;
        } else {
            // Append character to current argument
            commands[cidx][aidx] = append(commands[cidx][aidx], c);
            if (commands[cidx][aidx] == NULL)
                return NULL;
        }
    }

    // Null terminate the commands list (execv requires that)
    commands[cidx + 1] = NULL;

    return commands;
}

void setup_path() {
    // Every time the shell starts, /bin should be there by
    // default until the user changes it

    FILE *file = fopen(PATH, "w");
    if (file == NULL)
        error();

    fprintf(file, "/bin");
    fclose(file);
}

char *remove_trailing_whitespace(char *line) {
    char *start = line;
    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }

    char *end = line + strlen(line) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        *end = '\0';
        end--;
    }

    return start;
}

void handle_input(char *line) {
    line = remove_trailing_whitespace(line);

    if (strlen(line) == 0 || strcmp(line, "&") == 0)
        return;

    size_t line_len = strlen(line);
    if (line[line_len - 1] == '&') {
        line[line_len - 1] = '\0';
    }

    char ***commands = parse(line, '&');
    if (commands == NULL) {
        error();
        return;
    }

    // Keep track of child processes
    pid_t *child_pids = malloc(sizeof(pid_t) * 1024);
    int num_children = 0;

    // Execute each command
    for (int i = 0; commands[i] != NULL; i++) {
        int new_argc = 0;
        while (commands[i][new_argc] != NULL)
            new_argc++;

        if (new_argc == 0)
            continue;

        char *command = commands[i][0];

        if (strcmp(command, "exit") == 0) {
            if (new_argc != 1) {
                error();
                continue;
            }
            free(child_pids);
            exit(0);
        } else if (strcmp(command, "cd") == 0) {
            if (new_argc != 2) {
                error();
                continue;
            }
            if (chdir(commands[i][1]) == -1) {
                error();
                continue;
            }
        } else if (strcmp(command, "path") == 0) {
            FILE *file = fopen(PATH, "w+");
            if (file == NULL) {
                error();
                continue;
            }
            if (new_argc > 1) {
                for (int j = 1; j < new_argc; j++)
                    fprintf(file, "%s\n", commands[i][j]);
            }
            fclose(file);
        } else {
            pid_t pid = fork();
            if (pid < 0) {
                error();
            } else if (pid == 0) {
                // Child process
                free(child_pids);
                if (execute(new_argc, commands[i]) == 1) {
                    error();
                    exit(1);
                }
                exit(0);
            } else {
                // Parent process

                // indicate that a new child process has been created
                child_pids[num_children++] = pid;
            }
        }
    }

    // Wait for all child processes
    for (int i = 0; i < num_children; i++) {
        waitpid(child_pids[i], NULL, 0);
    }

    free(child_pids);

    // Free the commands array
    for (int i = 0; commands[i] != NULL; i++) {
        for (int j = 0; commands[i][j] != NULL; j++) {
            free(commands[i][j]);
        }
        free(commands[i]);
    }
    free(commands);
}

int main(int argc, char *argv[]) {
    setup_path();

    if (argc == 1) {
        // Interactive mode
        while (1) {
            printf("wish> ");
            char *line = NULL;
            size_t len = 0;
            ssize_t read = getline(&line, &len, stdin);

            if (read == -1) {
                free(line);
                break;
            }

            handle_input(line);
            free(line);
        }
    } else if (argc == 2) {
        // Batch mode
        FILE *file = fopen(argv[1], "r");
        if (file == NULL) {
            error();
            return 1;
        }

        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        while ((read = getline(&line, &len, file)) != -1)
            handle_input(line);

        free(line);
        fclose(file);
    } else {
        error();
        return 1;
    }

    return 0;
}