#define _POSIX_C_SOURCE 199309L //required for clock

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>
#define T_NOW ts.tv_nsec / (1e9) + ts.tv_sec;
int *shareMem(size_t size) {
    key_t memKey = IPC_PRIVATE;
    int shmId = shmget(memKey, size, IPC_CREAT | 0666);
    return (int *) shmat(shmId, NULL, 0);
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

void mergeSort(int *a, int l, int r) {
    if (r - l + 1 < 5) {
        selectionSort(a, r - l + 1, l);
    } else if (l < r) {
        int m = (l + r) / 2;
        mergeSort(a, l, m);
        mergeSort(a, m + 1, r);
        merge(a, l, m, r);
    }
}

void concurrentMergeSort(int *a, int l, int r) {
    if (r - l + 1 < 5) {
        selectionSort(a, r - l + 1, l);
    } else if (l < r) {
        int m = (l + r) / 2;
        int leftChild = fork();
        if (leftChild < 0) {
            perror("forking failed");
            _exit(1);
        }
        if (leftChild == 0) {
            mergeSort(a, l, m);
            _exit(0);
        }
        int rightChild = fork();
        if (rightChild < 0) {
            perror("forking failed");
            _exit(1);
        }
        if (rightChild == 0) {
            mergeSort(a, m + 1, r);
        }
        waitpid(leftChild, NULL, 0);
        waitpid(rightChild, NULL, 0);
        merge(a, l, m, r);
    }
}

struct arg {
    int l;
    int r;
    int *arr;
};

void *threadedMergeSort(void *arg) {
    struct arg *k = arg;
    int r = k->r;
    int l = k->l;
    int *a = k->arr;
    if (r - l + 1 < 5) {
        selectionSort(a, r - l + 1, l);
    } else if (l < r) {
        int m = (l + r) / 2;
        struct arg k2;
        k2.l = l;
        k2.r = m;
        k2.arr = a;
        pthread_t left;
        pthread_create(&left, NULL, threadedMergeSort, &k2);
        struct arg k3;
        k3.l = m + 1;
        k2.r = r;
        k2.arr = a;
        pthread_t right;
        pthread_create(&right, NULL, threadedMergeSort, &k3);
        pthread_join(left, NULL);
        pthread_join(right, NULL);
        merge(a, l, m, r);
    }
}
long double getTime (){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return T_NOW;
}
void runSorts(long long int n) {

    struct timespec ts;
    long double t1, t2, t3, start, end;
    //getting shared memory
    int *arr = shareMem(sizeof(int) * (n + 1));
    for (int i = 0; i < n; i++)
        scanf("%d", arr + i);

    int brr[n + 1];
   /* for (int i = 0; i < n; i++)
        brr[i] = arr[i];*/


    // MULTI PROCESS
    printf("Running concurrent merge sort for n = %lld\n", n);
    start = getTime();
    concurrentMergeSort(arr, 0, n - 1);
    end  = getTime();
    /*for (int i = 0; i < n; i++) {
     printf("%d ", arr[i]);
    }
    printf("\n");*/
    t1 = end - start;
    printf("time = %Lf\n", t1);
    // MULTI THREADED
    pthread_t tid;
    struct arg a;
    a.l = 0;
    a.r = n - 1;
    a.arr = brr;
    printf("Running threaded mergeSort for n = %lld\n", n);
    start = getTime();
    // remove thread here not required
    pthread_create(&tid, NULL, threadedMergeSort, &a);
    pthread_join(tid, NULL);
    /*for (int i = 0; i < n; i++) {
        printf("%d ", a.arr[i]);
    }
    printf("\n");
     */
    end = getTime();
    t2 = end - start;
    printf("time = %Lf\n", t2);


    // NORMAL MERGESORT
    printf("Running normal_quicksort for n = %lld\n", n);
    start = getTime();
    mergeSort(brr, 0, n - 1);
   /* for (int i = 0; i < n; i++) {
        printf("%d ", brr[i]);
    }*/
    end = getTime();
    t3 = end - start;
    printf("time = %Lf\n", t3);
    printf("normal mergeSort ran:\n"
           "\t[ %Lf ] times faster than concurrent mergeSort\n"
           "\t[ %Lf ] times faster than thread mergeSort\n\n\n",
           t1 / t3, t2 / t3);
    shmdt(arr);
}

int main() {

    long long int n;
    scanf("%lld", &n);
    runSorts(n);
    return 0;
}
