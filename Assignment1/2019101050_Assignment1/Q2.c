#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<stdio.h>
#include<string.h>
#define ll long long
void print_pattern(){
    char message[100];
    int s = sprintf(message, "**********************\n");
    write(1, message, s);
}
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
    print_pattern();
}


ll mini(ll a, ll b) {
    return (a < b) ? a : b;
}

ll max(ll a, ll b) {
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

int check(char *oldfile, char *newfile, ll total_bytes) {
    int read_speed = (int) 1e6;
    int reader = open(oldfile, O_RDONLY);
    int matcher = open(newfile, O_RDONLY);
    ll bytes_read = 0;
    char buff1[read_speed];
    char buff2[read_speed];
    float done_percent = -1;
    while (bytes_read < total_bytes) {
        ll bytes_to_read = mini(total_bytes - bytes_read, read_speed);
        if (lseek(matcher, max(total_bytes - bytes_read - read_speed, 0), SEEK_SET) == -1) {
            //printf("%d", total_bytes - bytes_read - read_speed);
            perror("Wrong seek on matcher, Error reading file");
            return 0;
        }
        int size1 = read(reader, buff1, bytes_to_read);
        int size2 = read(matcher, buff2, bytes_to_read);
        if (size1 != size2) {
            perror("Something is wrong");
            return 0;
        }
        reverse_buffer(buff1, size1);

        //char message[100];
        //int sd = sprintf(message, "%s ** %s ** %d %d\n", buff1, buff2, size1, size2);
        //write(1, message, sd);

        for (int i = 0; i < size1; i++) {
            if (buff1[i] != buff2[i]) {
                return 0;
            }
        }
        bytes_read += size1;
        float done_percent_new = (float) ((1.0 * bytes_read) / (total_bytes) * 100.0);

        if (done_percent_new - done_percent > 0.01) {
            fflush(stdout);
            char message[200];
            int size3 = sprintf(message, "Progress = %0.2f%%\r", done_percent_new);
            write(1, message, size3);
            done_percent = done_percent_new;
        }

    }
    close(reader);
    close(matcher);
    return 1;
}

int main(int arg_no, char *args[]) {
    if (arg_no < 4) {
        perror("Invalid Arguments");
        _exit(1);
    }
    struct stat stats_old, stats_new, stats_dir;
    char *oldfile = args[1];
    char *newfile = args[2];
    char *dir = args[3];
    int new = 1, old = 1, drr = 1;

    if (stat(oldfile, &stats_old) == -1) {
        perror("Error :: old file does not exist");
        old = 0;
    }
    if (stat(newfile, &stats_new) == -1) {
        perror("Error :: new file does not exist");
        new = 0;
    }
    if (new && old && stats_old.st_size == stats_new.st_size && check(oldfile, newfile, stats_new.st_size)) {
        char message[100];
        int size = sprintf(message, "Whether file contents are reversed in newfile: Yes\n");
        write(1, message, size);
    } else {
        char message[100];
        int size = sprintf(message, "Whether file contents are reversed in newfile: No\n");
        write(1, message, size);
    }

    if (stat(dir, &stats_dir) == 0 && (S_IFDIR & stats_dir.st_mode)) {
        char message[100];
        int size = sprintf(message, "Directory is created: Yes\n");
        write(1, message, size);

    } else {
        char message[100];
        int size = sprintf(message, "Directory is created: No\n");
        write(1, message, size);
        //solve("newfile", stats_new);
        //solve("oldfile", stats_old);
        // solve("directory", stats_dir);

        //if (mkdir(dir, S_IRUSR | S_IWUSR | S_IXUSR) == -1) {
        //   perror("Directory cannot be created");
        //  _exit(1);
        //}
        drr = 0;
    }
    print_pattern();
    if (new)
        solve("newfile", stats_new);
    if (old)
        solve("oldfile", stats_old);
    if (drr)
        solve("directory", stats_dir);
}

// something wrong for bigger read speed fix this shit
// file names fucked up
// another moodle update about dir fuckkkkk
