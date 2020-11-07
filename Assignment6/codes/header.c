//
// Created by Pulak Malhotra on 04/11/20.
//

#include <assert.h>
#include "header.h"

int printProgress(double progress, double *prev_progress){
    if (progress - *prev_progress >= 0.0001) {
        printf("Progress: %0.2lf%%\r", progress * 100);
        *prev_progress = progress;
        fflush(stdout);
    }
}
int sendAck(int socket) {
    char buff[1] = {1};
    write(socket, buff, 1);
}


int getAck(int socket) {
    char buff[1];
    read(socket, buff, 1);
    if(buff[0] != 1){
        perror("Something off\n");
    }
    return buff[0];
}

long long int getInt(int socket) {
    long long int ret;
    char *data = getString(socket);
    ret = strtoll(data, NULL, 10);
    return ret;
}

int sendInt(long long int num, int socket) {
    char str[256];
    sprintf(str, "%lld", num);
    return sendString(str, socket);
}


char *getString(int socket) {
    char *buff = malloc(FILENAME_MAX);
    int bytesRead = read(socket, buff, FILENAME_MAX);
    if (bytesRead == 0) {
        perror("Read 0 bytes");
        return NULL;
    } else if (bytesRead < 0) {
        perror("Some error in reading a string");
        return NULL;
    }
    sendAck(socket);
    return buff;
}

int sendString(char *str, int socket) {
//    printf("Sending %s\n", str);
    char *data = str;
    int left = strlen(str);
    int rc;
    do {
        rc = write(socket, data, left);
        if (rc < 0) {
            perror("send string cant write");
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                // use select() or epoll() to wait for the socket to be writable again
            } else if (errno != EINTR) {
                return -1;
            }
        } else {
            data += rc;
            left -= rc;
        }
    } while (left > 0);
    // acknowledgement check
    return getAck(socket);
}

int sendFile(const char *name, long long int size, int socket) {
    printf("Sending file " GRN "%s\n" RESET, name);
    int fd = open(name, O_RDONLY);
    if (fd == -1) {
        perror(RED"Sending Failed"RESET);
        return 1;
    }
    double progress;
    double prev_progress = 0;
    char buff[CHUNK];
    long long int left = size;
    while (left > 0) {
        int read_bytes = read(fd, buff, CHUNK);
        int sent_bytes = 0;
        if (read_bytes > 0) {
            sent_bytes = write(socket, buff, read_bytes);
        } else if (read_bytes == -1) {
            perror("Error reading");
            break;
        } else if (read_bytes == 0){
            perror("FDFD");
        }
        //fprintf(stderr, "Sent chunk \n");
        //fprintf(stderr, "%d read %d sent \n", read_bytes, sent_bytes);
        assert(read_bytes == sent_bytes);
        left -= read_bytes;

        if (getAck(socket) != 1) {
            perror("Something wrong \n");
            break;
        }
        if (size != 0) {
            progress = 1.0 - ((double) left) / (double) size;
        } else {
            progress = 1.0;
        }
        printProgress(progress, &prev_progress);
        //fprintf(stderr, "Got ack\n");
    }
    printf("Progress: 100.00%%\n\n");
    close(fd);
    return 0;
}

int getFile(const char *name, long long int size, int socket) {
    printf("Ready to receive file "GRN"%s\n"RESET, name);
    int fd = open(name, O_WRONLY | O_CREAT, 0644);
    if (fd == -1) {
        perror(RED "Receiving Failed");
        return 1;
    }
    double progress;
    double prev_progress = 0;
    char buff[CHUNK];
    long long int left = size;
    while (left > 0) {
        int read_bytes = read(socket, buff, CHUNK);
        int sent_bytes = 0;
        if (read_bytes > 0) {
            sent_bytes = write(fd, buff, read_bytes);
        } else if (read_bytes == -1) {
            perror("Error reading");
            break;
        }
        //fprintf(stderr, "Gpt chunk \n");
        //fprintf(stderr, "%d read %d sent \n", read_bytes, sent_bytes);
        assert(read_bytes == sent_bytes);
        left -= read_bytes;
        //fprintf(stderr, "Sending ack \n");
        sendAck(socket);
        if (size != 0) {
            progress = 1.0 - ((double) left) / (double) size;
        } else {
            progress = 1.0;
        }
        printProgress(progress, &prev_progress);

    }
    printf("Progress: 100.00%%\n");
    printf("Received file "GRN"%s\n"RESET, name);
    close(fd);
    return 0;
}

