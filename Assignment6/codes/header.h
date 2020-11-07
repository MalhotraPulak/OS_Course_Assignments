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

#define PORT 9002
#define CHUNK 10000


long long int getInt(int socket);

int sendInt(long long int num, int socket);

int sendString(char *, int socket);

char *getString(int socket);


int sendFile(const char *name, long long int size, int socket);
int getFile(const char *name, long long int , int socket);

#endif //ASSIGNMENT6_HEADER_H
