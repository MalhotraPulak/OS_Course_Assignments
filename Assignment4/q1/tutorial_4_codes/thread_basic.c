#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

typedef struct s{
    int a;
    int b;
    int sum;
} s;
void* sum(void* inp){
    s* inputs = (s*) inp;
    int a = inputs->a;
    int b = inputs->b;
    inputs->sum = a+b;
    sleep(3);
    return NULL;
}

int main(){
    pthread_t tid;
    s* operands = (s*)malloc(sizeof(s));
    operands->a = 1;
    operands->b = 2;

    pthread_create(&tid, NULL, sum, (void*)(operands));

    //join waits for the thread with id=tid to finish execution before proceeding
    pthread_join(tid, NULL);

    printf("%d\n", operands->sum);
    return 0;
}
