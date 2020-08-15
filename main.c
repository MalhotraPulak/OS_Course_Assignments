#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<stdio.h>
#include<string.h>

signed main(int argc, char *argv[])
{
    printf("start\n");
    char buff[2];
    char* pathname = argv[1];
    char* filename = "lol.txt";
    printf("%s\n", pathname);
    struct stat sb;
    int file_desc_end = open("input.txt",  O_RDWR);
    stat("input.txt", &sb);
    int check_dir = mkdir("./Assignment", 0700);
    char new_file[1000];
    strcpy(new_file, "./Assignment/");
    strcat(new_file, filename);
    printf("%lld\n", sb.st_size);
    int file_desc_start = open(new_file,  O_WRONLY |O_CREAT, 0777 );
    int curr_location = sb.st_size - 1;
    int total_bytes = sb.st_size;
    int bytes_read = 0;
    while(bytes_read < total_bytes){
        lseek(file_desc_start, bytes_read, SEEK_SET);
        lseek(file_desc_end, curr_location, SEEK_SET);
        read(file_desc_end, buff, 1);
        //printf("%c %c\n", buff[0], buff2[0]);
        lseek(file_desc_start, bytes_read, SEEK_SET);
        write(file_desc_start, buff, 1);
        curr_location --;
        bytes_read++;
    }
    int close(int fd);
    printf("Job done");
    printf("%s", new_file);
    return 0;
}

