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
        /* if elements less than 5 then use selection sort */
        selectionSort(a, r - l + 1, l);
    } else if (l < r) {
        int m = (l + r) / 2;
        int size_left = m - l + 1;
        int size_right = r - m;
        /* Init first pipe only in parent */
        int pipeL[2];
        if (pipe(pipeL) == 1) {
            perror("pipe opening failed");
            _exit(1);
        }
        /* fork a child process, this child mergeSorts the left half */
        int child_pid = fork();
        if (child_pid < 0) {
            perror("forking failed");
            _exit(1);
        }
        /* inside first child process */
        if (child_pid == 0) {
            /* child only writes */
            close(pipeL[READ_END]);

            mergeSort(a, l, m, n);

            /* mergeSort the left half */
            int t = write(pipeL[WRITE_END], a + l, size_left * sizeof(int));
            assert(t == 4 * size_left);
            _exit(0);
        }
        /* in parent now */
        /* parent only writes */
        close(pipeL[WRITE_END]);
        /* init pipe in parent for right half */
        int pipeR[2];
        if (pipe(pipeR) == -1) {
            perror("pipe opening failed");
            _exit(1);
        }
        /* fork another process */
        int child_pid_2 = fork();
        if (child_pid_2 < 0) {
            perror("forking failed");
            _exit(1);
        }
        /* inside 2nd child process */
        if (child_pid_2 == 0) {
            /* child only writes */
            close(pipeR[READ_END]);
            /* mergeSort the right half */
            mergeSort(a, m + 1, r, n);
            int t = write(pipeR[WRITE_END], a + m + 1, size_right * sizeof(int));
            assert(t == 4 * size_right);
            _exit(0);
        }

        /* this part of code reaps the child processes and gets data back */
        for (int k = 0; k < 2; k++) {
            int status;
            /* wait for a child process to finish and store its pid in kill_id*/
            int kill_id = waitpid(-1, &status, 0);
            /* make sure the child exits normally */
            if (WEXITSTATUS(status) == 1) {
                perror("child exited abnormally");
                _exit(1);
            }
            /* returned_data stores the read data */
            int returned_data[n];
            /* check if the child returned is the first or second child */
            if (kill_id == child_pid) {
                /* read data for left half */
                int bytes_read = read(pipeL[READ_END], returned_data, size_left * sizeof(int));
                assert(bytes_read == 4 * size_left);
                close(pipeL[0]);
                /* change data in original array */
                for (int i = 0; i < size_left; i++) {
                    a[l + i] = returned_data[i];
                }
            } else if (kill_id == child_pid_2){
                /* read data for right half */
                int bytes_read = read(pipeR[READ_END], returned_data, size_right * sizeof(int));
                assert(bytes_read == 4 * size_right);
                close(pipeR[0]);
                /* change data in original array */
                for (int j = 0; j < size_right; j++) {
                    a[m + 1 + j] = returned_data[j];
                }
            }
        }
        /* merge the two halves */
        merge(a, l, m, r);
    }
}


int main() {
    /* take input */
    int n;
    scanf("%d", &n);
    int a[n];
    for (int i = 0; i < n; i++) {
        scanf("%d", &a[i]);
    }
    /* start mergeSort */
    mergeSort(a, 0, n - 1, n);
    /* print final array */
    for (int i = 0; i < n; i++) {
        printf("%d\n", a[i]);
    }
}