//
// Created by Pulak Malhotra on 15/09/20.
//

#include "parser.h"
#include "headers.h"
#include "util.h"
#include "ls.h"
#include "pinfo.h"
#include "zombie_killer.h"
#include "process_maker.h"
#include <signal.h>
#include "history_handler.h"
#include "nightswatch.h"
#include "env_var.h"

// runs the commands / built ins
void run_command(char **tokens, int num_tokens, int bg, int *pipe, int prev_open) {
    char *first = tokens[0];
    if (strcmp(first, "cd") == 0) {
        if (num_tokens == 1) {
            tokens[1] = malloc(4);
            strcpy(tokens[1], "~");
        }
        cd_handler(tokens);
    } else if (strcmp(first, "pwd") == 0) {
        pwd_handler();
    } else if (strcmp(first, "ls") == 0) {
        ls_handler(tokens, num_tokens, currDir, homeDir);
    } else if (strcmp(first, "echo") == 0) {
        echo_handler(tokens, num_tokens);
    } else if (strcmp(first, "quit") == 0) {
        killbg();
        printf("cya\n");
        _exit(0);

    } else if (strcmp(first, "clear") == 0) {
        clearScreen();
    } else if (strcmp(first, "pinfo") == 0) {
        if (num_tokens == 1) {
            tokens[1] = malloc(10);
            sprintf(tokens[1], "%d", getpid());
        }
        pinfo_handler(tokens);
    } else if (strcmp(first, "history") == 0) {
        if (num_tokens == 1) {
            show_history(10);
        } else {
            /*if (tokens[1][0] != '-') {
                printf("Second arg must be a flag\n");
                return;
            }
            tokens[1]++;*/
            if (strtol(tokens[1], NULL, 10) <= 0 || strtol(tokens[1], NULL, 10) > 20) {
                fprintf(stderr, "history <int n> \n n > 0 && n <= 20\n");
                return;
            }
            show_history(atoi(tokens[1]));
        }
    } else if (strcmp(first, "nightswatch") == 0) {
        nightswatch_handler(tokens, num_tokens);
    } else if (strcmp(first, "getenv") == 0) {
        getenv_handler(tokens, num_tokens);
    } else if (strcmp(first, "setenv") == 0) {
        setenv_handler(tokens, num_tokens);
    } else if (strcmp(first, "unsetenv") == 0) {
        unsetenv_handler(tokens, num_tokens);
    } else if (strcmp(first, "jobs") == 0) {
        job_printer();
    } else if (strcmp(first, "kjob") == 0) {
        kjob_handler(tokens, num_tokens);
    }else if (strcmp(first, "overkill") == 0) {
        overkill_handler(tokens, num_tokens);
    }else if (strcmp(first, "fg") == 0) {
        fg_handler(tokens, num_tokens);
    }else if (strcmp(first, "bg") == 0) {
        bg_handler(tokens, num_tokens);
    } else
        make_process(tokens, num_tokens, bg, pipe, prev_open);
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

// parses > < >>
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

/*
    for (int i = 0; i < count; i++) {
        printf("%d--%s--\n", i, tokens[i]);
    }*/
    return count;
}

