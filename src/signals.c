#include "shell.h"
#include <termios.h>

pid_t fg_pid = 0;  // global foreground process ID

// Handler for Ctrl+C (SIGINT)
void sigint_handler(int sig) {
    if (fg_pid != 0) {
        // Send SIGINT to foreground process group
        kill(-fg_pid, SIGINT);
    } else {
        printf("\nUse 'exit' to quit.\n%s", PROMPT);
        fflush(stdout);
    }
}

// Handler for Ctrl+Z (SIGTSTP)
void sigtstp_handler(int sig) {
    if (fg_pid != 0) {
        kill(-fg_pid, SIGTSTP);
        printf("\nProcess %d suspended\n", fg_pid);
    } else {
        printf("\n");
        fflush(stdout);
    }
}

// Setup signal handling for the shell
void setup_signal_handlers(void) {
    struct sigaction sa_int, sa_tstp;

    // Ignore signals that might background or stop the shell itself
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa_int, NULL);

    sa_tstp.sa_handler = sigtstp_handler;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sa_tstp, NULL);

    // Give terminal control to the shell
    tcsetpgrp(STDIN_FILENO, getpid());
}

