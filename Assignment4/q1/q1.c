#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <pthread.h>

#define T_NOW ts.tv_nsec / (1e9) + ts.tv_sec;


/* wrapper function for pthread join*/
void pthreadJoin(pthread_t thread, void **retval) {
    if (pthread_join(thread, retval) != 0) {
        perror("Thread could not join");
    }

}
/* helper function to print arrays and get time */
void printArr(int *a, int n) {
    for (int i = 0; i < n; i++)
        printf("%d ", a[i]);
    printf("\n");

}
long double getTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return T_NOW;
}



/* Shared memory for multi process mergeSort */
int *shareMem(size_t size) {
    key_t memKey = IPC_PRIVATE;
    int shmId = shmget(memKey, size, IPC_CREAT | 0666);
    return (int *) shmat(shmId, NULL, 0);
}

/* Implementation of Selection Sort */
void selectionSort(int *a, int total, int start) {
    //printf("^ %d \n", total);
    for (int i = start; i < total + start; i++) {
        int min = i;
        for (int j = i + 1; j < total + start; j++) {
            if (a[j] < a[min]) {
                min = j;
            }
        }
        int temp = a[i];
        a[i] = a[min];
        a[min] = temp;
    }
}

/* *************************** MERGE SORTS STARTS **************************************** */
/* standard merge function */
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

/* single thread mergeSort implementation */
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

/* uses multiple processes to mergeSort */
void concurrentMergeSort(int *a, int l, int r) {
    //printf("%d %d \n", l, r);
    if (r - l + 1 < 5) {
        selectionSort(a, r - l + 1, l);
    } else if (l < r) {
        int m = (l + r) / 2;
        int leftChild = fork();
        if (leftChild < 0) {
            perror("forking failed switching to normal mergeSort");
            mergeSort(a, l, m);
            _exit(1);
        }
        if (leftChild == 0) {
            //printf("there");
            concurrentMergeSort(a, l, m);
            //printf("layer");
            _exit(0);
        }
        int rightChild = fork();
        if (rightChild < 0) {
            perror("forking failed switching to normal mergeSort");
            mergeSort(a, l, m);
            _exit(1);
        }
        if (rightChild == 0) {
            concurrentMergeSort(a, m + 1, r);
            _exit(0);
        }
        waitpid(leftChild, NULL, 0);
        waitpid(rightChild, NULL, 0);
        merge(a, l, m, r);
    }
}
/* struct which serves argument to threads */
struct arg {
    int l;
    int r;
    int *arr;
};

/* uses multiple threads */
void *threadedMergeSort(void *arg) {
    struct arg *args = arg;
    int r = args->r;
    int l = args->l;
    int *a = args->arr;
    //printf("%d %d \n", l, r);
    if(r - l + 1 < 5){
        selectionSort(a, r - l + 1, l);
    }
    else if (l < r) {
        int m = (l + r) / 2;
        /* create argument for left half mergeSort */
        struct arg args2;
        args2.l = l;
        args2.r = m;
        args2.arr = a;
        pthread_t left;
        /* create argument for right half mergeSort */
        struct arg args3;
        args3.l = m + 1;
        args3.r = r;
        args3.arr = a;
        pthread_t right;
        /* create the threads */
        if(pthread_create(&left, NULL, threadedMergeSort, &args2) != 0){
           perror("Thread creation failed switching to normal mergesort");
           mergeSort(a, l, m);
        }
        if(pthread_create(&right, NULL, threadedMergeSort, &args2) != 0){
            perror("Thread creation failed switching to normal mergesort");
            mergeSort(a, m + 1, r);
        }
        /* wait for them to join */
        pthreadJoin(left, NULL);
        pthreadJoin(right, NULL);
        /* merge as usual */
        merge(a, l, m, r);
    }
    return NULL;
}

/* ****************************** MERGE SORTS END ************************************ */

long double runMultiProcess(int n, const int *a) {
    printf("Running multiprocess mergeSort for n = %d\n", n);
    long double start, end;
    /* allocate b in shared memory */
    int *b = shareMem(sizeof(int) * (n + 1));
    /* copy values into a new array b */
    for (int i = 0; i < n; i++) {
        b[i] = a[i];
    }
    start = getTime();
    concurrentMergeSort(b, 0, n - 1);
    end = getTime();
    printArr(b, n);
    /* free up the shared memory */
    shmdt(b);
    return end - start;

}

long double runMultiThread(int n, const int *a) {
    printf("Running threaded mergeSort for n = %d\n", n);
    long double start, end;
    int b[n];
    /* copy values into a new array b */
    for (int i = 0; i < n; i++) {
        b[i] = a[i];
    }
    /* passing argument in thread-like manner */
    struct arg argInit;
    argInit.l = 0;
    argInit.r = n - 1;
    argInit.arr = b;
    start = getTime();
    threadedMergeSort(&argInit);
    end = getTime();
    printArr(b, n);
    return end - start;
}

long double runNormal(int n, const int *a) {
    long double start, end;
    printf("Running normal mergeSort for n = %d\n", n);
    int b[n];
    /* copy values into a new array b */
    for (int i = 0; i < n; i++)
        b[i] = a[i];
    start = getTime();
    mergeSort(b, 0, n - 1);
    end = getTime();
    printArr(b, n);
    return end - start;
}

/* runs different algorithms one by one */
void runSorts(int n, const int *a) {
    /* a is constant and never changes */
    long double time1 = runMultiProcess(n, a);
    long double time2 = runMultiThread(n, a);
    long double time3 = runNormal(n, a);

    printf("normal mergeSort ran:\n"
           "\t[ %Lf ] times faster than concurrent mergeSort\n"
           "\t[ %Lf ] times faster than threaded mergeSort\n",
           time1 / time3, time2 / time3);
}

int main() {
    int n;
    scanf("%d", &n);
    int a[n];
    for (int i = 0; i < n; i++) {
        scanf("%d", &a[i]);
    }
    runSorts(n, a);
    return 0;
}
