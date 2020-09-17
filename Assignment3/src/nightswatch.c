//
// Created by Pulak Malhotra on 02/09/20.
//
#include "headers.h"
#include "nightswatch.h"
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
    cfmakeraw(&new_termios); // only standard on BSD systems, flips the ICANON AND ECHO flag
    //   new_termios.c_iflag &= ~(ICRNL | IXON); //control key
    //   new_termios.c_oflag &= ~(OPOST); // \n -> \r turn off output processing
    //  new_termios.c_lflag &= ~(ECHO | ICANON); // turn off echo and canonical mode in terminal
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
            write(1, a, strlen(a));
        }
    }
    write(1, "\n", 2);
    fclose(f);
    for (int i = 0; i <= n; i++)
        free(words[i]);

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
        while (fscanf(f, " \t%s", words[n]) != -1) {
            words[++n] = malloc(size_buff);
        }
    }
    for (int i = 0; i < n; i++) {
        //printf("-%s-\n", words[i]);
        if (strcmp(words[i], "1:") == 0) {
            for (int j = i + 1;; j++) {
                if (!(words[j][0] >= '0' && words[j][0] <= '9')) {
                    break;
                } else {
                    char a[100];
                    sprintf(a, "%10s", words[j]);
                    write(1, a, strlen(a));
                }
            }
            break;
        }
    }
    fclose(f);
    write(1, "\n", 2);
    for (int i = 0; i <= n; i++)
        free(words[i]);


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
    char pr[1000];
    sprintf(pr, "%s\n", no);
    write(1, pr, strlen(pr));
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
    if (no != 4) {
        fprintf(stderr, "2 args required : -n  <int> <command>\n");
        return;
    }
    int seconds;
    if (strcmp(tokens[1], "-n") != 0) {
        printf("2 args required : -n  <int> <command>\n");

    }
    seconds = (int) strtol(tokens[2], NULL, 10);
    if (seconds <= 0) {
       fprintf(stderr, "n > 0\n");
    }
    //printf("%d", seconds);
    char *func = tokens[3];
    if (strcmp(func, "interrupt") == 0) {
        cpu_header();
    }
    while (true) {
        if (strcmp(func, "interrupt") == 0) {
            cpu();
        } else if (strcmp(func, "newborn") == 0) {
            new_born();
        } else {
           fprintf(stderr, "nightswatch : invalid command \n");
            break;
        }
        set_terminal_raw_mode();
        time_t secs = seconds;
        time_t startTime = time(NULL);
        while (time(NULL) - startTime < secs) {
            if (kbhit()) {
                if (getch() == 'q') {
                    reset_terminal_mode_to_canon();
                    return;
                }
            }
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


