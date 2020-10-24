
#include "types.h"
#include "user.h"

int number_of_processes = 10;

int main(int argc, char *argv[]) {

    for (int j = 0; j < number_of_processes; j++) {
        int pid = fork();
        if (pid < 0) {
            printf(1, "Fork failed\n");
            continue;
        }
        if (pid == 0) {
            int total = 0;
            for(volatile int ff = 0; ff < 10; ff++) {
                for (volatile int i = 0; i < 100000000 >> (j + 3); i++) { ;

                }
            }
            printf(2, "im leaving %d %d\n", j, total);
            exit();
        } else {
//            set_priority(100-(20+j) % 2,pid); // will only matter for PBS, comment it out if not implemented yet (better priorty for more IO intensive jobs)
        }
    }

    for (int j = 0; j < number_of_processes + 5; j++) {
        wait();
    }
    exit();
}
