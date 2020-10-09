#include<stdio.h>
#include <unistd.h>
#include <sys/wait.h>

void print_a(int a[], int n){
    for(int i = 0; i < n; i++){
        printf("%d ", a[i]);
    }
    printf("\n");
}
void merge(int a[], int l, int m, int r) {
    //fprintf(stderr, "%d %d %d\n", l , m , r);
    int s_l = m - l + 1;
    int s_r = r - m;
    int left[s_l];
    int right[s_r];

    for (int i = 0; i < s_l; i++) {
        left[i] = a[l + i];
    }
    for (int i = 0; i < s_r; i++) {
        right[i] = a[m + i + 1];
    }

    int f = 0;
    int s = 0;
    int c = 0;
    while (f < s_l && s < s_r) {
        if (left[f] < right[s]) {
            a[c + l] = left[f];
            c++;
            f++;
        } else {
            a[c + l] = right[s];
            s++;
            c++;
        }
    }

    while (f < s_l) {
        a[c + l] = left[f];
        c++;
        f++;
    }

    while (s < s_r) {
        a[c + l] = right[s];
        c++;
        s++;
    }
}

void mergesort(int a[], int l, int r, int n) {
    if (l < r) {
        print_a(a, n);
        int m = (l + r) / 2;
        int child_pid = fork();
        int pipo[2];
        pipe(pipo);
        if(child_pid == 0){
            close(pipo[0]);
            mergesort(a, l, m, n);
            write(pipo[1], a, n * sizeof(int));
            close(pipo[1]);
            _exit(0);
        }
        close(pipo[1]);
        int pipoo[2];
        pipe(pipoo);
        int child_pid_2 = fork();
        if(child_pid_2 == 0){
            close(pipoo[0]);
            mergesort(a, l, m, n);
            write(pipoo[1], a, n * sizeof(int));
            close(pipoo[1]);
            _exit(0);
        }
        waitpid(child_pid, NULL, 0);
        int b[n];
        read(pipo[0], b, n * sizeof(int));
        close(pipo[0]);
        waitpid(child_pid_2, NULL, 0);
        int c[n];
        read(pipoo[0], c, n * sizeof(int));
        close(pipoo[0]);
        for(int i = l; i <= m; i++){
            a[i] = b[i];
        }
        for(int j = m + 1; j <= r; j++){
            a[j] = c[j];
        }
        merge(a, l, m, r);
    }
}


int main() {
    int n = 6;
    int a[6] = {400, 1, 1, 2000, -1, -24};
    mergesort(a, 0, n - 1, n);
    for(int i = 0; i < n; i++){
        printf("%d ", a[i]);
    }
}