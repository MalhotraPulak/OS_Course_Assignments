#include "headers.h"
#include "util.h"
#include "history_handler.h"
#include "zombie_killer.h"
#include "parser.h"
#include <signal.h>

char *shellName;


void rip_child(int signum) {
    if (signum == SIGCHLD)
        zombie_process_check();
}

void quit() {
    killbg();
    printf("cya\n");
    _exit(0);
}

void printExitCode() {
    if (exit_code == 0) {
        printf(":) ");
    } else {
        printf(":( ");
    }
    exit_code = 0;
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
    //signal(SIGCHLD, rip_child);
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    //signal(SIGTTIN, SIG_IGN);
    strcpy(currDir, homeDir);
    updateShowDir();

    while (1) {
        rip_child(SIGCHLD);
        printCyan();
        printf("%s", shellName);
        printGreen();
        printf("%s ", showDir);
        printYellow();
        printf("$ ");
        resetColor();
        char *line = malloc(size_buff);
        //size_t t = size_buff;
        char *line2 = line;
        if(fgets(line, size_buff, stdin) == NULL){
             //fprintf(stderr, "NULL LOL \n");
             quit();
         }
        //getline(&line, &t, stdin);
        size_t ln = strlen(line) - 1;
        if (*line && line[ln] == '\n')
            line[ln] = '\0';
        line = trim_whitespace(line);
        add_history(line);
        getIndividualCommands(line);
        free(line2);
        printYellow();
        printExitCode();
        resetColor();
    }

}

/*
 getenv setenv - get unset and fetch environment variables from host environment list
 */

