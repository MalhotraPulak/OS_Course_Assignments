// Simple grep.  Only supports ^ . * $ operators.

#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
    if(argc < 2){
        printf(2, "fuck");
        exit();
    }
    printf(2, "%s %s", argv[0], argv[1]);
    int rc = fork();
    if(rc == 0){
        exec(argv[1], argv + 1);
    }
    else {
        int wtime, rtime;
        printf(1, "here");
        waitx(&wtime, &rtime);
        printf(1, "Wait time is %d\n", wtime);
        printf(1, "Run time is %d\n", rtime);
    }
    exit();
}
