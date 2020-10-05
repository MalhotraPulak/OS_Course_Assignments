//
// Created by Pulak Malhotra on 15/09/20.
//

#include "env_var.h"
#include "headers.h"
#include "util.h"

void getenv_handler(char **tokens, int num) {
    if (num != 2) {
        fprintf(stderr, "getenv: invalid format : getenv <varname>\n");
        exit_code = 1;
        return;
    }
    char *t = getenv(tokens[1]);
    if (t == NULL) {
        fprintf(stderr, "getenv: %s does not exist\n", tokens[1]);
        exit_code = 1;
        return;
    }
    fprintf(stderr, "%s \n", t);
}

void setenv_handler(char **tokens, int num) {
    if (num == 1 || num >= 4) {
        fprintf(stderr, "setenv: invalid format : setenv <varname> <value>\n");
        exit_code = 1;
        return;
    }
    if (num == 2) {
        tokens[2] = strdup("");
    }
    if (setenv(tokens[1], tokens[2], 1) == 1) {
        perror("setenv: setenv failed\n");
        exit_code = 1;
    } else {
        printf("setenv: %s set successfully\n", tokens[1]);

    }
}

void unsetenv_handler(char **tokens, int num) {
    if (num == 1 || num >= 3) {
        fprintf(stderr, "unsetenv: invalid format : unsetenv <varname> \n");
        exit_code = 1;
        return;
    }
    if (unsetenv(tokens[1]) == -1) {
        perror("unsetenv failed\n");
        exit_code = 1;
    } else
        printf("unsetenv: %s unset successfully\n", tokens[1]);
}