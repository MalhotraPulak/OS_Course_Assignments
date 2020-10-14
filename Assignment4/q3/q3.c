#include<stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>

#define NAME_SIZE 100
#define QUEUE_SIZE 500
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"
#define ACOUSTIC 1
#define ELECTRIC 2
#define ACOUSTIC_ELECTRIC 3

/* Wrapper functions and Helper functions */
int randNum(long int lower, long int upper) {
    return (int) (random() % (upper - lower + 1) + lower);
}

int min(int a, int b) {
    return (a < b) ? a : b;
}

void pthreadCreate(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg) {
    if (pthread_create(thread, attr,
                       start_routine, arg) != 0) {
        perror("Thread was not created");
    }
}

void pthreadJoin(pthread_t thread, void **retval) {
    if (pthread_join(thread, retval) != 0) {
        perror("Thread could not join");
    }

}

void condWait(pthread_cond_t *cv, pthread_mutex_t *mutex) {
    if (pthread_cond_wait(cv, mutex) != 0) {
        perror("Cant cond wait on cv");
    }
}

void mutexLock(pthread_mutex_t *mutex) {
    if (pthread_mutex_lock(mutex) != 0) {
        perror("Cant lock mutex");
    }
}

void mutexUnlock(pthread_mutex_t *mutex) {
    if (pthread_mutex_unlock(mutex) != 0) {
        perror("Cant lock mutex");
    }
}

/*
void condBroadcast(pthread_cond_t *cv) {
    if (pthread_cond_broadcast(cv) != 0) {
        perror("Cant broadcast");
    }
}*/
void condSignal(pthread_cond_t *cv) {
    if (pthread_cond_signal(cv) != 0) {
        perror("Cant signal");
    }
}

/* global variables */
int numAcoustic, numElectric;
int numberMusician;
int tMin, tMax;
int cordCount;
int waitTime;
int aStage, eStage;
int performanceNoSigner = 0;
char singersQueue[QUEUE_SIZE][NAME_SIZE];
int QReadPos, QWritePos;
/* semaphores */
/* mutex */
pthread_mutex_t stageLock;

/* CV */
pthread_cond_t noAStage = PTHREAD_COND_INITIALIZER;
pthread_cond_t noEStage = PTHREAD_COND_INITIALIZER;
pthread_cond_t anyStage = PTHREAD_COND_INITIALIZER;
pthread_cond_t anySinger = PTHREAD_COND_INITIALIZER;
pthread_cond_t stageOrPerf = PTHREAD_COND_INITIALIZER;
struct musicianData {
    char iCode;
    int time;
    char *name;
    bool singer;
    int stage;
};

struct timespec getTime(int timeInMs) {
    struct timeval tv;
    struct timespec ts;
    gettimeofday(&tv, NULL);
    ts.tv_sec = time(NULL) + timeInMs / 1000;
    ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (timeInMs % 1000);
    ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
    ts.tv_nsec %= (1000 * 1000 * 1000);
    return ts;
}

void perform(struct musicianData *data, int time) {
    //mutexLock(&stageLock);
    struct timespec tt = getTime(time * 1000);
    int rc = pthread_cond_timedwait(&anySinger, &stageLock, &tt);
    if (rc != ETIMEDOUT) {
        char name[NAME_SIZE];
        strcpy(name, singersQueue[QReadPos++]);
        printf(RED "%s joined %s performance, performance extended by 2 secs\n" RESET,
               name, data->name
        );
        performanceNoSigner--;
        mutexUnlock(&stageLock);
        sleep(2);
        mutexLock(&stageLock);
    }
    //mutexUnlock(&stageLock);
}

void performanceStart(struct musicianData *data, int time, int stage) {
    if (stage == ACOUSTIC) {
        printf(YEL "%s will perform %c on Electric Stage for %d seconds\n" RESET,
               data->name,
               data->iCode,
               time);
        eStage--;
    } else if (stage == ELECTRIC) {
        printf(BLU"%s will perform %c on Acoustic Stage for %d seconds\n" RESET,
               data->name,
               data->iCode,
               time);
        aStage--;
    }
    condSignal(&stageOrPerf);
    performanceNoSigner++;
    perform(data, time);
    if (stage == ELECTRIC) {
        printf(MAG "%s performance on Electric Stage is finished\n" RESET,
               data->name);
        condSignal(&noEStage);
        eStage++;
    } else if(stage == ACOUSTIC) {
        printf(CYN "%s performance on Acoustic Stage is finished\n" RESET, data->name);
        condSignal(&noAStage);
        aStage++;
    }
    performanceNoSigner--;
    condSignal(&anyStage);
    condSignal(&stageOrPerf);
    mutexUnlock(&stageLock);

}

