//
// Created by Pulak Malhotra on 31/08/20.
//
#include "headers.h"
#include "process_maker.h"
#include <signal.h>
#include <errno.h>

// ***** process handling *******
// signal - send signal to a process
// waitpid - makes the current process wait until the other is terminated
// getpid - get pid of the calling process
// kill - can be used to send any signal to process or process group
//
//
int stack[1000];
int top = 1; // the next position to insert the process
void waitForIt();

int pid_to_job(int pid) {
    for (int i = 1; i < top; i++) {
        if (stack[i] == pid) {
            return i;
        }
    }
    return -1;
}


int job_to_pid(int job) {
    if (job <= 0 || job >= 1000 || stack[job] == 0) {
        return -1;
    } else {
        return stack[job];
    }

}

void kjob_handler(char *tokens[], int n) {
    if (n != 3) {
        fprintf(stderr, "kjob <job number> <signal number>\n");
        return;
    }
    int t = (int) strtol(tokens[1], NULL, 10);
    int pid = job_to_pid(t);
    int signal = (int) strtol(tokens[2], NULL, 10);
    if (pid <= 0) {
        fprintf(stderr, "Job does not exist \n");
        return;
    }
    if (signal < 0) {
        fprintf(stderr, "invalid signal \n");
        return;
    }
    if (kill(pid, signal) == -1)
        perror("Signal Failed");

}

void remove_child(int pid) {
    int job = pid_to_job(pid);
    stack[job] = 0;
    while (top > 1 && stack[top - 1] == 0) {
        top -= 1;
    }
}


void print_job_data(int pid) {
    int job = pid_to_job(pid);
    char location[size_buff];
    sprintf(location, "/proc/%d/cmdline", pid);
    FILE *fg = fopen(location, "r");
    char cmd_name[size_buff];
    if (fg != NULL)
        fgets(cmd_name, size_buff, fg);
    sprintf(location, "/proc/%d/stat", pid);
    FILE *f = fopen(location, "r");
    if (f == NULL) {
        fprintf(stderr, "%d process not found\n", pid);
        return;
    }
    char state = 0;
    int pd;
    char new_name[size_buff];
    fscanf(f, " %d ", &pd);
    fscanf(f, " %s ", new_name);
    fscanf(f, " %c ", &state);
    char statee[100];
    if (state == 'R')
        strcpy(statee, "Running");
    else if (state == 'T')
        strcpy(statee, "Stopped");
    else if (state == 'Z')
        strcpy(statee, "Zombie");
    else if (state == 'S')
        strcpy(statee, "Interruptible sleep");
    fclose(f);
    if (fg != NULL)
        fclose(fg);
    else
        strcpy(cmd_name, new_name);
    printf("%c[%d] %s %s [%d]\n", state, job, statee, cmd_name, pid);
}

void job_printer() {
    for (int i = 1; i < top; i++) {
        if (stack[i] != 0) {
            print_job_data(stack[i]);
        }
    }
}

int add_child(int pid) {
    if (top == 1) {
        for (int i = 0; i < 1000; i++)
            stack[i] = 0;
    }
    if (top <= 0) {
        perror("Error adding child process to stack");
        return -1;
    } else {
        stack[top] = pid;
        top++;
        return top - 1;
    }
}


void make_process(char *tokens[], int num, int bg, int *pipe, int prev_open) {
    char *cmd = strdup(tokens[0]);
    char *argv[num + 1];
    for (int i = 0; i < num; i++) {
        argv[i] = strdup(tokens[i]);
    }
    argv[num] = NULL;
    int rc = fork();
    if (rc < 0) {
        perror("creating child process failed\n");
        return;
    }

    add_child(rc);
    if (rc == 0) {
        // if bg the child process is now in a new session with no terminal
        if (pipe != NULL) {
            close(pipe[1]);// required
            //perror("pipe1 close in child");
            close(pipe[0]); // should close the input (Not req ig)
            //perror("pipe0 close in child");
        }
        if (bg) {
            setpgid(0, 0);
        }
        if (execvp(cmd, argv) == -1) {
            fprintf(stderr, "invalid command : %s\n", cmd);
            exit(1);
        }
    } else if (rc > 0) {
        if (pipe != NULL) {
            close(pipe[1]);
            if (prev_open != -1)
                close(prev_open);
            //perror("pipe1 closed in parent");

        }
        if (!bg) {
            //waitForIt();
            int status;
            waitpid(rc, &status, WUNTRACED);
            if (WIFEXITED(status)) {
                remove_child(rc);
            }
            /*printf("exited:    %d status: %d\n"
                   "signalled: %d signal: %d\n"
                   "stopped:   %d signal: %d\n"
                   "continued: %d\n",
                   WIFEXITED(status),
                   WEXITSTATUS(status),
                   WIFSIGNALED(status),
                   WTERMSIG(status),
                   WIFSTOPPED(status),
                   WSTOPSIG(status),
                   WIFCONTINUED(status));*/


        } else {
            printf("child with pid [%d] sent to background\n", rc);
        }
    }

}

/*void waitForIt() {

}*/

void bg_handler(char **tokens, int n) {
    if (n != 2) {
        fprintf(stderr, "bg <job number>\n");
        return;
    }
    int t = (int) strtol(tokens[1], NULL, 10);
    int pid = job_to_pid(t);
    if (pid <= 0) {
        fprintf(stderr, "Job does not exist \n");
        return;
    }



    kill(pid, SIGCONT);

}

void fg_handler(char **tokens, int n) {
    if (n != 2) {
        fprintf(stderr, "fg <job number>\n");
        return;
    }
    int t = (int) strtol(tokens[1], NULL, 10);
    int pid = job_to_pid(t);
    if (pid <= 0) {
        fprintf(stderr, "Job does not exist \n");
        return;
    }
    tcsetpgrp(STDOUT_FILENO, getpgid(pid));
    setpgid(getpid(), pid);
    kill(pid, SIGCONT);
    int status;
    waitpid(pid, &status, WUNTRACED);
     
    if (WIFEXITED(status)) {
        remove_child(pid);
    }
}

void overkill_handler(char **tokens, int n) {
    for (int i = 1; i < top; i++)
        if (stack[i] > 0) {
            int pid = stack[i];
            kill(pid, 9);
        }
}
