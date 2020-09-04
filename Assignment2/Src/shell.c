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

char *shell_name;
char show_dir[size_buff];
char home_dir[size_buff]; // has / in the end
char curr_dir[size_buff]; // has / in the end

void updateShowDir() {
    int homeDirLen = strlen(home_dir);
    if (strlen(curr_dir) < homeDirLen) {
        strcpy(show_dir, curr_dir);
        if (show_dir[strlen(show_dir) - 1] == '/') {
            show_dir[strlen(show_dir) - 1] = '\0';
        }
        return;
    }
    for (int i = 0; i < homeDirLen; i++) {
        if (home_dir[i] != curr_dir[i]) {
            strcpy(show_dir, curr_dir);
            return;
        }
    }
    strcpy(show_dir, "~/");
    strcat(show_dir, curr_dir + homeDirLen);
    if (show_dir[strlen(show_dir) - 1] == '/') {
        show_dir[strlen(show_dir) - 1] = '\0';
    }

}

char *getShellName() {
    const int max_len = 2000;
    char login[max_len];
    char hostname[max_len];
    // getlogin_r
    if (getlogin_r(login, max_len) == 0) {
    } else {
        perror("Cant get login name");
    }
    // sysname
    struct utsname sys_name;
    if (uname(&sys_name) == 0) {
    } else {
        perror("Cant get system name");
    }
    // get hostname
    if (gethostname(hostname, max_len) == 0) {
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
    get_raw_address(new_address, cd_location, curr_dir, home_dir);
    struct stat stats_dir;
    if (stat(new_address, &stats_dir) == 0 && (S_IFDIR & stats_dir.st_mode)) {
        if (chdir(new_address) == -1) {
            printf("cd : directory does not exist\n");
        }
        if (getcwd(curr_dir, size_buff) == NULL) {
            printf("cd : getcwd failed\n");
        }
        strcat(curr_dir, "/");
        updateShowDir();
    } else {
        printf("cd : directory does not exist: %s\n", token[1]);
    }

}

// pwd handler
void pwd_handler() {
    printf("%s\n", curr_dir);
}


// echo handler
void echo_handler(char *tokens[], int num) {
    for (int i = 1; i < num; i++) {
        printf("%s ", tokens[i]);
    }
    printf("\n");
}

// separates a command by spaces and sends to appropriate handler
void processInput(char *input) {
    char *tokens[1000];
    int num_tokens = 0;
    tokens[0] = strtok(input, " \t\n");
    while (tokens[num_tokens] != NULL) {
        tokens[++num_tokens] = strtok(NULL, " \t");
    }

    if (strcmp(tokens[0], "cd") == 0) {
        if (num_tokens == 1) {
            tokens[1] = malloc(size_buff);
            strcpy(tokens[1], "~");
        }
        cd_handler(tokens);
    } else if (strcmp(tokens[0], "pwd") == 0) {
        pwd_handler();
    } else if (strcmp(tokens[0], "ls") == 0) {
        ls_handler(tokens, num_tokens, curr_dir, home_dir);
    } else if (strcmp(tokens[0], "echo") == 0) {
        echo_handler(tokens, num_tokens);
    } else if (strcmp(tokens[0], "exit") == 0) {
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
            if(tokens[1][0] != '-'){
                printf("Second arg must be a flag\n");
                return;
            }
            tokens[1]++;
            if (atoi(tokens[1]) <= 0 || atoi(tokens[1]) > 15) {
                printf("n > 0 && n <= 15\n");
                return;
            }
            show_history(atoi(tokens[1]));
        }
    } else if (strcmp(tokens[0], "nightswatch") == 0) {
        nightswatch_handler(tokens, num_tokens);
    } else
        make_process(tokens, num_tokens);
}

// separates commands by ;
void get_commands(char *line) {
    char *command;
    char line2[size_buff];
    strcpy(line2, line);
    command = strtok(line, ";");
    int c = 0;
    while (command != NULL) {
        c++;
        command = strtok(NULL, ";");
    }
    char *commands[c + 1];
    int i = 0;
    if (c <= 0) return;
    commands[0] = strtok(line2, ";");
    while (commands[i] != NULL) {
        i++;
        commands[i] = strtok(NULL, ";");

    }
    for (int j = 0; j < c; j++) {

        processInput(commands[j]);
    }
    // everything gets automatically deallocated as strtok is in place
}


void rip_child(int signum) {
    if (signum == SIGCHLD)
        zombie_process_check();
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
    for (int i = strlen(line); i >= 0; i--) {
        if (line[i] == ' ' || line[i] == '\t' || line[i] == '\n') {
            line[i] = '\0';
        } else {
            break;
        }
    }
    return line;
}




int main() {
    clearScreen();
    shell_name = getShellName();
    if (getcwd(home_dir, size_buff) == NULL) {
        perror("getcwd failed");
    }
    if (home_dir[strlen(home_dir) - 1] != '/') {
        strcat(home_dir, "/");
    }
    //setSignalHandler();
    signal(SIGCHLD, rip_child);
    strcpy(curr_dir, home_dir);
    updateShowDir();
    while (1) {
        printBlue();
        printf("%s", shell_name);
        printGreen();
        printf("%s $ ", show_dir);
        resetColor();
        char *line = malloc(size_buff);
        fgets(line, size_buff, stdin);
        size_t ln = strlen(line) - 1;
        if (*line && line[ln] == '\n')
            line[ln] = '\0';
        line = trim_whitespace(line);
        add_history(line);
        get_commands(line);
        free(line);

    }

}



