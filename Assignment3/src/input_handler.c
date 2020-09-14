/*
//
// Created by Pulak Malhotra on 14/09/20.
//

#include "input_handler.h"
#include "headers.h"
#include "headers.h"
#include "pinfo.h"
#include "process_maker.h"
#include "util.h"
#include "ls.h"
#include <signal.h>
#include "history_handler.h"
#include "zombie_killer.h"
#include "nightswatch.h"


// pwd handler
void pwd_handler() {
    printf("%s\n", currDir);
}


// echo handler
void echo_handler(char *tokens[], int num) {
    for (int i = 1; i < num; i++) {
        printf("%s ", tokens[i]);
    }
    printf("\n");
}
void run_command(char **tokens, int num_tokens, int bg) {
    if (strcmp(tokens[0], "cd") == 0) {
        if (num_tokens == 1) {
            tokens[1] = malloc(4);
            strcpy(tokens[1], "~");
        }
        cd_handler(tokens);
    } else if (strcmp(tokens[0], "pwd") == 0) {
        pwd_handler();
    } else if (strcmp(tokens[0], "ls") == 0) {
        ls_handler(tokens, num_tokens, currDir, homeDir);
    } else if (strcmp(tokens[0], "echo") == 0) {
        echo_handler(tokens, num_tokens);
    } else if (strcmp(tokens[0], "exit") == 0) {
        killbg();
        printf("cya\n");
        _exit(0);

    } else if (strcmp(tokens[0], "clear") == 0) {
        clearScreen();
    } else if (strcmp(tokens[0], "pinfo") == 0) {
        if (num_tokens == 1) {
            tokens[1] = malloc(10);
            sprintf(tokens[1], "%d", getpid());
        }
        pinfo_handler(tokens);
    } else if (strcmp(tokens[0], "history") == 0) {
        if (num_tokens == 1) {
            show_history(10);
        } else {
            */
/*if (tokens[1][0] != '-') {
                printf("Second arg must be a flag\n");
                return;
            }
            tokens[1]++;*//*

            if (strtol(tokens[1], NULL, 10) <= 0 || strtol(tokens[1], NULL, 10) > 20) {
                printf("history <int n> \n n > 0 && n <= 20\n");
                return;
            }
            show_history(atoi(tokens[1]));
        }
    } else if (strcmp(tokens[0], "nightswatch") == 0) {
        nightswatch_handler(tokens, num_tokens);
    } else
        make_process(tokens, num_tokens, bg);
}

// note
// redirection are parsed left to right
// the positon of them doesnt matter
// <fileA means stdin of this command is from fileA
// >fileB means stdout of this command is to fileB
// the file is the "word" after the symbol
// none of these are arguments
// cat file means the C code for cat opens the file
// cat < file means that file is now the stdin and cat reads from its stdin
// >fileA >fileB will first open fileA as stdout (open as overwriting), then as it parses ">fileB" it will close fileA (thus overwriting it as empty) and then link fileB to the stdout
// > opens in overwrite mode and >> opens in append mode
// pipe uses pipe syscall to get the fds and sets stdin/stdout based on that
int tokenize(const char *token, char *string, char *tokens[100]) {
    int len_tok = strlen(token);
    int len_str = strlen(string);
    int count = 0;
    int start = 0;
    for (int i = 0; i < len_str - len_tok;) {
        bool found = true;
        for (int j = 0; j < len_tok; j++) {
            if (token[j] != string[i + j]) {
                found = false;
                break;
            }
        }
        if (found) {
            // last char has just been matched
            for (int k = i; k < i + len_tok; k++)
                string[k] = '\0';
            tokens[count] = malloc(size_buff);
            strcpy(tokens[count], string + start);
            count++;
            tokens[count] = malloc(size_buff);
            strcpy(tokens[count], token);
            count++;
            start = i + len_tok;
            i += len_tok;
        } else {
            i += 1;
        }
    }
    tokens[count] = malloc(size_buff);
    strcpy(tokens[count], string + start);
    count++;

*/
/*
    for (int i = 0; i < count; i++) {
        printf("%d--%s--\n", i, tokens[i]);
    }*//*

    return count;
}


void changeInput(char *token, char *file) {
    if (strcmp(token, ">") == 0) {
        // change stdout
        int new_fd;
        if ((new_fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) {
            perror("cannot redirect output");
        } else {
            close(STDOUT_FILENO);
            dup(new_fd);
            //printf("redirecting %d to another file", t);
        }
    } else if (strcmp(token, ">>") == 0) {
        // change stdout
        int new_fd;
        if ((new_fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644)) == -1) {
            perror("cannot redirect output");
        } else {
            close(STDOUT_FILENO);
            dup(new_fd);
            //printf("redirecting %d to another file", t);
        }
    } else if (strcmp(token, "<") == 0) {
        // change in
        int new_fd;
        //printf("redirecting stdin to another file");
        if ((new_fd = open(file, O_RDONLY)) == -1) {
            perror("cannot redirect input");
        } else {
            close(STDIN_FILENO);
            dup(new_fd);
            //printf("redirecting %d to another file", t);
        }
    }

}

void fixInput(int in, int out) {
    //ftruncate(STDOUT_FILENO, lseek(STDOUT_FILENO, 0, SEEK_CUR));
    close(STDOUT_FILENO);
    dup(out);
    close(STDIN_FILENO);
    dup(in);
}

void processInput(char *input, int bg) {
    char *tokens[1000];
    int num_tokens = 0;
    tokens[0] = strtok(input, " \t\n");
    while (tokens[num_tokens] != NULL) {
        tokens[++num_tokens] = strtok(NULL, " \t");
    }
    if (num_tokens == 0) {
        return;
    }
    char tokens_append[100][1000];
    int n = 0;
    for (int i = 0; i < num_tokens; i++) {
        char *new_tokens[100];
        int c = tokenize(">>", tokens[i], new_tokens);
        for (int j = 0; j < c; j++) {
            if (strcmp(new_tokens[j], "") != 0)
                strcpy(tokens_append[n++], new_tokens[j]);
            free(new_tokens[j]);
        }
    }
    */