void waitForStage(pthread_cond_t *cv, pthread_mutex_t *lock, struct timespec *tt, struct musicianData *data) {
    int rc = pthread_cond_timedwait(cv, lock, tt);
    if (rc == ETIMEDOUT) {
        printf(RED "%s %c left due to impatience\n" RESET,
               data->name,
               data->iCode);
        mutexUnlock(lock);
        pthread_exit(NULL);
    }
}

void performAcoustic(struct musicianData *data) {
    int time = randNum(tMin, tMax);
    mutexLock(&stageLock);
    struct timespec tt = getTime(waitTime * 1000);
    while (aStage <= 0) {
        waitForStage(&noAStage, &stageLock, &tt, data);
    }
/*    aStage--;
    printf(BLU "%s will perform %c on Acoustic Stage for %d seconds\n" RESET,
           data->name,
           data->iCode,
           time)*/;
    /*  //mutexUnlock(&stageLock);
      perform(data, time);
      //mutexLock(&stageLock);
      printf(CYN "%s performance %c on Acoustic Stage is finished\n" RESET,
             data->name,
             data->iCode);
      condSignal(&noAStage);
      condSignal(&anyStage);
      aStage++;
      mutexUnlock(&stageLock);*/
    performanceStart(data, time, ACOUSTIC);
}


void performElectric(struct musicianData *data) {
    int time = randNum(tMin, tMax);
    mutexLock(&stageLock);
    struct timespec tt = getTime(waitTime * 1000);
    while (eStage <= 0) {
        waitForStage(&noEStage, &stageLock, &tt, data);
    }
/*    eStage--;
    printf(YEL "%s will perform %c on Electric Stage for %d seconds\n" RESET,
           data->name,
           data->iCode,
           time);*/
/*    //mutexUnlock(&stageLock);
    perform(data, time);
    //mutexLock(&stageLock);
    printf(MAG "%s performance on Electric Stage is finished\n" RESET, data->name);
    condSignal(&noEStage);
    condSignal(&anyStage);
    eStage++;
    mutexUnlock(&stageLock);*/
    performanceStart(data, time, ELECTRIC);
}


void performBoth(struct musicianData *data) {
    int time = randNum(tMin, tMax);
    mutexLock(&stageLock);
    struct timespec tt = getTime(waitTime * 1000);
    while (eStage + aStage <= 0) {
        waitForStage(&anyStage, &stageLock, &tt, data);
    }
    if(eStage > 0 && aStage > 0){
        if(randNum(1,2) == 1){
            performanceStart(data, time, ELECTRIC);
        } else {
            performanceStart(data, time, ACOUSTIC);
        }
    } else if(eStage > 0){
        performanceStart(data, time, ELECTRIC);
    } else {
        performanceStart(data, time, ACOUSTIC);
    }
    /*if (eStage > 0) {
        printf(YEL "%s will perform %c on Electric Stage for %d seconds\n" RESET,
               data->name,
               data->iCode,
               time);
        eStage--;
        stage = ELECTRIC;
    } else {
        printf(BLU"%s will perform %c on Acoustic Stage for %d seconds\n" RESET,
               data->name,
               data->iCode,
               time);
        aStage--;
        stage = ACOUSTIC;
    }*/
    /*//mutexUnlock(&stageLock);
    perform(data, time);
    //mutexLock(&stageLock);
    if (stage == ELECTRIC) {
        printf(MAG "%s performance on Electric Stage is finished\n" RESET,
               data->name);
        condSignal(&noEStage);
        eStage++;
    } else {
        printf(CYN "%s performance on Acoustic Stage is finished\n" RESET, data->name);
        condSignal(&noAStage);
        aStage++;
    }
    condSignal(&anyStage);
    mutexUnlock(&stageLock);*/
    //performanceStart(data, time, stage);
}

