//
// Created by Pulak Malhotra on 02/09/20.
//
#include "headers.h"
#include "nightswatch.h"
#include "util.h"
#include <termios.h>
#include <ctype.h>

struct termios orig_termios;

void reset_terminal_mode_to_canon() {
    tcsetattr(0, TCSANOW, &orig_termios);
}

void set_terminal_raw_mode() {
    struct termios new_termios; // terminal attributes
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));
    atexit(reset_terminal_mode_to_canon); // incase some error occurs
    //cfmakeraw(&new_termios); // only standard on BSD systems, flips the ICANON AND ECHO flag
    //new_termios.c_iflag &= ~(ICRNL | IXON); control key
    new_termios.c_oflag &= ~(OPOST); // \n -> \r turn off output processing
    new_termios.c_lflag &= ~(ECHO | ICANON); // turn off echo and canonical mode in terminal
    tcsetattr(0, TCSANOW, &new_termios);
}

int getch() {
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
}

void cpu() {
    char a[100];
    strcpy(a, "cpu");
    write(2, a, strlen(a));

    







}

void new_born() {
    struct dirent *dir_stuff;
    DIR *dir = opendir("/proc");
    time_t max_time = 0;
    if (dir == NULL) {
        perror("cannot access /proc");
        return;
    }
    char pids[size_buff];
    strcpy(pids, "-1");
    while ((dir_stuff = readdir(dir)) != NULL) {
        if (isdigit(dir_stuff->d_name[0])) {
            //write(2, "gre", 4);
            char add[1000];
            sprintf(add, "/proc/%s/stat", dir_stuff->d_name);
            struct stat t;
            if (stat(add, &t) == -1)
                continue;
            time_t tt = t.st_mtime;
            if (max_time == 0) {
                max_time = tt;
                strcpy(pids, dir_stuff->d_name);
            } else {
                if (difftime(tt, max_time) > 0) {
                    max_time = tt;
                    strcpy(pids, dir_stuff->d_name);
                }
            }

        }
    }
    char a[100];
    sprintf(a, "- %s\n", pids);
    write(1, a, strlen(a));
    closedir(dir);
}

int kbhit() {
    // has a key been pressed or what
    struct timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

void nightswatch_handler(char *tokens[], int no) {
    if (no != 3) {
        printf("2 args required : -<int> <command>");
        return;
    }
    int seconds;
    if (tokens[1][0] != '-') {
        printf("2 args required : -<int> <command>");
        return;
    }
    tokens[1]++;
    seconds = atoi(tokens[1]);
    //printf("%d", seconds);
    while (true) {
        if (strcmp(tokens[2], "interrupt") == 0) {
            cpu();
        } else if (strcmp(tokens[2], "newborn") == 0) {
            new_born();
        } else {
            break;
        }
        set_terminal_raw_mode();
        time_t secs = seconds; // 2 minutes (can be retrieved from user's input)
        time_t startTime = time(NULL);
        while (time(NULL) - startTime < secs && !kbhit()) {
        }
        //sleep(seconds);
        if (kbhit()) {
            if (getch() == 'q') {
                reset_terminal_mode_to_canon();
                return;
            }
        }
        reset_terminal_mode_to_canon();
        fflush(stdout);
    }

}

// nightswatch -1 interrupt
// nightswatch -1 newborn


