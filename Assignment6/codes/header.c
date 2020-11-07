//
// Created by Pulak Malhotra on 04/11/20.
//

#include <assert.h>
#include "header.h"


long long int getInt(int socket) {
    long long int ret;
    char *data = getString(socket);
    ret = strtoll(data, NULL, 10);
    return ret;
}

int sendInt(long long int num, int socket) {
    printf("\r");
    /*long long int conv = htonl(num);
    char *data = (char *) &conv;
    int left = sizeof(conv);
    int rc;
    do {
        rc = write(socket, data, left);
        if (rc < 0) {
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
    return 0;*/
    char str[256];
    sprintf(str, "%lld", num);
    sendString(str, socket);
    return 0;
}


char *getString(int socket) {
    char *buff = malloc(CHUNK);
    int bytesRead = read(socket, buff, CHUNK);
    if (bytesRead == 0) {
        perror("Read 0 bytes");
        return NULL;
    } else if (bytesRead < 0) {
        perror("Some error in reading a string");
    }
    return buff;
}

int sendString(char *str, int socket) {
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
    return 0;
}

int sendFile(const char *name, long long int size, int socket) {
    printf("Sending file %s\n", name);
    int fd = open(name, O_RDONLY);
    if (fd == -1) {
        perror("Cannot open file\n");
        return 1;
    }
    double progress;
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
        }
        //fprintf(stderr, "Sent chunk \n");
        //fprintf(stderr, "%d read %d sent \n", read_bytes, sent_bytes);
        assert(read_bytes == sent_bytes);
        left -= read_bytes;

        if (getInt(socket) != 1) {
            perror("Something wrong \n");
            break;
        }
        if (size != 0) {
            progress = 1.0 - ((double) left) / (double) size;
        } else {
            progress = 1.0;
        }
        printf("Progress: %0.2lf%%\r", progress * 100);
        //fprintf(stderr, "Got ack\n");
    }
    return 0;
}

// TODO long long
int getFile(const char *name, long long int size, int socket) {
    printf("Getting file %s\n", name);
    int fd = open(name, O_WRONLY | O_CREAT, 0644);
    if (fd == -1) {
        perror("Cannot open file\n");
        return 1;
    }
    double progress;
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
        sendInt(1LL, socket);
        if (size != 0) {
            progress = 1.0 - ((double) left) / (double) size;
        } else {
            progress = 1.0;
        }
        printf("Progress: %0.2lf%%\r", progress * 100);
    }
    printf("\n");
    fprintf(stderr, "Written file %s\n", name);
    return 0;
}