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
int mini(int a, int b) {
    return (a < b) ? a : b;
}
int max(int a, int b) {
    return (a > b) ? a : b;
}
void reverse_buffer(char buff[], int size) {
    int l = 0, r = size - 1;
    while (l < r) {
        char t = buff[l];
        buff[l] = buff[r];
        buff[r] = t;
        l++;
        r--;
    }

}

int check(char *oldfile, char *newfile, int total_bytes){
    int read_speed = 3;
    int reader = open(oldfile, O_RDONLY);
    int matcher = open(newfile, O_RDONLY);
    int bytes_read = 0;
    char buff1[read_speed + 1];
    char buff2[read_speed + 1];
    while(bytes_read < total_bytes){
        int bytes_to_read = mini(total_bytes - bytes_read + 1, read_speed);
        lseek(matcher, total_bytes - bytes_read - read_speed, SEEK_SET);
        int size1 = read(reader, buff1, bytes_to_read);
        int size2 = read(matcher, buff2, bytes_to_read);
        reverse_buffer(buff1, size1);
        char message[100];
        int sd = sprintf(message, "%s -- %s --%d %d\n", buff1, buff2, size1, size2);
        write(1, message, sd);
        for(int i = 0; i < size1; i++){
            if(buff1[i] != buff2[i]){
                return 0;
            }
        }
        bytes_read += size1;
    }
    return 1;
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
    char total_name[100];
    strcpy(total_name, dir);
    strcat(total_name, "/");
    strcat(total_name, newfile);
    perror(total_name);
    perror(oldfile);
    //perror(total_name);
    //printf("%s", dir);
    if (stat(oldfile, &stats_old) == -1) {
        perror("old file");
        exit(EXIT_FAILURE);
    }
    if (stat(total_name, &stats_new) == -1) {
        perror("new file");
        exit(EXIT_FAILURE);
    }
    if(stats_old.st_size == stats_new.st_size && check(oldfile, total_name, stats_new.st_size)){
        char message[100];
        int size = sprintf(message, "Whether file contents are reversed in newfile: Yes\n");
        write(1, message, size);
    } else {
        char message[100];
        int size = sprintf(message, "Whether file contents are reversed in newfile: No\n");
        write(1, message, size);
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

// something wrong for bigger read speed fix this shit