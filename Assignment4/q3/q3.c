#include<stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
int acousticStages, electricStages;
int numberMusician;
int tMin, tMax;
int cordCount;
int waitTime;

int main(){
    scanf("%d %d %d %d %d %d %d",
            &numberMusician,
            &acousticStages,
            &electricStages,
            &cordCount,
            &tMin,
            &tMax,
            &waitTime);


}