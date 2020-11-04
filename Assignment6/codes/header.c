//
// Created by Pulak Malhotra on 04/11/20.
//

#include "header.h"


int getInt(int socket) {
    int32_t ret;
    char *data = (char *) &ret;
    int left = sizeof(ret);
    int rc;
    do {
        rc = read(socket, data, left);
        if (rc <= 0) { /* instead of ret */
            perror("ffff");
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                // use select() or epoll() to wait for the socket to be readable again
            } else if (errno != EINTR) {
                return -1;
            }
        } else {
            data += rc;
            left -= rc;
        }
    } while (left > 0);
    return ntohl(ret);
}

int sendInt(int num, int fd) {
    int32_t conv = htonl(num);
    char *data = (char *) &conv;
    int left = sizeof(conv);
    int rc;
    do {
        rc = write(fd, data, left);
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
    fprintf(stderr, "Got a string\n");
    return buff;
}

int sendString(char *str, int socket){
    printf("sending %s\n", str);
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
