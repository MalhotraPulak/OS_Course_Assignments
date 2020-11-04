//
// Created by Pulak Malhotra on 04/11/20.
//

#ifndef ASSIGNMENT6_HEADER_H
#define ASSIGNMENT6_HEADER_H

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "netinet/in.h"
#include <string.h>

#define PORT 9003
#define CHUNK 1000


int getInt(int socket);

int sendInt(int num, int socket);

int sendString(char *, int socket);

char *getString(int socket);


#endif //ASSIGNMENT6_HEADER_H
