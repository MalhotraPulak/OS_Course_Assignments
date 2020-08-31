#include "headers.h"
#include "pinfo.h"
#include "process_maker.h"

#define size_buff 500


char *getShellName();

char *shell_name;
char show_dir[size_buff]; // has / in the end
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

void getParent(char *add) {
    int len = strlen(add);
    for (int i = len - 2; i >= 0; i--) {
        if (add[i] == '/') {
            add[i + 1] = '\0';
            return;
        }
    }


}

char *month_name(int no, char *name) {
    no += 1;
    switch (no) {
        case 1:
            strcpy(name, "Jan");
            break;
        case 2:
            strcpy(name, "Feb");
            break;
        case 3:
            strcpy(name, "Mar");
            break;
        case 4:
            strcpy(name, "Apr");
            break;
        case 5:
            strcpy(name, "May");
            break;
        case 6:
            strcpy(name, "Jun");
            break;
        case 7:
            strcpy(name, "Jul");
            break;
        case 8:
            strcpy(name, "Aug");
            break;
        case 9:
            strcpy(name, "Sep");
            break;
        case 10:
            strcpy(name, "Oct");
            break;
        case 11:
            strcpy(name, "Nov");
            break;
        case 12:
            strcpy(name, "Dec");
            break;
        default:
            break;
    }
    return name;
}


void cleanupAddress(char *add) {
    char add2[size_buff];
    strcpy(add2, add);
    char refinedAddress[size_buff];
    strcpy(refinedAddress, "/");
    char *dirname = strtok(add2, "/");
    if (dirname == NULL) return;
    while (dirname != NULL) {
        if (strcmp(dirname, "..") == 0) {
            getParent(refinedAddress);
        } else if (strcmp(dirname, ".") == 0) {

        } else {
            //strcpy(parent_dir, refinedAddress);
            strcat(refinedAddress, dirname);
            strcat(refinedAddress, "/");

        }
        dirname = strtok(NULL, "/");
    }
    //printf("refined address = %s \n", refinedAddress);
    strcpy(add, refinedAddress);

}

// cd handle edge case around /
void get_raw_address(char *new_address, char *cd_location) {
    if (cd_location[0] == '/') {
        // Absolute address
        strcpy(new_address, cd_location);
    } else if (cd_location[0] == '~') {
        strcpy(new_address, home_dir);
        strcat(new_address, cd_location + 1);
        //printf("%s", new_address);
    } else {
        // Relative address
        // check if the file address has ./ or not
        if (cd_location[0] == '.' && cd_location[1] == '/') {
            cd_location++;
            cd_location++;
        }
        // copy current directory in new address
        strcpy(new_address, curr_dir);
        // if no / at the end of curr_dir now there is
        if (new_address[strlen(new_address) - 1] != '/')
            strcat(new_address, "/");
        strcat(new_address, cd_location);
        if (new_address[strlen(new_address) - 1] != '/')
            strcat(new_address, "/");

    }


}

