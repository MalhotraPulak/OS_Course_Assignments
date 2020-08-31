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
    char *argv[num + 1];
    for (int i = 0; i < num; i++) {
        argv[i] = strdup(tokens[i]);
    }
    argv[num] = NULL;
    int rc = fork();
    if (rc < 0) {
        printf("creating child process failed\n");
    } else if (rc == 0) {
        //setpgid(0, 0);
        printf("in child process pid = %d\n", (int) getpid());
        if (execvp(cmd, argv) == -1) {
            perror("error in execvp");
            printf("%s: command not found\n", cmd);
        }
    } else {
        //int rc_wait = waitpid(rc, NULL, 0);
        int rc_wait = wait(NULL);
        printf("Control is back rc_wait = %d, current pid = %d  \n", rc_wait, (int) getpid());
    }

}