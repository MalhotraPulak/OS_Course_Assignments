#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

void solve(char filename[], struct stat stats) {
    signed int flags[] = {
            S_IRUSR, S_IWUSR, S_IXUSR,
            S_IRGRP, S_IWGRP, S_IXGRP,
            S_IROTH, S_IWOTH, S_IXOTH
    };
    char *entity[] = {
            "User", "User", "User",
            "Group", "Group", "Group",
            "Others", "Others", "Others"
    };

    for (int i = 0; i < 9; i++) {
        char verd[100];
        if (stats.st_mode & flags[i]) {
            strcpy(verd, "Yes");
        } else {
            strcpy(verd, "No");
        }
        char per_type[100];
        if (i % 3 == 0) {
            strcpy(per_type, "read");
        } else if (i % 3 == 1) {
            strcpy(per_type, "write");
        } else {
            strcpy(per_type, "execute");
        }
        char message[100];
        int size = sprintf(message, "%s has %s permission on %s: %s\n", entity[i], per_type, filename, verd);
        write(1, message, size);

    }

}


int main(int arg_no, char *args[]) {
    if(arg_no < 4){
        perror("Invalid Arguments");
        exit(EXIT_FAILURE);
    }
    struct stat stats_old, stats_new, stats_dir;
    char *oldfile = args[1];
    char *newfile = args[2];
    char *dir = args[3];
    //printf("%s", dir);
    if (stat(oldfile, &stats_old) == -1) {
        perror("old file");
        exit(EXIT_FAILURE);
    }
    if (stat(newfile, &stats_new) == -1) {
        perror("new file");
        exit(EXIT_FAILURE);
    }
   // stat(newfile, &stats_new);
    if (stat(dir, &stats_dir) == 0 && (S_IFDIR & stats_dir.st_mode)) {
        char message[100];
        int size = sprintf(message, "Directory is created: Yes\n");
        write(1, message, size);
    } else {
        char message[100];
        int size = sprintf(message, "Directory is created: No\n");
        write(1, message, size);
    }

    solve("newfile", stats_new);
    solve("oldfile", stats_old);
    solve("directory", stats_dir);

}