// redirection changes
void changeInput(char *token, char *file) {
    if (strcmp(token, ">") == 0) {
        // change stdout
        int new_fd;
        if ((new_fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) {
            perror("cannot redirect output");
        } else {
            close(STDOUT_FILENO);
            dup(new_fd);
            close(new_fd);
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
            close(new_fd);
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
            close(new_fd);
            //printf("redirecting %d to another file", t);
        }
    }

}

void fixInput(int in, int out) {
    dup2(in, 0);
    close(in);
    dup2(out, 1);
    close(out);
}

// takes care of redirection and spaces
void redirectionHandler(char *input, int bg, int *pipe, int prev_open) {
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
    //now we need to do the redirection
    //int backup_stdout = dup(STDOUT_FILENO);
    //int backup_stdin = dup(STDIN_FILENO);
    char *command_tokens[1000];
    int num_word_command = 0;
    for (int i = 0; i < n; i++) {
        char *word = tokens_final[i];
        if (strcmp(word, ">") == 0 || strcmp(word, ">>") == 0 || strcmp(word, "<") == 0) {
            if (i + 1 == n || tokens_final[i + 1] == NULL) {
                fprintf(stderr, "unexpected token after %s \n", word);
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
    run_command(command_tokens, num_word_command, bg, pipe, prev_open);
    for (int i = 0; i < num_word_command; i++)
        free(command_tokens[i]);
    //close(STDOUT_FILENO);
    //close(STDIN_FILENO);
    //backup making before function call so it doesnt interfere with piping
    //fixInput(backup_stdin, backup_stdout);
    //close(backup_stdout);
    //close(backup_stdin);


}

// checks for pipes
void pipeChecker(char *cmd, int bg) {
    int pipee = 0;
    for (int i = 0; i < strlen(cmd); i++)
        if (cmd[i] == '|')
            pipee++;
    if (pipee == 0) {
        redirectionHandler(cmd, bg, NULL, -1);
        return;
    } else if (cmd[0] == '|' || cmd[strlen(cmd) - 1] == '|') {
        fprintf(stderr, "Pipe does not have both ends \n");
        return;
    }
    char *commands[1000];
    int n = 0;
    char *t = strtok(cmd, "|");
    while (t != NULL) {
        commands[n] = malloc(size_buff);
        strcpy(commands[n], t);
        t = strtok(NULL, "|");
        //printf("-%s-\n", commands[n]);
        n++;
    }


    int out = dup(1);
    int in = dup(0);
    int prev_open = -1;
    for (int i = 0; i < n - 1; i++) {
        int pipes[2];
        if (pipe(pipes) == -1) {
            //perror("cannot open pipe");
            return;
        }
        if (prev_open != -1) {
            dup2(prev_open, 0);
            //perror("connected prev input pipe");
            close(prev_open);
            //perror("closed prev input pipe");
        }
        dup2(pipes[1], 1);
        //perror("connected new output pipe");
        close(pipes[1]);
        //perror("connected output pipe old fd");
        redirectionHandler(commands[i], bg, pipes, prev_open);
        prev_open = pipes[0];
        free(commands[i]);
    }
    dup2(out, 1);
    //perror("connected output to stdout");
    close(out);
    if (prev_open != -1) {
        dup2(prev_open, 0);
        //perror("connected prev input pipe");
        close(prev_open);
    }
    redirectionHandler(commands[n - 1], bg, NULL, prev_open);
    dup2(in, 0);
    close(in);
}

// separates commands by ;
void getIndividualCommands(char *line) {
    char *command;
    char line2[size_buff], line3[size_buff];
    strcpy(line2, line);
    strcpy(line3, line);
    command = strtok(line, ";&");
    int c = 0;
    while (command != NULL) {
        c++;
        command = strtok(NULL, ";&");
    }
    char *commands[c + 1];
    int i = 0;
    if (c <= 0) return;
    char *beg = line2;
    commands[0] = strtok(line2, ";&");

    while (commands[i] != NULL && strcmp(commands[i], "") != 0) {
        //printf("%c\n", line3[strlen(commands[i]) + line2 - beg]);
        //printf("%c\n", line3[strlen(commands[i]) + (commands[i] - beg)]);
        i++;
        commands[i] = strtok(NULL, ";&");

    }
    for (int j = 0; j < c; j++) {
        bool bg = false;
        if (line3[strlen(commands[j]) + (commands[j] - beg)] == '&') {
            bg = true;
        }
        //redirectionHandler(commands[j], bg);
        int backup_stdout = dup(STDOUT_FILENO);
        int backup_stdin = dup(STDIN_FILENO);
        pipeChecker(commands[j], bg);
        fixInput(backup_stdin, backup_stdout);

    }
    // everything gets automatically deallocated as strtok is in place
}

// input -> get commands (splits by & and ;) -> pipeChecker (handles piping) -> redirectionHandler (handles redirection)
// -> (uses tokenizer) -> run command