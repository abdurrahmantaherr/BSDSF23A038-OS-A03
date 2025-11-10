#include "shell.h"
#include <termios.h>


typedef struct {
    pid_t pid;
    char cmd[256];
    int active;
} Job;

static Job jobs[MAX_JOBS];
static int job_count = 0;

// Declare fg_pid (defined in signals.c)
extern pid_t fg_pid;

void add_job(pid_t pid, const char *cmd) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        strncpy(jobs[job_count].cmd, cmd, 255);
        jobs[job_count].cmd[255] = '\0';
        jobs[job_count].active = 1;
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
            printf("[%d] PID: %d  CMD: %s\n", i + 1, jobs[i].pid, jobs[i].cmd);
    }
    printf("\n");
}

int execute_single(char **arglist, int background);

int execute(char **arglist) {
    if (!arglist || !arglist[0]) return 0;

    // Split on ';' for chaining
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

int execute_single(char **arglist, int background) {
    if (arglist == NULL || arglist[0] == NULL) return 0;

    // Check if '&' at end → background job
    int i;
    for (i = 0; arglist[i] != NULL; i++);
    if (i > 0 && strcmp(arglist[i - 1], "&") == 0) {
        background = 1;
        arglist[i - 1] = NULL;
    }

pid_t pid = fork();

if (pid == 0) {
    // --- CHILD PROCESS ---
    setpgid(0, 0); // new process group for this child
    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);

    // Give terminal control to this process group
    tcsetpgrp(STDIN_FILENO, getpid());

    execvp(arglist[0], arglist);
    perror("execvp");
    exit(1);
} 
else if (pid > 0) {
    // --- PARENT PROCESS (SHELL) ---
    setpgid(pid, pid); // ensure child in new group

    if (background) {
        printf("[Job %d] %d running in background: %s\n", job_count + 1, pid, arglist[0]);
        add_job(pid, arglist[0]);
    } else {
        fg_pid = pid;
        // Give terminal to child process group
        tcsetpgrp(STDIN_FILENO, pid);

        int status;
        waitpid(pid, &status, WUNTRACED);

        // Return terminal control to shell
        tcsetpgrp(STDIN_FILENO, getpid());

        if (WIFSTOPPED(status)) {
            printf("\nProcess %d suspended\n", pid);
            add_job(pid, arglist[0]);
        }

        fg_pid = 0;
    }
} 
else {
    perror("fork");
}

    return 1;
}

