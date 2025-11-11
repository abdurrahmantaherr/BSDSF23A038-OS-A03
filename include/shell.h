#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "FCIT> "
#define HISTORY_SIZE 100
#define MAX_JOBS 20

// Function prototypes
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char** arglist);
int handle_builtin(char **args);

// History
void add_history_local(const char *line);
void print_history(void);
char *get_history_command(int index);
void free_history(void);

// Jobs
void add_job(pid_t pid, const char *cmd);
void check_jobs(void);
void print_jobs(void);
void bring_job_foreground(int job_id);   // NEW for fg

// Signals
void setup_signal_handlers(void);
void sigint_handler(int sig);
void sigtstp_handler(int sig);

// Global foreground PID
extern pid_t fg_pid;

#endif

