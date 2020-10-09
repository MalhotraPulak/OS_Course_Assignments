#include<stdio.h>
#include <unistd.h>
#include <sys/wait.h>

void print_a(int a[], int n){
    for(int i = 0; i < n; i++){
        printf("%d ", a[i]);
    }
    printf("\n");
}


void mergesort(int a[], int l, int r, int n) {
        //print_a(a, n);
        int child_pid = fork();
        int pipo[2];
        pipe(pipo);
        if(child_pid == 0){
            close(pipo[0]);// close read end
            int b[] = {1, 2, 3, 5, 6};
            write(pipo[1], b, sizeof(b));
            close(pipo[1]);
            _exit(0);
        }
        else if (child_pid > 0) {
            wait(NULL);
            close(pipo[1]); // close write end
            int b[1000];
            int t = read(pipo[0], b,sizeof(b));
            close(pipo[0]);
            print_a(b, t/4);
        }
}


int main() {
    int n = 6;
    int a[6] = {400, 1, 1, 2000, -1, -24};
    mergesort(a, 0, n - 1, n);

}