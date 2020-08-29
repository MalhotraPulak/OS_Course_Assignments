#include<stdio.h>
#include<unistd.h>
#include<sys/utsname.h>
#include<stdlib.h>
#include<string.h>

char * getShellName(){
  const int max_len = 200;
  char login[max_len];
  char hostname[max_len];
  // getlogin_r
  if(getlogin_r(login, 100) == 0){
    // printf("%s\n", login); 
  } else {
    perror("Cant get login name");
  }
  // sysname
  struct utsname sys_name;
  if(uname(&sys_name) == 0){
    // printf("%s\n", sys_name.sysname);
  } else {
    perror("Cant get system name");
  }
  // gethostname
  if(gethostname(hostname, 100) == 0){
     // printf("%s\n", hostname);
  } else {
    perror("Cant get hostname");
  }
  char * final_name = (char *) malloc(500);
  strcpy(final_name, login);
  strcat(final_name, "@");
  strcat(final_name, hostname);
  strcat(final_name, ":");
  return final_name;
}

void clearScreen(){
  printf("\e[1;1H\e[2J");
}

void printGreen(){
  printf("%s", "\x1B[32m");
}

void resetColor(){
  printf("%s", "\x1B[0m");
}

void processInput(char* input){
   printf("%s\n", input);
   char * tokens[100];
   int num_tokens = 0;
}


int main(){
  clearScreen();
  char * shell_name = getShellName();
  char directory[500];
  strcpy(directory, "~");
  char buffer[100];
  while(1){
    printGreen();
    printf("<%s%s>", shell_name, directory); 
    resetColor();
    char line[500];
    scanf(" %499[^\n]%*c", line); 
    processInput(line);
  }

}


