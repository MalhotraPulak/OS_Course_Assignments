#include "headers.h"
#include "util.h"
#include <signal.h>
#include "history_handler.h"
#include "zombie_killer.h"
#include "parser.h"

char *shellName;


void rip_child(int signum) {
    if (signum == SIGCHLD)
        zombie_process_check();
}

void exit_2(int signum) {
    if (signum == SIGINT) {
        killbg();
        _exit(0);
    }
}


int main() {
    clearScreen();
    welcomeMessage();
    shellName = getShellName();
    if (getcwd(homeDir, size_buff) == NULL) {
        perror("getcwd failed");
    }
    if (homeDir[strlen(homeDir) - 1] != '/') {
        strcat(homeDir, "/");
    }
    signal(SIGCHLD, rip_child);
    signal(SIGINT, exit_2);
    strcpy(currDir, homeDir);
    updateShowDir();
    while (1) {
        printCyan();
        printf("%s", shellName);
        printGreen();
        printf("%s ", showDir);
        printYellow();
        printf("$ ");
        resetColor();
        char *line = malloc(size_buff);
        char *line2 = line;
        fgets(line, size_buff, stdin);
        size_t ln = strlen(line) - 1;
        if (*line && line[ln] == '\n')
            line[ln] = '\0';
        line = trim_whitespace(line);
        add_history(line);
        get_commands(line);
        free(line2);
    }

}

/*
 getenv setenv - get unset and fetch environment variables from host environment list
 signal - know already
 dup - duplicate and existing file descriptor
 dup2 - also used to duplicate
 strtok fork setpgid wait waitpid getpid kill execvp - know already
 getchar - reads a character from stdin

other -
 pipe -
 */

