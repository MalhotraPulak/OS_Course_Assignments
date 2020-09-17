//
// Created by Pulak Malhotra on 15/09/20.
//

#include "env_var.h"
#include "headers.h"

void getenv_handler(char **tokens, int num) {
    if (num != 2) {
       fprintf(stderr, "invalid format : getenv <varname>\n");
        return;
    }
    char *t = getenv(tokens[1]);
    if (t == NULL) {
       fprintf(stderr, "%s does not exist\n", tokens[1]);
        return;
    }
   fprintf(stderr, "%s \n", t);
}

void setenv_handler(char **tokens, int num) {
    if (num == 1 || num >= 4) {
       fprintf(stderr, "invalid format : setenv <varname> <value>\n");
        return;
    }
    if (num == 2) {
        tokens[2] = strdup("");
    }
    if (setenv(tokens[1], tokens[2], 1) == 1) {
        perror("setenv failed\n");
    } else {
        printf("%s set successfully\n", tokens[1]);

    }
}

void unsetenv_handler(char **tokens, int num) {
    if (num == 1 || num >= 3) {
       fprintf(stderr, "invalid format : unsetenv <varname> \n");
        return;
    }
    if(unsetenv(tokens[1]) == -1)
        perror("unsetenv failed\n");
    else
        printf("%s unset successfully\n", tokens[1]);
}