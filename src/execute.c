#include "shell.h"

int execute(char **arglist) {
    if (arglist == NULL || arglist[0] == NULL)
        return 0;

    // Step 1: Detect pipe symbol
    int pipe_index = -1;
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "|") == 0) {
            pipe_index = i;
            break;
        }
    }

    // ---------------------------------------------------------------
    // PIPE HANDLING
    // ---------------------------------------------------------------
    if (pipe_index != -1) {
        arglist[pipe_index] = NULL; // split left/right commands

        char *left[MAXARGS + 1];
        char *right[MAXARGS + 1];
        int i, j;

        for (i = 0; i < pipe_index; i++)
            left[i] = arglist[i];
        left[i] = NULL;

        for (j = 0; arglist[pipe_index + 1 + j] != NULL; j++)
            right[j] = arglist[pipe_index + 1 + j];
        right[j] = NULL;

        int fd[2];
        if (pipe(fd) == -1) {
            perror("pipe");
            return 1;
        }

        pid_t pid1 = fork();
        if (pid1 == 0) {
            dup2(fd[1], STDOUT_FILENO);
            close(fd[0]);
            close(fd[1]);
            execvp(left[0], left);
            perror("execvp");
            exit(1);
        }

        pid_t pid2 = fork();
        if (pid2 == 0) {
            dup2(fd[0], STDIN_FILENO);
            close(fd[1]);
            close(fd[0]);
            execvp(right[0], right);
            perror("execvp");
            exit(1);
        }

        close(fd[0]);
        close(fd[1]);
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
        return 1;
    }

    // ---------------------------------------------------------------
    // REDIRECTION HANDLING (<, >)
    // ---------------------------------------------------------------
    int in_redirect = -1, out_redirect = -1;
    char *infile = NULL, *outfile = NULL;

    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "<") == 0) {
            in_redirect = i;
            infile = arglist[i + 1];
        } else if (strcmp(arglist[i], ">") == 0) {
            out_redirect = i;
            outfile = arglist[i + 1];
        }
    }

    pid_t pid = fork();

    if (pid == 0) {
        // Input redirection
        if (in_redirect != -1 && infile != NULL) {
            int fd = open(infile, O_RDONLY);
            if (fd < 0) {
                perror("open infile");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // Output redirection
        if (out_redirect != -1 && outfile != NULL) {
            int fd = open(outfile, O_CREAT | O_WRONLY | O_TRUNC, 0644);
            if (fd < 0) {
                perror("open outfile");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // Remove redirection operators and filenames before exec
        char *clean_args[MAXARGS + 1];
        int j = 0;
        for (int i = 0; arglist[i] != NULL; i++) {
            if (strcmp(arglist[i], "<") == 0 || strcmp(arglist[i], ">") == 0) {
                i++; // skip next token (filename)
                continue;
            }
            clean_args[j++] = arglist[i];
        }
        clean_args[j] = NULL;

        execvp(clean_args[0], clean_args);
        perror("execvp");
        exit(1);
    } 
    else if (pid > 0) {
        waitpid(pid, NULL, 0);
    } 
    else {
        perror("fork");
    }

    return 1;
}

