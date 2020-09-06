//
// Created by Pulak Malhotra on 31/08/20.
//
#include "headers.h"
#include "process_maker.h"

// ***** process handling *******
// signal - send signal to a process
// waitpid - makes the current process wait until the other is terminated
// getpid - get pid of the calling process
// kill - can be used to send any signal to process or process group
//


void make_process(char *tokens[], int num) {
    char *cmd = strdup(tokens[0]);
    int bg = 0;
    if (strcmp(tokens[num - 1], "&") == 0) {
        bg = 1;
        num -= 1;
    }
    char *argv[num + 1];
    for (int i = 0; i < num; i++) {
        argv[i] = strdup(tokens[i]);
    }
    argv[num] = NULL;
    int rc = fork();
    if (rc < 0) {
        perror("creating child process failed\n");
    } else if (rc == 0) {
        // if bg the child process is now in a new session with no terminal
        if (bg) {

            setpgid(0, 0);

        }
        if (execvp(cmd, argv) == -1) {
            printf("invalid command\n");
            exit(1);
        }
    } else if (rc > 0) {
        if (!bg) {
            //printf("gonna wait \n");
            waitpid(rc, NULL, 0);

        } else {
            printf("child with pid [%d] sent to background\n", rc);
        }
    }

}