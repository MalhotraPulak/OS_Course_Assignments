#include "headers.h"
#include "pinfo.h"
#include "process_maker.h"
#include "util.h"
#include "ls.h"
#include <signal.h>
#include "history_handler.h"
#include "zombie_killer.h"
#include "nightswatch.h"


char *getShellName();

char *shellName;
char showDir[size_buff];
char homeDir[size_buff]; // has / in the end
char currDir[size_buff]; // has / in the end

void updateShowDir() {
    int homeDirLen = (int) strlen(homeDir);
    if (strlen(currDir) < homeDirLen) {
        strcpy(showDir, currDir);
        if (showDir[strlen(showDir) - 1] == '/') {
            showDir[strlen(showDir) - 1] = '\0';
        }
        return;
    }
    for (int i = 0; i < homeDirLen; i++) {
        if (homeDir[i] != currDir[i]) {
            strcpy(showDir, currDir);
            return;
        }
    }
    strcpy(showDir, "~/");
    strcat(showDir, currDir + homeDirLen);
    if (showDir[strlen(showDir) - 1] == '/') {
        showDir[strlen(showDir) - 1] = '\0';
    }

}

char *getShellName() {
    const int maxLen = 2000;
    char login[maxLen];
    char hostname[maxLen];
    // getlogin_r
    if (getlogin_r(login, maxLen) == 0) {
    } else {
        perror("Cant get login name");
    }
    // sysname
    struct utsname sysName;
    if (uname(&sysName) == 0) {
    } else {
        perror("Cant get system name");
    }
    // get hostname
    if (gethostname(hostname, maxLen) == 0) {
    } else {
        perror("Cant get hostname");
    }
    char *final_name = (char *) malloc(500);
    strcpy(final_name, login);
    strcat(final_name, "@");
    strcat(final_name, hostname);
    strcat(final_name, ":");
    return final_name;
}

// handles cd
void cd_handler(char *token[]) {
    char cd_location[size_buff];
    strcpy(cd_location, token[1]);
    char new_address[size_buff];
    get_raw_address(new_address, cd_location, currDir, homeDir);
    struct stat stats_dir;
    if (stat(new_address, &stats_dir) == 0 && (S_IFDIR & stats_dir.st_mode)) {
        if (chdir(new_address) == -1) {
            printf("cd : directory does not exist\n");
        }
        if (getcwd(currDir, size_buff) == NULL) {
            printf("cd : getcwd failed\n");
        }
        strcat(currDir, "/");
        updateShowDir();
    } else {
        printf("cd : directory does not exist: %s\n", token[1]);
    }

}

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


void run_command(char **tokens, int num_tokens, int bg, int *pipe, int prev_open) {
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
            /*if (tokens[1][0] != '-') {
                printf("Second arg must be a flag\n");
                return;
            }
            tokens[1]++;*/
            if (strtol(tokens[1], NULL, 10) <= 0 || strtol(tokens[1], NULL, 10) > 20) {
                printf("history <int n> \n n > 0 && n <= 20\n");
                return;
            }
            show_history(atoi(tokens[1]));
        }
    } else if (strcmp(tokens[0], "nightswatch") == 0) {
        nightswatch_handler(tokens, num_tokens);
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
    dup2(in, 0);
    close(in);
    dup2(out, 1);
    close(out);
}

void processInput(char *input, int bg, int *pipe, int prev_open) {
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
    /*   for (int i = 0; i < n; i++) {
           printf("--%s--\n", tokens_append[i]);
       }*/

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
/*    for (int i = 0; i < n; i++) {
        printf("---%s---\n", tokens_append_out[i]);
    }*/

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
    /*    for (int i = 0; i < n; i++) {
            printf("----%s----\n", tokens_final[i]);
        }*/

    //now we need to do the redirection
    //int backup_stdout = dup(STDOUT_FILENO);
    //int backup_stdin = dup(STDIN_FILENO);
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
    run_command(command_tokens, num_word_command, bg, pipe, prev_open);
    for (int i = 0; i < num_word_command; i++)
        free(command_tokens[i]);
    //backup making before function call so it doesnt interfere with piping
    //fixInput(backup_stdin, backup_stdout);
    //close(backup_stdout);
    //close(backup_stdin);


}


void pipechecker(char *cmd, int bg) {
    int pipee = 0;
    for (int i = 0; i < strlen(cmd); i++)
        if (cmd[i] == '|')
            pipee++;
    if (pipee == 0) {
        processInput(cmd, bg, NULL, -1);
        return;
    } else if (cmd[0] == '|' || cmd[strlen(cmd) - 1] == '|') {
        printf("Pipe does not have both ends \n");
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
        processInput(commands[i], bg, pipes, prev_open);
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
    processInput(commands[n - 1], bg, NULL, prev_open);
    dup2(in, 0);
    close(in);
}

// separates commands by ;
void get_commands(char *line) {
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
        //processInput(commands[j], bg);
        int backup_stdout = dup(STDOUT_FILENO);
        int backup_stdin = dup(STDIN_FILENO);
        pipechecker(commands[j], bg);
        fixInput(backup_stdin, backup_stdout);

    }
    // everything gets automatically deallocated as strtok is in place
}


void rip_child(int signum) {
    if (signum == SIGCHLD)
        zombie_process_check();
}

void exit_2(int signum) {
    if (signum == SIGINT) {
        killbg();
        _exit(0);
    }
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
    signal(SIGCHLD, rip_child);
    signal(SIGINT, exit_2);
    strcpy(currDir, homeDir);
    updateShowDir();
    while (1) {
        printCyan();
        printf("%s", shellName);
        printGreen();
        printf("%s ", showDir);
        printYellow();
        printf("$ ");
        resetColor();
        char *line = malloc(size_buff);
        char *line2 = line;
        fgets(line, size_buff, stdin);
        size_t ln = strlen(line) - 1;
        if (*line && line[ln] == '\n')
            line[ln] = '\0';
        line = trim_whitespace(line);
        add_history(line);
        get_commands(line);
        free(line2);
    }

}

/*
 getenv setenv - get unset and fetch environment variables from host environment list
 signal - know already
 dup - duplicate and existing file descriptor
 dup2 - also used to duplicate
 strtok fork setpgid wait waitpid getpid kill execvp - know already
 getchar - reads a character from stdin

other -
 pipe -
 */

