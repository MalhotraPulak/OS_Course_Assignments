//
// Created by Pulak Malhotra on 31/08/20.
//
#include "headers.h"
#include "pinfo.h"

// parse the file in /proc/pid/stat, /proc/pid/status (easier) and /proc/pid/exe


void pinfo_handler(char *tokens[]) {
    //printf("%s", tokens[i]);
    char add[size_buff];
    char exe[size_buff];
    sprintf(add, "/proc/%s/stat", tokens[1]);
    FILE *ptr = fopen(add, "r");
    if (ptr == NULL) {
        printf("Cannot access the process\n");
        return;
    }
    int pid;
    char exec_name[1000];
    char st;
    fscanf(ptr, "%d ", &pid);
    fscanf(ptr, "%s ", exec_name);
    fscanf(ptr, "%c ", &st);
    sprintf(add, "/proc/%s/exe", tokens[1]);
    int t = readlink(add, exe, size_buff);
    if (t != -1) {
        exe[t] = '\0';
    } else {
        printf("Cannot access the process\n");
        return;
    }
    sprintf(add, "/proc/%s/status", tokens[1]);
    ptr = fopen(add, "r");
    if (ptr == NULL) {
        printf("Cannot access the process\n");
        return;
    }
    char word[size_buff];
    while (fscanf(ptr, " %499s", word) == 1) {
        if (strcmp(word, "VmSize:") == 0) {
            fscanf(ptr, " %499s", word);
            break;
        }
    }
    printf("pid -- %d\n", pid);
    printf("Process Status -- %c\n", st);
    printf("memory -- %s\n", word);
    printf("Executable Path -- %s\n", exe);
    printf("\n");
}
