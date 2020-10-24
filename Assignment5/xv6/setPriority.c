#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[]) {
    if (argc < 3) {
        printf(2, "Pass a new priority and pid\n");
        exit();
    }
    int t = set_priority(atoi(argv[1]), atoi(argv[2]));
    printf(1, "PRIORITY HAS BEEN SET from %d\n", t);
    exit();
}
