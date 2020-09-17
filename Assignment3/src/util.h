//
// Created by Pulak Malhotra on 31/08/20.
//

#ifndef UNTITLED_UTIL_H
#define UNTITLED_UTIL_H
char showDir[size_buff];
char homeDir[size_buff]; // has / in the end
char currDir[size_buff]; // has / in the end
void get_raw_address(char *new_address, char *cd_location,const char* curr_dir, const char* home_dir);

void printGreen();

void printBlue();
void printYellow();
void printCyan();
void welcomeMessage();
void resetColor();
void clearScreen();
int max(int a, int b);
int min(int a, int b);
char * trim_whitespace(char *);
void pwd_handler();
void echo_handler(char *tokens[], int num);
void cd_handler(char * []);
char * getShellName();
void updateShowDir();


#endif //UNTITLED_UTIL_H
