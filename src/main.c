#include "shell.h"

int main() {
    char* cmdline;
    char** arglist;

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {

        // Handle !n before anything
        if (cmdline[0] == '!') {
            int n = atoi(cmdline + 1);
            char *old_cmd = get_history_command(n);
            if (old_cmd == NULL) {
                fprintf(stderr, "No such command in history: %d\n", n);
                free(cmdline);
                continue;
            }
            printf("%s\n", old_cmd);
            free(cmdline);
            cmdline = old_cmd;
        }

        // Add command to history
        add_history_local(cmdline);

        if ((arglist = tokenize(cmdline)) != NULL) {
            if (handle_builtin(arglist)) {
                // Free memory and skip external execution
                for (int i = 0; arglist[i] != NULL; i++) {
                    free(arglist[i]);
                }
                free(arglist);
                free(cmdline);
                continue;
            }

            // Execute external commands
            execute(arglist);

            // Free memory allocated by tokenize()
            for (int i = 0; arglist[i] != NULL; i++) {
                free(arglist[i]);
            }
            free(arglist);
        }
        free(cmdline);
    }

    free_history();
    printf("\nShell exited.\n");
    return 0;
}

