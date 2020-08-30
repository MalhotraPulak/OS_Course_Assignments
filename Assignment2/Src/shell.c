#include<stdio.h>
#include<unistd.h>
#include<sys/utsname.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

const int size_buff = 500;

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

void getParent(char *add) {
    int len = strlen(add);
    for (int i = len - 2; i >= 0; i--) {
        if (add[i] == '/') {
            add[i + 1] = '\0';
            return;
        }
    }


}

char *month_name(int no) {
    char *name = malloc(5);
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
    }
    return name;
}


void cleanupAddress(char *add) {
    char add2[size_buff];
    strcpy(add2, add);
    char *refinedAddress = malloc(size_buff);
    //char *parent_dir = malloc(size_buff);
    strcpy(refinedAddress, "/");
    //strcpy(parent_dir, "/");
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
    }


}

void cd_handler(char *token[]) {
    char *cd_location = malloc(size_buff);
    strcpy(cd_location, token[1]);
    char new_address[size_buff];
/*    if (cd_location[0] == '/') {
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
    }*/
    get_raw_address(new_address, cd_location);
    struct stat stats_dir;
    if (stat(new_address, &stats_dir) == 0 && (S_IFDIR & stats_dir.st_mode)) {
        printf("new address is %s\n", new_address);
        cleanupAddress(new_address);
        strcpy(curr_dir, new_address);
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

void detail_print(char *add, char *name) {
    // file mode number of links, owner name, group name, number of
    //     bytes in the file, abbreviated month, day-of-month file was last modi-
    //     fied, hour file last modified, minute file last modified, and the path-
    //     name
    struct stat data;
    //printf("%s", add);
    if (stat(add, &data) == -1) {
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
    strcat(permission, perm);
    printf("%s %5d %10s   %10s  %10lld  %s %02d %02d:%02d %s\n", permission, links, user_name, group_name, bytes,
           month_name(month), day,
           hour, min, name);
}

void sort_names(char *name[], int n) {
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

void print_ls_data(const char *location, int hidden, int details) {
    struct dirent *dir_stuff;
    DIR *dir = opendir(location);
    int total = 0;
    while (readdir(dir) != NULL) {
        total++;
    }
    char *names[total];
    dir = opendir(location);
    int count = 0;
    while ((dir_stuff = readdir(dir)) != NULL) {
        names[count] = malloc(size_buff);
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
}


// ls file = file
// ls @ +
// ls dir size is 4kb or whatever
// 512 bytes shit
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
                    printf("invalid flag only l and a supported\n");
                    return;
                }
            }
        } else {
            break;

        }
    }
    if (i == no) {
        get_raw_address(location, ".");
    } else {
        get_raw_address(location, tokens[i]);
    }
    struct stat stats_dir;

    if (stat(location, &stats_dir) == 0 && (S_IFDIR & stats_dir.st_mode)) {
        printf("Dir address is %s\n", location);
        cleanupAddress(location);
        print_ls_data(location, hidden, details);
    } else if (S_IFREG & stats_dir.st_mode) {
        // same as ls dir but on one file

    } else {
        printf("ls : No such file or directory\n");
    }
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
            printf("here");
        }
        cd_handler(tokens);
    } else if (strcmp(tokens[0], "pwd") == 0) {
        pwd_handler();
    } else if (strcmp(tokens[0], "ls") == 0) {
        ls_handler(tokens, num_tokens);
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
    shell_name = getShellName();
    if (getcwd(home_dir, size_buff) == NULL) {
        perror("getcwd failed");
    }
    strcat(home_dir, "/");
    strcpy(curr_dir, home_dir);
    //printf("%s", home_dir);
    updateShowDir();
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


