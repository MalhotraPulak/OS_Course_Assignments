//
// Created by Pulak Malhotra on 31/08/20.
//
#include "headers.h"
#include "util.h"
#include "ls.h"

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

void detail_print(const char *add, char *name, int detail) {
    struct stat data;
    //printf("%s", add);
    if (lstat(add, &data) == -1) {
        fprintf(stderr, "%s\n", add);
        perror("Error getting stat struct");
        return;
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
    if(data.st_mode & S_IXUSR){
        //printGreen();
    }
    if (data.st_mode & S_IFDIR) {
        strcpy(permission, "d");
        //printBlue();
    } else if (S_ISLNK(data.st_mode)) {
        strcpy(permission, "l");
        //printYellow();
    } else{
        strcpy(permission, "-");
    }
    if (!detail) {
        printf("%s\n", name);
        //resetColor();
        return;
    }
    permission_format(data.st_mode, perm);
    char monthName[5];
    month_name(month, monthName);
    strcat(permission, perm);

    printf("%s%5d%10s%10s%10lld %s %02d %02d:%02d %s\n", permission, links, user_name, group_name, bytes,
           monthName, day,
           hour, min, name);
    //resetColor();
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


void print_ls_data(const char *location, int hidden, int details, int file, char *outName, int detail) {
    struct dirent *dir_stuff;
    if (strcmp(outName, "") != 0 && detail)
        printf("%s :\n", outName);
    if (file == 1) {
        // if (details)
        detail_print(location, outName, details);
        //      else
        //    printf("%s\n", outName);
        return;
    }
    int total_blocks = 0;
    DIR *dir = opendir(location);
    int total = 0;
    while (readdir(dir) != NULL) {
        total++;
    }
    closedir(dir);
    char names[total][size_buff];
    dir = opendir(location);
    int count = 0;
    while ((dir_stuff = readdir(dir)) != NULL) {
        strcpy(names[count], dir_stuff->d_name);
        count++;
        if (!hidden && (dir_stuff->d_name[0] == '.'))
            continue;
        char element_address[size_buff];
        strcpy(element_address, location);
        strcat(element_address, dir_stuff->d_name);
        struct stat data;
        //printf("%s", add);
        if (stat(element_address, &data) == -1) {
            fprintf(stderr, "%s\n", element_address);
            perror("Error getting stat struct");
            continue;
        }
        total_blocks += data.st_blocks;

    }
    sort_names(names, total);
    if (details)
        printf("total %d\n", total_blocks / 2);
    for (int i = 0; i < total; i++) {
        char *curr_name = names[i];
        if (curr_name[0] == '.' && hidden == 0) {
            continue;
        }
        /*  if (!details)
              printf("%s\n", curr_name);
          else {*/
        char element_address[size_buff];
        strcpy(element_address, location);
        strcat(element_address, curr_name);
        detail_print(element_address, curr_name, details);

    }
    closedir(dir);
    free(dir_stuff);
    //free(names);
}


// ls @ +
void ls_handler(char *tokens[], int no, const char *curr_dir, const char *home_dir) {
    char location[size_buff];
    int hidden = 0, details = 0;
    int dir[no + 1];
    int dirs = 0;
    for (int i = 1; i < no; i++) {
        if (tokens[i][0] == '-') {
            for (int j = 1; j < strlen(tokens[i]); j++) {
                if (tokens[i][j] == 'l')
                    details = 1;
                else if (tokens[i][j] == 'a')
                    hidden = 1;
                else {
                    fprintf(stderr, "ls : invalid flag only l and a supported\n");
                    return;
                }
            }
        } else {
            dir[dirs] = i;
            dirs++;
        }
    }
    int detail = 1;
    if (dirs == 0) {
        detail = 0;
        tokens[no] = strdup(".");
        dir[dirs] = no;
        dirs++;
    }

    for (int j = 0; j < dirs; j++) {
        int i = dir[j];
        get_raw_address(location, tokens[i], curr_dir, home_dir);
        struct stat stats_dir;
        //printf("%s", location);
        if (stat(location, &stats_dir) == 0 && (S_IFDIR & stats_dir.st_mode)) {
            if (location[strlen(location) - 1] != '/')
                strcat(location, "/");
            print_ls_data(location, hidden, details, 0, tokens[i], detail);
        } else if (S_IFREG & stats_dir.st_mode) {
            print_ls_data(location, hidden, details, 1, tokens[i], detail);

        } else {
            fprintf(stderr, "ls : No such file or directory\n");
        }
        //printf("\n");
    }

}

