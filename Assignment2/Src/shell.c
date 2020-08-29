#include<stdio.h>
#include<unistd.h>
#include<sys/utsname.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

const int size_buff = 500;

char *getShellName();

char *shell_name;
char show_dir[size_buff];
char home_dir[size_buff];
char curr_dir[size_buff];

char *getShellName() {
    const int max_len = 200;
    char login[max_len];
    char hostname[max_len];
    // getlogin_r
    if (getlogin_r(login, 100) == 0) {
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
    if (gethostname(hostname, 100) == 0) {
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
void cleanupAddress(char *add) {
    char* add2 = add;
    strtok(add2, "/");

}


void cd_handler(char *token[]) {
    char * cd_location = malloc(size_buff);
    strcpy(cd_location, token[1]);
    char new_address[size_buff];
    if(cd_location[0] == '/') {
        // Absolute address
        strcpy(new_address, cd_location);
    }
    else {
        // Relative address
        // check if the file address has ./ or not
        if (cd_location[0] == '.' && cd_location[1] == '/') {
            cd_location++;
            cd_location++;
        }
        // copy current directory in new address
        strcpy(new_address, curr_dir);
        // if no / at the end of curr_dir now there is
        if(new_address[strlen(new_address) - 1] != '/')
            strcat(new_address, "/");
        strcat(new_address, cd_location);
    }
    struct stat stats_dir;
    if(stat(new_address, &stats_dir) == 0 && (S_IFDIR & stats_dir.st_mode)){
       printf("new address is %s\n", new_address);
       cleanupAddress(new_address);
       strcpy(curr_dir, new_address);
    } else {
        printf("directory does not exist: %s\n", new_address);
    }

}
void pwd_handler() {
    printf("%s\n", curr_dir);
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
        cd_handler(tokens);
    } else if (strcmp(tokens[0], "pwd") == 0) {
        pwd_handler();
    } else {
        perror("Not a valid command");
    }
}

// the directory from which shell is invoked is the home directory
// opendir - get a pointer to directory stream
// readdir - get next directory entry in directory stream
// closedir - closes directory stream 
// getcwd - get current working directory
int main() {
    clearScreen();
    strcpy(show_dir, "~");
    shell_name = getShellName();
    if (getcwd(home_dir, size_buff) == NULL) {
        perror("getcwd failed");
    }
    strcpy(curr_dir, home_dir);
    while (1) {
        printGreen();
        printf("<%s%s>", shell_name, show_dir);
        resetColor();
        char line[500];
        scanf(" %499[^\n]%*c", line);
        processInput(line);
        //printf("%s", home_dir);

    }

}


