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
    new_termios.c_iflag &= ~(ICRNL | IXON); //control key
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

void cpu_header() {
    FILE *f;
    f = fopen("/proc/interrupts", "r");
    if (f == NULL)
        return;
    char *words[10000];
    int n = 0;
    //perror("ff");
    if (f != NULL) {
        words[n] = malloc(size_buff);
        size_t s = size_buff;
        while (fscanf(f, "%s ", words[n]) != -1) {
            words[++n] = malloc(size_buff);
        }
    }
    // get cpu cores
    for (int i = 0; i < n; i++) {
        if (isdigit(words[i][0])) {
            break;
        } else {
            char a[100];
            sprintf(a, "%10s ", words[i]);
            write(2, a, strlen(a));
        }
    }
    write(2, "\n", 2);
}

void cpu() {
    FILE *f;
    f = fopen("/proc/interrupts", "r");
    if (f == NULL)
        return;
    char *words[10000];
    int n = 0;
    //perror("ff");
    if (f != NULL) {
        words[n] = malloc(size_buff);
        size_t s = size_buff;
        while (fscanf(f, "%s ", words[n]) != -1) {
            words[++n] = malloc(size_buff);
        }
    }
    // get cpu cores
    for (int i = 0; i < n; i++) {
        if (words[i][0] == '1') {
            for (int j = i + 1;; j++) {
                if (!isdigit(words[j][0])) {
                    break;
                } else {
                    char a[100];
                    sprintf(a, "%10s ", words[j]);
                    write(2, a, strlen(a));
                }
            }
            break;
        }
    }
    write(2, "\n", 2);


}

void new_born() {
    FILE *f = fopen("/proc/loadavg", "r");
    if (f == NULL) {
        perror("not found");
    }
    char no[100];
    for (int i = 0; i < 5; i++) {
        fscanf(f, "%s\n", no);
    }
    write(1, no, strlen(no));
    fclose(f);
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
    if (strcmp(tokens[2], "interrupt") == 0) {
        cpu_header();
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
}

// nightswatch -1 interrupt
// nightswatch -1 newborn


