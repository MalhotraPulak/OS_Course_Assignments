#include "headers.h"
#include "pinfo.h"
#include "process_maker.h"
#include "util.h"
#include "ls.h"


char *getShellName();

char *shell_name;
char show_dir[size_buff]; // has / in the end since they are directories
char home_dir[size_buff]; // has / in the end
char curr_dir[size_buff]; // has / in the end

void updateShowDir() {
    //printf("curr dir %s home dir %s show dir %s \n\n", curr_dir, home_dir, show_dir);
    int homeDirLen = strlen(home_dir);
    if (strlen(curr_dir) < homeDirLen) {
        strcpy(show_dir, curr_dir);
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
    //printf("show dir %s\n", show_dir);
}

char *getShellName() {
    const int max_len = 2000;
    char login[max_len];
    char hostname[max_len];
    // getlogin_r
    if (getlogin_r(login, max_len) == 0) {
        // printf("%s\n", login);
    } else {
        perror("Cant get login name");
    }
    // sysname
    struct utsname sys_name;
    if (uname(&sys_name) == 0) {
        // printf("%s\n", sys_name.sysname);
    } else {
        perror("Cant get system name");
    }
    // gethostname
    if (gethostname(hostname, max_len) == 0) {
        // printf("%s\n", hostname);
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

void clearScreen() {
    printf("\e[1;1H\e[2J");
}

void printGreen() {
    printf("%s", "\x1B[32m");
}

void resetColor() {
    printf("%s", "\x1B[0m");
}


void cd_handler(char *token[]) {
    char cd_location[size_buff];
    strcpy(cd_location, token[1]);
    char new_address[size_buff];
    get_raw_address(new_address, cd_location, curr_dir, home_dir);
    struct stat stats_dir;
    if (stat(new_address, &stats_dir) == 0 && (S_IFDIR & stats_dir.st_mode)) {
        //printf("new address is %s\n", new_address);
        // changing the currrent working directory as required
        if (chdir(new_address) == -1) {
            printf("cd : directory does not exist\n");
        }
        //cleanupAddress(new_address);
        //strcpy(curr_dir, new_address);
        if (getcwd(curr_dir, size_buff) == NULL) {
            printf("cd : getcwd failed\n");
        }
        strcat(curr_dir, "/");
        updateShowDir();
    } else {
        printf("directory does not exist: %s\n", new_address);
    }

}

void pwd_handler() {
    printf("%s\n", curr_dir);
}


// echo "4324"
void echo_handler(char *tokens[], int num) {
    for (int i = 1; i < num; i++) {
        printf("%s ", tokens[i]);
    }
    printf("\n");
}

void processInput(char *input) {
    //printf("%s\n", input);
    char *tokens[100];
    int num_tokens = 0;
    tokens[0] = strtok(input, " \t");
    while (tokens[num_tokens] != NULL) {
        tokens[++num_tokens] = strtok(NULL, " \t");
    }
    /* for (int i = 0; i < num_tokens; i++) {
         printf("%s\n", tokens[i]);
     }*/

    if (strcmp(tokens[0], "cd") == 0) {
        if (num_tokens == 1) {
            tokens[1] = malloc(size_buff);
            strcpy(tokens[1], "~");
        }
        cd_handler(tokens);
        //free(tokens[1]);
    } else if (strcmp(tokens[0], "pwd") == 0) {
        pwd_handler();
    } else if (strcmp(tokens[0], "ls") == 0) {
        ls_handler(tokens, num_tokens, curr_dir, home_dir);
    } else if (strcmp(tokens[0], "echo") == 0) {
        echo_handler(tokens, num_tokens);
    } else if (strcmp(tokens[0], "exit") == 0) {
        _exit(1);
    } else if (strcmp(tokens[0], "clear") == 0) {
        clearScreen();
    } else if (strcmp(tokens[0], "pinfo") == 0) {
        if (num_tokens == 1) {
            tokens[1] = malloc(10);
            sprintf(tokens[1], "%d", getpid());
        }
        pinfo_handler(tokens);
    } else
        make_process(tokens, num_tokens);
}

void get_commands(char *line) {
    // printf("%s--\n", line);
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


#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

int main() {
    clearScreen();
    shell_name = getShellName();
    if (getcwd(home_dir, size_buff) == NULL) {
        perror("getcwd failed");
    }
    if (home_dir[strlen(home_dir) - 1] != '/') {
        strcat(home_dir, "/");
    }
    strcpy(curr_dir, home_dir);
    updateShowDir();
    while (1) {
        printGreen();
        printf("%s%s", shell_name, show_dir);
        resetColor();
        char line[500];
        scanf(" %499[^\n]%*c", line);
        get_commands(line);
        //printf("%s", home_dir);

    }

}

#pragma clang diagnostic pop


