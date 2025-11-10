#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ------------------------------------------------------------------
   Function: read_cmd
   Purpose : Read a full command line from given input stream
------------------------------------------------------------------ */
char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    char* cmdline = (char*) malloc(sizeof(char) * MAX_LEN);
    int c, pos = 0;

    while ((c = getc(fp)) != EOF) {
        if (c == '\n') break;
        cmdline[pos++] = c;
    }

    if (c == EOF && pos == 0) {
        free(cmdline);
        return NULL; // Handle Ctrl+D (EOF)
    }

    cmdline[pos] = '\0';
    return cmdline;
}

/* ------------------------------------------------------------------
   Function: tokenize
   Purpose : Split command line into tokens (arguments)
------------------------------------------------------------------ */
char** tokenize(char* cmdline) {
    if (cmdline == NULL || cmdline[0] == '\0' || cmdline[0] == '\n') {
        return NULL;
    }

    char** arglist = (char**) malloc(sizeof(char*) * (MAXARGS + 1));
    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = (char*) malloc(sizeof(char) * ARGLEN);
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
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) {
            len++;
        }
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
        return 1; // empty line handled

    // exit
    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting shell...\n");
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
        printf("---------------------------------\n\n");
        return 1;
    }

    // jobs
    else if (strcmp(args[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }

    return 0; // not a built-in command
}

