#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<stdio.h>
#include<string.h>

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

int mini(int a, int b) {
    return (a < b) ? a : b;
}
int max(int a, int b) {
    return (a > b) ? a : b;
}

signed main(int argc, char *argv[]) {
    if (argc < 2) {
        perror("Invalid Arguments");
        _exit(1);
    }

    char *pathname = argv[1];
    char *filename = pathname;
    struct stat file_stats;
    int file_desc_end = open(pathname, O_RDWR);
    if (stat(pathname, &file_stats) == -1) {
        perror("File does not exist");
        _exit(1);
    }
    if (mkdir("./Assignment", S_IRUSR | S_IWUSR | S_IXUSR) == -1) {
        perror("Directory cannot be created");
        _exit(1);
    }
    char new_file[1000];
    strcpy(new_file, "./Assignment/");
    strcat(new_file, filename);
    int file_desc_start = open(new_file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    int total_bytes = file_stats.st_size;
    int bytes_read = 0;
    int done_percent = -1;
    int read_speed = 5;
    int curr_location = max(0, file_stats.st_size - read_speed);
    char buff[read_speed + 1];
    while (bytes_read < total_bytes) {
        lseek(file_desc_start, bytes_read, SEEK_SET);
        lseek(file_desc_end, curr_location, SEEK_SET);
        int to_read = mini(total_bytes - bytes_read , read_speed);
        int size = read(file_desc_end, buff, to_read);
        reverse_buffer(buff, size);
        char str[32];
        sprintf(str, "Bytes read : %d", size);
        perror(str);
        write(file_desc_start, buff, size);
        curr_location -= size;
        bytes_read += size;
        int done_percent_new = (1.0 * bytes_read) / total_bytes * 100.0;
        if (done_percent_new > done_percent) {
            fflush(stdout);
            char message[100];
            int size2 = sprintf(message, "Progress = %d%%\r", done_percent_new);
            write(1, message, size2);
            done_percent = done_percent_new;
        }
    }
    return 0;
}
// TODO IMPLEMENT ERROR HANDLING
// No of bytes vs Size of file may be diff
// File name and file path separation
// empty line