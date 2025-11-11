#include "shell.h"

typedef struct {
    pid_t pid;
    char cmd[256];
    int active;
    int stopped;
} Job;

static Job jobs[MAX_JOBS];
static int job_count = 0;
extern pid_t fg_pid;

// Utility functions
void add_job(pid_t pid, const char *cmd) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        strncpy(jobs[job_count].cmd, cmd, 255);
        jobs[job_count].cmd[255] = '\0';
        jobs[job_count].active = 1;
        jobs[job_count].stopped = 0;
        job_count++;
    }
}

void check_jobs(void) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].active) {
            int status;
            pid_t result = waitpid(jobs[i].pid, &status, WNOHANG);
            if (result > 0) {
                jobs[i].active = 0;
                printf("[Job %d] %d finished: %s\n", i + 1, jobs[i].pid, jobs[i].cmd);
            }
        }
    }
}

void print_jobs(void) {
    printf("\nActive background jobs:\n");
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].active)
            printf("[%d] PID: %d  CMD: %s%s\n", i + 1, jobs[i].pid, jobs[i].cmd, jobs[i].stopped ? " (stopped)" : "");
    }
    printf("\n");
}

void bring_job_foreground(int job_id) {
    if (job_id <= 0 || job_id > job_count || !jobs[job_id - 1].active) {
        printf("No such job.\n");
        return;
    }

    pid_t pid = jobs[job_id - 1].pid;
    fg_pid = pid;
    jobs[job_id - 1].stopped = 0;

    // Give control of terminal
    tcsetpgrp(STDIN_FILENO, pid);

    // Send continue signal
    kill(-pid, SIGCONT);

    int status;
    waitpid(pid, &status, WUNTRACED);

    tcsetpgrp(STDIN_FILENO, getpid());
    fg_pid = 0;
}

int execute_single(char **arglist, int background);
int execute_pipeline(char ***cmds, int n);

// Entry point
int execute(char **arglist) {
    if (!arglist || !arglist[0]) return 0;

    // Handle command chaining ';'
    int start = 0;
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], ";") == 0) {
            arglist[i] = NULL;
            execute_single(&arglist[start], 0);
            start = i + 1;
        }
    }
    execute_single(&arglist[start], 0);
    check_jobs();
    return 1;
}

// Execute pipeline commands (cmd1 | cmd2 | cmd3)
int execute_pipeline(char ***cmds, int n) {
    int pipes[n - 1][2];
    pid_t pids[n];

    for (int i = 0; i < n - 1; i++) {
        pipe(pipes[i]);
    }

    for (int i = 0; i < n; i++) {
        pids[i] = fork();

        if (pids[i] == 0) {
            setpgid(0, 0);
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            if (i > 0)
                dup2(pipes[i - 1][0], STDIN_FILENO);
            if (i < n - 1)
                dup2(pipes[i][1], STDOUT_FILENO);

            for (int j = 0; j < n - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(cmds[i][0], cmds[i]);
            perror("execvp");
            exit(1);
        }
    }

    for (int i = 0; i < n - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < n; i++) {
        waitpid(pids[i], NULL, 0);
    }

    return 1;
}

// Main command executor
int execute_single(char **arglist, int background) {
    if (arglist == NULL || arglist[0] == NULL)
        return 0;

    // Detect pipeline
    int num_pipes = 0;
    for (int i = 0; arglist[i] != NULL; i++)
        if (strcmp(arglist[i], "|") == 0)
            num_pipes++;

    if (num_pipes > 0) {
        char ***cmds = malloc((num_pipes + 1) * sizeof(char **));
        int cmd_index = 0, start = 0;
        for (int i = 0; arglist[i] != NULL; i++) {
            if (strcmp(arglist[i], "|") == 0) {
                arglist[i] = NULL;
                cmds[cmd_index++] = &arglist[start];
                start = i + 1;
            }
        }
        cmds[cmd_index++] = &arglist[start];
        execute_pipeline(cmds, cmd_index);
        free(cmds);
        return 1;
    }

    // Check for background '&'
    int i;
    for (i = 0; arglist[i] != NULL; i++);
    if (i > 0 && strcmp(arglist[i - 1], "&") == 0) {
        background = 1;
        arglist[i - 1] = NULL;
    }

    // Handle redirection <, >, >>
    int in_redirect = -1, out_redirect = -1, append_redirect = -1;
    char *infile = NULL, *outfile = NULL;

    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "<") == 0) {
            in_redirect = i;
            infile = arglist[i + 1];
        } else if (strcmp(arglist[i], ">") == 0) {
            out_redirect = i;
            outfile = arglist[i + 1];
        } else if (strcmp(arglist[i], ">>") == 0) {
            append_redirect = i;
            outfile = arglist[i + 1];
        }
    }

    pid_t pid = fork();
    if (pid == 0) {
        setpgid(0, 0);
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        // Redirection
        if (in_redirect != -1 && infile) {
            int fd = open(infile, O_RDONLY);
            if (fd < 0) { perror("open infile"); exit(1); }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (out_redirect != -1 && outfile) {
            int fd = open(outfile, O_CREAT | O_WRONLY | O_TRUNC, 0644);
            if (fd < 0) { perror("open outfile"); exit(1); }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        if (append_redirect != -1 && outfile) {
            int fd = open(outfile, O_CREAT | O_WRONLY | O_APPEND, 0644);
            if (fd < 0) { perror("open outfile"); exit(1); }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // Clean args
        char *clean_args[MAXARGS + 1];
        int j = 0;
        for (int i = 0; arglist[i] != NULL; i++) {
            if (strcmp(arglist[i], "<") == 0 || strcmp(arglist[i], ">") == 0 || strcmp(arglist[i], ">>") == 0) {
                i++;
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
        setpgid(pid, pid);
        if (background) {
            printf("[Job %d] %d running in background: %s\n", job_count + 1, pid, arglist[0]);
            add_job(pid, arglist[0]);
        } else {
            fg_pid = pid;
            tcsetpgrp(STDIN_FILENO, pid);
            int status;
            waitpid(pid, &status, WUNTRACED);
            tcsetpgrp(STDIN_FILENO, getpid());

            if (WIFSTOPPED(status)) {
                printf("\nProcess %d suspended\n", pid);
                add_job(pid, arglist[0]);
            }

            fg_pid = 0;
        }
    } else {
        perror("fork");
    }

    return 1;
}

