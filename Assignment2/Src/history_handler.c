//
// Created by Pulak Malhotra on 01/09/20.
//
#include "headers.h"
#include "history_handler.h"
#include "util.h"
// read lines
// if read line is not equal to last read then
// get last 20 lines if possible
// write the last 20 lines

#define history_file "/tmp/.shell_history"


void add_history(char tokens[]) {
    if (strcmp(tokens, "") == 0) return;;
    FILE *f;
    f = fopen(history_file, "a");
    fclose(f);
    f = fopen(history_file, "r");
    char *lines[100];
    int n = 0;
    //perror("ff");
    if (f != NULL) {
        lines[n] = malloc(size_buff);
        size_t s = size_buff;
        while (getline(&lines[n], &s, f) != -1) {
            if (strcmp(lines[n], "\n") != 0)
                lines[++n] = malloc(size_buff);
        }
    }
    fclose(f);
    f = fopen(history_file, "w");
    int  i;
    for ( i = max(0, n - 20); i < n; i++) {
        fprintf(f, "%s", lines[i]);
        //printf("%s", lines[i]);
    }

    char new[size_buff];
    sprintf(new, "%s\n", tokens);
    if(strcmp(new, lines[i - 1]) == 0){

    } else {
        fprintf(f, "%s", new);
    }
    fclose(f);
}


void show_history(int no) {
    FILE *f;
    f = fopen(history_file, "a");
    fclose(f);
    f = fopen(history_file, "r");
    char *lines[100];
    int n = 0;
    if (f != NULL) {
        lines[n] = malloc(size_buff);
        size_t s = size_buff;
        while (getline(&lines[n], &s, f) != -1) {
            if (strcmp(lines[n], "\n") != 0)
                lines[++n] = malloc(size_buff);
        }
    }
    fclose(f);
    for (int i = max(0, n - no); i < n; i++) {
        printf("%s\n", lines[i]);
    }
}