/*   for (int i = 0; i < n; i++) {
           printf("--%s--\n", tokens_append[i]);
       }*//*


    // >
    char tokens_append_out[100][1000];
    num_tokens = n;
    n = 0;
    for (int i = 0; i < num_tokens; i++) {
        //printf("-token : %s-\n", tokens_append[i]);
        char *new_tokens[100];
        if (strcmp(tokens_append[i], ">>") == 0) {
            strcpy(tokens_append_out[n++], tokens_append[i]);
            continue;
        }
        int c = tokenize(">", tokens_append[i], new_tokens);
        for (int j = 0; j < c; j++) {
            if (strcmp(new_tokens[j], "") != 0) {
                strcpy(tokens_append_out[n++], new_tokens[j]);
            }
            free(new_tokens[j]);

        }
    }
*/
/*    for (int i = 0; i < n; i++) {
        printf("---%s---\n", tokens_append_out[i]);
    }*//*


    // <
    char tokens_final[100][1000];
    num_tokens = n;
    n = 0;
    for (int i = 0; i < num_tokens; i++) {
        //printf("-token : %s-\n", tokens_append_out[i]);
        char *new_tokens[100];
        int c = tokenize("<", tokens_append_out[i], new_tokens);
        for (int j = 0; j < c; j++) {
            if (strcmp(new_tokens[j], "") != 0)
                strcpy(tokens_final[n++], new_tokens[j]);
            free(new_tokens[j]);
        }

    }
    */
/*    for (int i = 0; i < n; i++) {
            printf("----%s----\n", tokens_final[i]);
        }*//*


    //now we need to do the redirection
    int backup_stdout = dup(STDOUT_FILENO);
    int backup_stdin = dup(STDIN_FILENO);
    char *command_tokens[1000];
    int num_word_command = 0;
    for (int i = 0; i < n; i++) {
        char *word = tokens_final[i];
        if (strcmp(word, ">") == 0 || strcmp(word, ">>") == 0 || strcmp(word, "<") == 0) {
            if (i + 1 == n || tokens_final[i + 1] == NULL) {
                printf("unexpected token after %s \n", word);
                return;
            }
            changeInput(word, tokens_final[i + 1]);
            i++;
        } else {
            command_tokens[num_word_command] = malloc(size_buff);
            strcpy(command_tokens[num_word_command], tokens_final[i]);
            num_word_command++;
        }
    }
    run_command(command_tokens, num_word_command, bg);
    for (int i = 0; i < num_word_command; i++)
        free(command_tokens[i]);
    fixInput(backup_stdin, backup_stdout);
    close(backup_stdout);
    close(backup_stdin);


}*/
