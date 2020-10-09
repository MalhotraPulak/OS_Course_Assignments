#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#define READ_END  0
#define WRITE_END 1

void printArray(int *a, int n){
    for(int i = 0; i < n; i++){
        printf("%d ", a[i]);
    }
    printf("\n");
}


void tt(int a[]) {
        int pipo[2];
        pipe(pipo);
        if(pipe(pipo) == -1){
            fprintf(stderr, "fml");
        }
        int child_pid = fork();
        if(child_pid < 0){
            fprintf(stderr, "fml");
        }
        if(child_pid == 0){
            // in child
            close(pipo[READ_END]);
            int buf[] = {1, 2 ,3, 5};
            int t = write(pipo[1], buf, sizeof(buf));
            fprintf(stdout, "written %d\n", t);
            _exit(0);
        }
        else if (child_pid > 0) {
            // in parent

            close(pipo[WRITE_END]);
            waitpid(child_pid, NULL, 0);
            fprintf(stderr, "after wait\n");
            int b[1000];
            int t = read(pipo[READ_END], b, sizeof(b));
            printf("read %d", t);
            printArray(b, t / 4);
        }
}


int main() {
    int n = 6;
    int a[6] = {400, 1, 1, 2000, -1, -24};
    tt(a);

}