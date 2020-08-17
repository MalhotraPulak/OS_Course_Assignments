#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
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
    // make sure argument is supplied
    if (argc < 2) {
        perror("Invalid Arguments");
        _exit(1);
    }
    // get path and filename
    char *pathname = argv[1];
    char *filename = strchr(pathname, '/');
    if (filename == NULL) {
        perror("Not a valid filename");
        _exit(1);
    }
    //perror(filename);
    struct stat file_stats;
    int file_desc_end = open(pathname, O_RDWR);
    if (stat(pathname, &file_stats) == -1) {
        perror("File does not exist");
        _exit(1);
    }
    if (!(file_stats.st_mode & S_IFREG)) {
        perror("File does not exist");
        _exit(1);
    }
    if (mkdir("./Assignment", S_IRUSR | S_IWUSR | S_IXUSR) == -1) {
        // perror("Directory cannot be created");
        // TODO remove use the old directory
        // Rewrite the file too if it exists what bull shit
        //_exit(1);
    }
    char new_file[1000];
    strcpy(new_file, "./Assignment");
    strcat(new_file, filename);
    //perror(new_file);
    remove(new_file);
    int file_desc_start = open(new_file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    int total_bytes = file_stats.st_size;
    int bytes_read = 0;
    float done_percent = -1;
    int read_speed = (int) 1e6; // reading 1 mb per second at max
    int curr_location = max(0, file_stats.st_size - read_speed);
    char buff[read_speed];

    while (bytes_read < total_bytes) {
        if (lseek(file_desc_end, curr_location, SEEK_SET) == -1) {
            perror("lseek on old file is invalid");
            _exit(1);
        }
        int to_read = mini(total_bytes - bytes_read, read_speed);
        int size = read(file_desc_end, buff, to_read);
        if (size == -1) {
            perror("Error reading the old file");
            _exit(1);
        }
        reverse_buffer(buff, size);
        // debug statement
        /*char str[300];
        sprintf(str, "%d %d %d %d", done_percent, to_read, total_bytes, bytes_read);
        perror(str);*/
        if (write(file_desc_start, buff, size) == -1) {
            perror("Writing to newfile failed");
            _exit(1);
        }
        // change the read pointer
        curr_location -= size;
        curr_location = max(0, curr_location);
        bytes_read += size;
        // calculate the percent done and print it as required
        float done_percent_new = (float) ((1.0 * bytes_read) / (total_bytes) * 100.0);

        if (done_percent_new - done_percent > 0.01) {
            fflush(stdout);
            char message[200];
            int size2 = sprintf(message, "Progress = %0.2f%%\r", done_percent_new);
            write(1, message, size2);
            done_percent = done_percent_new;
        }
    }
    close(file_desc_end);
    close(file_desc_start);
    //char message_final[100];
    //int size_3 = sprintf(message_final, "Progress = 100%%");
    //write(1, message_final, size_3);
    return 0;
}
// Close the file descriptors