void cd_handler(char *token[]) {
    char cd_location[size_buff];
    strcpy(cd_location, token[1]);
    char new_address[size_buff];
    get_raw_address(new_address, cd_location);
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

void permission_format(int mode, char *permission) {
    int i;
    int index = 0;
    for (i = 8; i >= 0; i--) {
        int t = 1 << i;
        if (mode & t) {
            if (i % 3 == 0) {
                permission[index] = 'x';
            } else if (i % 3 == 1) {
                permission[index] = 'w';
            } else {
                permission[index] = 'r';
            }
        } else {
            permission[index] = '-';
        }
        index++;
    }
    permission[index] = '\0';

}

void detail_print(const char *add, char *name) {
    struct stat data;
    //printf("%s", add);
    if (stat(add, &data) == -1) {
        printf("%s\n", add);
        perror("Error getting stat struct");
    }
    int links = data.st_nlink;
    struct passwd *pws = getpwuid(data.st_uid);
    char *user_name = pws->pw_name;
    struct group *grp = getgrgid(data.st_gid);
    char *group_name = grp->gr_name;
    long long bytes = data.st_size;
    time_t l_m = data.st_mtime;
    struct tm last_mod;
    localtime_r(&l_m, &last_mod);
    int month = last_mod.tm_mon;
    int day = last_mod.tm_mday;
    int min = last_mod.tm_min;
    int hour = last_mod.tm_hour;
    char permission[100];
    char perm[100];
    if (data.st_mode & S_IFDIR) {
        strcpy(permission, "d");
    } else {
        strcpy(permission, "-");
    }
    permission_format(data.st_mode, perm);
    char monthName[5];
    month_name(month, monthName);
    strcat(permission, perm);
    printf("%s%5d%10s%10s%10lld %s %02d %02d:%02d %s\n", permission, links, user_name, group_name, bytes,
           monthName, day,
           hour, min, name);
}

void sort_names(char name[][size_buff], int n) {
    char temp[size_buff];
    int i, j;
    for (i = 0; i < n - 1; i++) {
        for (j = i + 1; j < n; j++) {
            if (strcmp(name[i], name[j]) > 0) {
                strcpy(temp, name[i]);
                strcpy(name[i], name[j]);
                strcpy(name[j], temp);
            }
        }
    }
}

void print_ls_data(const char *location, int hidden, int details, int file, char *outName) {
    struct dirent *dir_stuff;
    printf("%s :\n", outName);
    if (file == 1) {
        if (details)
            detail_print(location, outName);
        else
            printf("%s\n", outName);
        return;
    }
    if (details) {
        struct stat tt;
        if (stat(location, &tt) == -1) {
            printf("error\n");
        };
        printf("total = %lld\n", tt.st_blocks);
        printf("total = %lld\n", tt.st_size);
    }
    DIR *dir = opendir(location);
    int total = 0;
    while (readdir(dir) != NULL) {
        total++;
    }
    char names[total][size_buff];
    dir = opendir(location);
    int count = 0;
    while ((dir_stuff = readdir(dir)) != NULL) {
        strcpy(names[count], dir_stuff->d_name);
        count++;
    }
    sort_names(names, total);
    for (int i = 0; i < total; i++) {
        char *curr_name = names[i];
        if (curr_name[0] == '.' && hidden == 0) {
            continue;
        }
        if (!details)
            printf("%s\n", curr_name);
        else {
            char element_address[size_buff];
            strcpy(element_address, location);
            strcat(element_address, curr_name);
            detail_print(element_address, curr_name);
        }
    }
    //free(names);
}


// ls file = file
// ls @ +
// ls dir size is 4kb or whatever
// 512 bytes shit
// multiple dir for ls
void ls_handler(char *tokens[], int no) {
    char location[size_buff];
    int hidden = 0, details = 0;
    int i;
    for (i = 1; i < no; i++) {
        if (tokens[i][0] == '-') {
            for (int j = 1; j < strlen(tokens[i]); j++) {
                if (tokens[i][j] == 'l')
                    details = 1;
                else if (tokens[i][j] == 'a')
                    hidden = 1;
                else {
                    printf("ls : invalid flag only l and a supported\n");
                    return;
                }
            }
        } else {
            break;

        }
    }
    if (i == no) {
        tokens[no] = strdup(".");
        no++;
    }
    for (; i < no; i++) {
        get_raw_address(location, tokens[i]);
        struct stat stats_dir;

        if (stat(location, &stats_dir) == 0 && (S_IFDIR & stats_dir.st_mode)) {
            print_ls_data(location, hidden, details, 0, tokens[i]);
        } else if (S_IFREG & stats_dir.st_mode) {
            // same as ls dir but on one file
            print_ls_data(location, hidden, details, 1, tokens[i]);

        } else {
            printf("ls : No such file or directory\n");
        }
        printf("\n");
    }

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
        ls_handler(tokens, num_tokens);
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
//printf("hre");


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
        printf("<%s%s>", shell_name, show_dir);
        resetColor();
        char line[500];
        scanf(" %499[^\n]%*c", line);
        get_commands(line);
        //printf("%s", home_dir);

    }

}

#pragma clang diagnostic pop


