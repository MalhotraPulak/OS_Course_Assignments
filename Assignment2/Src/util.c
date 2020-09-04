//
// Created by Pulak Malhotra on 31/08/20.
//
#include "headers.h"
#include "util.h"


int max(int a, int b) {
    return a > b? a : b;
}
int min(int a, int b)
{
    return  a< b ? a : b;
}


// handles relative and home and absolute addressing
void get_raw_address(char *new_address, char *cd_location,const char* curr_dir, const char* home_dir) {
    if (cd_location[0] == '/') {
        // Absolute address
        strcpy(new_address, cd_location);
    } else if (cd_location[0] == '~') {
        strcpy(new_address, home_dir);
        strcat(new_address, cd_location + 1);
        //printf("%s", new_address);
    } else {
        // Relative address
        // check if the file address has ./ or not
        // not needed now
       /* if (cd_location[0] == '.' && cd_location[1] == '/') {
            cd_location++;
            cd_location++;
        }*/
        // copy current directory in new address
        strcpy(new_address, curr_dir);
        // if no / at the end of curr_dir now there is
        if (new_address[strlen(new_address) - 1] != '/')
            strcat(new_address, "/");
        strcat(new_address, cd_location);
    }


}
void printGreen() {
    printf("%s", "\x1B[32m");
}

void printBlue(){
    printf("%s", "\x1B[34m");
}

void resetColor() {
    printf("%s", "\x1B[0m");
}
void clearScreen() {
    printf("\e[1;1H\e[2J");
}