#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>

/* ------------------------------------------------------------------
   Feature 3 & 4: Command History + Readline Integration
------------------------------------------------------------------ */
static char *history[HISTORY_SIZE];
static int hist_count = 0;

/* Add command to custom history */
void add_history_local(const char *line) {
    if (!line || strlen(line) == 0) return;

    // Avoid duplicate consecutive entries
    if (hist_count > 0 && strcmp(history[(hist_count - 1) % HISTORY_SIZE], line) == 0)
        return;

    if (hist_count < HISTORY_SIZE) {
        history[hist_count] = strdup(line);
    } else {
        free(history[0]);
        for (int i = 1; i < HISTORY_SIZE; i++)
            history[i - 1] = history[i];
        history[HISTORY_SIZE - 1] = strdup(line);
    }
    hist_count++;
}

/* Print stored history */
void print_history(void) {
    int n = (hist_count < HISTORY_SIZE) ? hist_count : HISTORY_SIZE;
    for (int i = 0; i < n; i++) {
        printf("%d  %s\n", i + 1, history[i]);
    }
}

/* Retrieve nth command (1-based) */
char *get_history_command(int index) {
    if (index <= 0 || index > hist_count)
        return NULL;
    return strdup(history[index - 1]);
}

/* Free memory at exit */
void free_history(void) {
    int n = (hist_count < HISTORY_SIZE) ? hist_count : HISTORY_SIZE;
    for (int i = 0; i < n; i++) {
        free(history[i]);
        history[i] = NULL;
    }
    hist_count = 0;
}

/* ------------------------------------------------------------------
   Function: read_cmd
   Purpose : Read a full command line from stdin using GNU Readline
------------------------------------------------------------------ */
char* read_cmd(char* prompt, FILE* fp) {
    (void)fp; // unused since readline reads from stdin

    char *cmdline = readline(prompt);

    if (cmdline == NULL) { // Ctrl+D
        printf("\n");
        return NULL;
    }

    // Only add non-empty commands
    if (strlen(cmdline) > 0) {
        add_history(cmdline);          // Readline's internal history
        add_history_local(cmdline);    // Our custom history
    }

    return cmdline; // malloc'ed by readline
}

/* ------------------------------------------------------------------
   Function: tokenize
   Purpose : Split command line into tokens (arguments)
------------------------------------------------------------------ */
char** tokenize(char* cmdline) {
    if (cmdline == NULL || cmdline[0] == '\0' || cmdline[0] == '\n')
        return NULL;

    char** arglist = malloc(sizeof(char*) * (MAXARGS + 1));
    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = malloc(sizeof(char) * ARGLEN);
        bzero(arglist[i], ARGLEN);
    }

    char* cp = cmdline;
    char* start;
    int len;
    int argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++;
        if (*cp == '\0') break;

        start = cp;
        len = 1;
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t'))
            len++;

        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }

    if (argnum == 0) {
        for (int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}

/* ------------------------------------------------------------------
   Function: handle_builtin
   Purpose : Implement internal (built-in) commands
------------------------------------------------------------------ */
int handle_builtin(char **args) {
    if (args == NULL || args[0] == NULL)
        return 1; // Empty line handled

    // exit
    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting shell...\n");
        free_history();
        exit(0);
    }

    // cd
    else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "cd: expected argument\n");
        } else if (chdir(args[1]) != 0) {
            perror("cd");
        }
        return 1;
    }

    // help
    else if (strcmp(args[0], "help") == 0) {
        printf("\nPUCFIT Custom Shell Help:\n");
        printf("---------------------------------\n");
        printf("Built-in commands:\n");
        printf("  cd <dir>   : change directory\n");
        printf("  exit       : exit the shell\n");
        printf("  help       : display this help message\n");
        printf("  jobs       : show background jobs (not yet implemented)\n");
        printf("  history    : show command history\n");
        printf("---------------------------------\n\n");
        return 1;
    }

    // jobs
   else if (strcmp(args[0], "jobs") == 0) {
    print_jobs();
    return 1;
    }


    // history
    else if (strcmp(args[0], "history") == 0) {
        print_history();
        return 1;
    }
    
    else if (strcmp(args[0], "fg") == 0) {
    if (args[1] == NULL) {
        fprintf(stderr, "Usage: fg <job_id>\n");
    } else {
        int job_id = atoi(args[1]);
        bring_job_foreground(job_id);
    }
    return 1;
}


    return 0; // Not a built-in command
}

