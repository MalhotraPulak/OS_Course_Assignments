//
// Created by Pulak Malhotra on 31/08/20.
//
#include "headers.h"
#include "util.h"


int max(int a, int b) {
    return a > b ? a : b;
}

int min(int a, int b) {
    return a < b ? a : b;
}

char *old_add;

// handles relative and home and absolute addressing
int get_raw_address(char *new_address, char *cd_location) {
    if (strcmp(cd_location, "-") == 0) {
        strcpy(new_address, old_add);
        return 911;
    }
    if (cd_location[0] == '/') {
        // Absolute address
        strcpy(new_address, cd_location);
    } else if (cd_location[0] == '~') {
        strcpy(new_address, homeDir);
        strcat(new_address, cd_location + 1);
        //printf("%s", new_address);
    } else {
        // Relative address
        // check if the file address has ./ or not
        // not needed now
        /* if (cd_location[0] == '.' && cd_location[1] == '/') {
             cd_location++;
             cd_location++;
         }*/
        // copy current directory in new address
        strcpy(new_address, currDir);
        // if no / at the end of curr_dir now there is
        if (new_address[strlen(new_address) - 1] != '/')
            strcat(new_address, "/");
        strcat(new_address, cd_location);
    }

    return 0;
}


void printGreen() {
    printf("%s", "\033[1m\033[32m");
}

void printBlue() {
    printf("%s", "\033[1m\033[34m");
}

void printCyan() {
    printf("%s", "\033[1m\033[36m");
}

void resetColor() {
    printf("%s", "\033[1m\033[0m");
}

void printYellow() {
    printf("%s", "\033[1m\033[33m");
}

void clearScreen() {
    printf("\e[1;1H\e[2J");
}

void welcomeMessage() {
    printf(" ---------------\n");
    printf("|   Welcome     |\n");
    printf("|     to        |\n");
    printf("|  Boring Shell |\n");
    printf(" ---------------\n");
}

char *trim_whitespace(char *line) {
    int t = 0;
    for (int i = 0; i < strlen(line); i++) {
        if (line[i] == ' ' || line[i] == '\t' || line[i] == '\n') {
            t++;
        } else {
            break;
        }
    }
    for (int i = 0; i < t; i++) {
        line++;
    }
    for (int i = strlen(line) - 1; i >= 0; i--) {
        if (line[i] == ' ' || line[i] == '\t' || line[i] == '\n') {
            line[i] = '\0';
        } else {
            break;
        }
    }
    return line;
}


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
    if (old_add == NULL) {
        old_add = malloc(size_buff);
        strcpy(old_add, homeDir);
    }
    char new_address[size_buff];
    int f = get_raw_address(new_address, cd_location);
    struct stat stats_dir;
    char ol[size_buff];
    strcpy(ol, currDir);
    if (stat(new_address, &stats_dir) == 0 && (S_IFDIR & stats_dir.st_mode)) {
        if (chdir(new_address) == -1) {
            fprintf(stderr, "cd : directory does not exist\n");
            exit_code = 1;
        } else {
            strcpy(old_add, ol);
        }
        if (getcwd(currDir, size_buff) == NULL) {
            fprintf(stderr, "cd : getcwd failed\n");
        }
        strcat(currDir, "/");
        updateShowDir();
        if (f == 911)
            pwd_handler();
    } else {
        fprintf(stderr, "cd : directory does not exist: %s\n", token[1]);
        exit_code = 1;
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
