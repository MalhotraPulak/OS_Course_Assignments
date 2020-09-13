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

// separates a command by spaces and sends to appropriate handler
void processInput(char *input, int bg) {
    char *tokens[1000];
    int num_tokens = 0;
    tokens[0] = strtok(input, " \t\n");
    while (tokens[num_tokens] != NULL) {
        tokens[++num_tokens] = strtok(NULL, " \t");
    }
    if (num_tokens == 0)
        return;
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
        make_process(tokens, num_tokens, bg);
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
        processInput(commands[j], bg);

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

char *trim_whitespace(char *line) {
    // leading
    int t = 0;
    for (int i = 0; i < strlen(line); i++) {
        if (line[i] == ' ' || line[i] == '\t' || line[i] == '\n') {
            t++;
        } else {
            break;
        }
    }
    //printf("%d\n", t);
    for (int i = 0; i < t; i++) {
        line++;
    }
    //printf("%s\n", line);
    // trailing
    for (int i = strlen(line) - 1; i >= 0; i--) {
        if (line[i] == ' ' || line[i] == '\t' || line[i] == '\n') {
            line[i] = '\0';
        } else {
            break;
        }
    }
    //printf("%s", line);
    return line;
}


int main() {
    clearScreen();
    welcomeMessage();
    shellName = getShellName();
    //atexit(killbg);
    if (getcwd(homeDir, size_buff) == NULL) {
        perror("getcwd failed");
    }
    if (homeDir[strlen(homeDir) - 1] != '/') {
        strcat(homeDir, "/");
    }
    //setSignalHandler();
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


//TODO process maker support for strings

// parsing command redo  done
// newline after nightswatch error message done
// kill bg processes before exit done
// nightswatch and history syntax done
// nightswatch done
// ls done