void joinAMusician(struct musicianData *data) {
    strcpy(singersQueue[QWritePos++], data->name);
    condSignal(&anySinger);
    mutexUnlock(&stageLock);
    pthread_exit(NULL);
}


void goOnStage(struct musicianData *data) {
    int time = randNum(tMin, tMax);
    if(eStage > 0 && aStage > 0){
        if(randNum(1,2) == 1){
            performanceStart(data, time, ELECTRIC);
        } else {
            performanceStart(data, time, ACOUSTIC);
        }
    } else if(eStage > 0){
        performanceStart(data, time, ELECTRIC);
    } else {
        performanceStart(data, time, ACOUSTIC);
    }
/*    //mutexUnlock(&stageLock);
    perform(data, time);
    //mutexLock(&stageLock);
    if (stage == ELECTRIC) {
        printf(MAG "%s performance on Electric Stage is finished\n" RESET,
               data->name);
        condSignal(&noEStage);
        eStage++;
    } else {
        printf(CYN "%s performance on Acoustic Stage is finished\n" RESET, data->name);
        condSignal(&noAStage);
        aStage++;
    }
    condSignal(&anyStage);
    mutexUnlock(&stageLock);*/
    //performanceStart(data, time, stage);
}


void singersCase(struct musicianData *data) {
    mutexLock(&stageLock);
    while ((eStage + aStage <= 0) && performanceNoSigner <= 0) {
        condWait(&stageOrPerf, &stageLock);
    }
    if ((eStage > 0 || aStage > 0) && performanceNoSigner > 0) {
        int n = randNum(1, 2);
        if (n == 1) {
            joinAMusician(data);
        } else {
            goOnStage(data);
        }
    } else if (performanceNoSigner >= 0) {
        joinAMusician(data);
    } else {
        goOnStage(data);
    }
}

void *musician(void *arg) {
    struct musicianData *data = arg;
    sleep(data->time);
    printf(GRN "%s %c has arrived\n" RESET, data->name, data->iCode);
    int stage = data->stage;
    if (stage == ACOUSTIC) {
        performAcoustic(data);
    } else if (stage == ELECTRIC) {
        performElectric(data);
    } else {
        if (data->singer) {
            singersCase(data);
        } else {
            performBoth(data);
        }
    }
}

int main() {
    srandom(time(0));
    scanf("%d %d %d %d %d %d %d",
          &numberMusician,
          &numAcoustic,
          &numElectric,
          &cordCount,
          &tMin,
          &tMax,
          &waitTime);
    char *names[numberMusician];
    char iCode[numberMusician];
    int time[numberMusician];
    struct musicianData musicianArgs[numberMusician];
    for (int i = 0; i < numberMusician; i++) {
        names[i] = malloc(NAME_SIZE);
        scanf("%s ", names[i]);
        scanf("%c ", &iCode[i]);
        scanf("%d ", &time[i]);
    }
    aStage = numAcoustic;
    eStage = numElectric;
    pthread_t musicians[numberMusician];
    for (int i = 0; i < numberMusician; i++) {
        struct musicianData *mData = &musicianArgs[i];
        mData->iCode = iCode[i];
        mData->time = time[i];
        mData->name = names[i];
        if (iCode[i] == 'p' || iCode[i] == 'g' || iCode[i] == 's') {
            musicianArgs[i].stage = ACOUSTIC_ELECTRIC;
        } else if (iCode[i] == 'v') {
            musicianArgs[i].stage = ACOUSTIC;
        } else {
            musicianArgs[i].stage = ELECTRIC;
        }
        musicianArgs[i].singer = (iCode[i] == 's');
        pthreadCreate(&musicians[i], NULL, &musician, &musicianArgs[i]);
    }
    for (int i = 0; i < numberMusician; i++) {
        pthreadJoin(musicians[i], NULL);
    }
    printf(RED "Finished\n" RESET);
}

/*
 *
 *
Testcase 1:

Change first line of input to 5 1 1 4 2 10 5



Testcase 2:

Input:

1 1 1 4 2 7 1

Abhinav b 3



Output:

Abhinav b arrived -> at t~3

Abhinav performing b at electric stage for 7 secs -> at t~3

Abhinav performance at electric stage ended -> at t~10

Abhinav collecting t-shirt -> at t~10

Finished -> at t~12
 */