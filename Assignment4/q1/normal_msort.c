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
        mergeSort(a, l, m, n);
        mergeSort(a, m + 1, r, n);
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


