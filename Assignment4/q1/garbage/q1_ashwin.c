# define _POSIX_C_SOURCE 199309L //required for clock
# include <sys/types.h>
# include <sys/ipc.h>
# include <sys/shm.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/wait.h>
# include <fcntl.h>
# include <time.h>
# include <pthread.h>
# define GREEN "\033[0;32m"
# define RESET "\033[m"


// ------------------- GLOBAL STRUCTURES -------------------
typedef struct arrayInfo {
    int lb;
    int ub;
    int* arr;
} arrayInfo;


// ------------------- HELPER FUNCTIONS -------------------
int * shareMem(size_t size){
    key_t mem_key = IPC_PRIVATE;
    int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
    return (int*)shmat(shm_id, NULL, 0);
}

void printArray(const int* arr, int n)
{
    for(int i=0; i<n; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

long double getTime(struct timespec ts)
{
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_nsec/(1e9) + ts.tv_sec;
}


// ------------------- SELECTION SORT IMPLEMENTATION -------------------
void selectionSort(int* arr, int lb, int ub)
{
    int min, pos;
    for(int i=lb; i<=ub-1; i++)
    {
        min = arr[i], pos = i;
        for(int j=i+1; j<=ub; j++)
        {
            if(arr[j] < min)
                pos = j, min = arr[j];
        }
        arr[pos] = arr[i];
        arr[i] = min;
    }
}


// ------------------- MERGESORT IMPLEMENTATIONS -------------------
void merge(int* arr, int lb, int mid, int ub)
{
    int len1 = mid-lb+1, len2 = ub-mid;
    int arr1[len1], arr2[len2];
    int i, j, k = lb;
    for(i=0; i<len1; i++, k++)
        arr1[i] = arr[k];
    for(j=0; j<len2; j++, k++)
        arr2[j] = arr[k];
    for(i=0, j=0, k=lb; (i<len1 && j<len2); k++)
    {
        if(arr1[i] < arr2[j])
            arr[k] = arr1[i], i++;
        else
            arr[k] = arr2[j], j++;
    }
    for(int x=i; x<len1; x++, k++)
        arr[k] = arr1[x];
    for(int x=j; x<len2; x++, k++)
        arr[k] = arr2[x];
}

void normalMergeSort(int* arr, int lb, int ub)
{
    if(ub-lb >= 5)
    {
        int mid = lb + (ub-lb)/2;
        normalMergeSort(arr, lb, mid);
        normalMergeSort(arr, mid+1, ub);
        merge(arr, lb, mid, ub);
    }
    else
        selectionSort(arr, lb, ub);
}

void concurrentMergeSort(int* arr, int lb, int ub)
{
    if(ub-lb >= 5)
    {
        int mid = lb + (ub-lb)/2;
        int pid1, pid2;
        pid1 = fork();
        if(pid1 == 0)
        {
            concurrentMergeSort(arr, lb, mid);
            exit(1);
        }
        else
        {
            pid2 = fork();
            if(pid2 == 0)
            {
                concurrentMergeSort(arr, mid+1, ub);
                exit(1);
            }
            else
            {
                waitpid(pid1, NULL, 0);
                waitpid(pid2, NULL, 0);
                merge(arr, lb, mid, ub);
            }
        }
    }
    else
        selectionSort(arr, lb, ub);
}

void* threadedMergeSort(void* input)
{
    arrayInfo* ai = (arrayInfo*)input;
    int lb = ai->lb;
    int ub = ai->ub;
    int* arr = ai->arr;

    if(ub-lb >= 5)
    {
        int mid = lb + (ub-lb)/2;
        pthread_t tid1, tid2;
        arrayInfo ai1 = {lb, mid, arr};
        arrayInfo ai2 = {mid+1, ub, arr};

        pthread_create(&tid1, NULL, threadedMergeSort, (void*)(&ai1));
        pthread_create(&tid2, NULL, threadedMergeSort, (void*)(&ai2));
        pthread_join(tid1, NULL);
        pthread_join(tid2, NULL);

        merge(arr, lb, mid, ub);
    }
    else
        selectionSort(arr, lb, ub);

    return NULL;
}


// ------------------- RUN MERGESORTS  -------------------
void runMergeSorts(int n)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long double start_time, t1, t2, t3;

    int *arr = shareMem(sizeof(int)*(n+1));
    for(int i=0;i<n;i++)
        scanf("%d", &arr[i]);

    int arr_copy1[n], arr_copy2[n];
    for(int i=0;i<n;i++)
        arr_copy1[i] = arr_copy2[i] = arr[i];

    // concurrent mergesort
    printf("Running concurrent mergesort\n");
    start_time = getTime(ts);

    concurrentMergeSort(arr, 0, n-1);

    t1 = getTime(ts) - start_time;
    printArray(arr, n);
    printf(GREEN "Time taken by concurrent mergesort = %Lf\n\n" RESET, t1);

    //multi-threaded mergesort
    pthread_t tid;
    arrayInfo ai = {0, n-1, arr_copy1};
    printf("Running multi-threaded mergesort\n");
    start_time = getTime(ts);

    pthread_create(&tid, NULL, threadedMergeSort, (void*)(&ai));
    pthread_join(tid, NULL);

    t2 = getTime(ts) - start_time;
    //printArray(arr_copy1, n);
    printf(GREEN "Time taken by multi-threaded mergesort = %Lf\n\n" RESET, t2);

    // normal mergesort
    printf("Running normal mergesort\n");
    start_time = getTime(ts);

    normalMergeSort(arr_copy2, 0, n-1);

    t3 = getTime(ts) - start_time;
    //printArray(arr_copy2, n);
    printf(GREEN "Time taken by normal mergesort = %Lf\n\n" RESET, t3);

    // compare implementations
    printf(GREEN "Normal mergesort ran [ %Lf ] times faster than concurrent mergesort\n" RESET, t1 / t3);
    printf(GREEN "Normal mergesort ran [ %Lf ] times faster than multi-threaded mergesort\n" RESET, t2 / t3);
    shmdt(arr);
}

int main()
{
    int n;
    scanf("%d", &n);
    runMergeSorts(n);
    return 0;
}
