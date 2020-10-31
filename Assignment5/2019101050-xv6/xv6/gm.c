
#include "types.h"
#include "user.h"

int number_of_processes = 15;

int main(int argc, char *argv[]) {
    //int parent_pid = getpid();
    for (int pNo = 0; pNo < number_of_processes; pNo++) {
        int pid = fork();
        if (pid < 0) {
            printf(1, "Fork failed\n");
            continue;
        }
        if (pid == 0) {
            int total = 0;
            // CPU
            if (pNo % 2 == 0)
                for (int i = 0; i < 1e9; i++) {
                    if(i % ((int) 1e8) == 0)
                        sleep(25);
                    total += i;
                }
            else {
                for (int i = 0; i < 1000000; i++) {
                    sleep(50);
                    total += i;
                }
            }
            printf(1, "%d total", total);
            exit();
        } else {
            set_priority(100 - (20 + pNo) % 2,
                         pid); // will only matter for PBS, comment it out if not implemented yet (better priorty for more IO intensive jobs)
        }
    }

    for (int j = 0; j < number_of_processes + 5; j++) {
        wait();
    }
    exit();
}
