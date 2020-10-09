#include<stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>

#define READ_END  0
#define WRITE_END 1

void printArray(int *a, int n) {
    for (int i = 0; i < n; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
}

void selectionSort(int *a, int n, int l) {
    for (int i = l; i < n + l; i++) {
        int min = i;
        for (int j = i + 1; j < n + l; j++) {
            if (a[j] < a[min]) {
                min = j;
            }
        }
        int temp = a[i];
        a[i] = a[min];
        a[min] = temp;
    }
}


void merge(int a[], int l, int m, int r) {
    //printf("*%d %d %d*\n", l ,m , r);
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

void mergeSort(int *a, int l, int r, int n) {
    if (r - l + 1 < 5) {
        selectionSort(a, r - l + 1, l);
    } else if (l < r) {
        int m = (l + r) / 2;
        int pipo[2];
        if(pipe(pipo) == 1){
            perror("pipe opening failed");
            _exit(1);
        }
        int child_pid = fork();
        if(child_pid < 0){
           perror("forking failed");
           _exit(1);
        }
        if (child_pid == 0) {
            close(pipo[READ_END]);
            mergeSort(a, l, m, n);
            int t = write(pipo[WRITE_END], a, n * sizeof(int));
            assert(t == 4 * n);
            _exit(0);
        }
        close(pipo[1]);
        int pipoo[2];
        if(pipe(pipoo) == -1){
            perror("pipe opening failed");
            _exit(1);
        }
        int child_pid_2 = fork();
        if(child_pid_2 < 0){
            perror("forking failed");
            _exit(1);
        }
        if (child_pid_2 == 0) {
            close(pipoo[READ_END]);
            mergeSort(a, m + 1, r, n);
            int t = write(pipoo[WRITE_END], a, n * sizeof(int));
            assert(t == 4 * n);
            _exit(0);
        }
        int status;
        waitpid(child_pid, &status, 0);
        if(WEXITSTATUS(status) == 1){
            perror("child exited abnormally");
           _exit(1);
        }
        int b[n];
        int t = read(pipo[READ_END], b, n * sizeof(int));
        assert(t == 4 * n);
        close(pipo[0]);
        waitpid(child_pid_2, &status, 0);
        if(WEXITSTATUS(status) == 1){
            perror("child exited abnormally");
            _exit(1);
        }
        int c[n];
        t = read(pipoo[READ_END], c, n * sizeof(int));
        assert(t == 4 * n);
        close(pipoo[0]);
        for (int i = l; i <= m; i++) {
            a[i] = b[i];
        }
        for (int j = m + 1; j <= r; j++) {
            a[j] = c[j];
        }
        merge(a, l, m, r);
    }
}


int main() {
    int n;
    scanf("%d", &n);
    int a[n];
    for (int i = 0; i < n; i++) {
        scanf("%d", &a[i]);
    }
    mergeSort(a, 0, n - 1, n);
    for (int i = 0; i < n; i++) {
        printf("%d\n", a[i]);
    }
}