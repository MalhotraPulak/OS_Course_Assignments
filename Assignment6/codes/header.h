//
// Created by Pulak Malhotra on 04/11/20.
//

#ifndef ASSIGNMENT6_HEADER_H
#define ASSIGNMENT6_HEADER_H
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "netinet/in.h"
#include <string.h>
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

#define PORT 9002
#define CHUNK 10000


long long int getInt(int socket);

int sendInt(long long int num, int socket);

int sendString(char *, int socket);

char *getString(int socket);


int sendFile(const char *name, long long int size, int socket);
int getFile(const char *name, long long int , int socket);

#endif //ASSIGNMENT6_HEADER_H
