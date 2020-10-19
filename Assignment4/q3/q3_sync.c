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
#include <fcntl.h>

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
        perror("Cant unlock mutex");
    }
}

void condSignal(pthread_cond_t *cv) {
    if (pthread_cond_signal(cv) != 0) {
        perror("Cant signal");
    }
}



void semDestroy(sem_t *sm){
    char a[10] = "hi";
    sem_close(sm);
    sem_unlink(a);
}

/* Global variables */
int tMin, tMax;
int waitTime;
int aStage, eStage;
int performanceNoSigner = 0;
char singersQueue[QUEUE_SIZE][NAME_SIZE];
int qReadPos, qWritePos;
int stageNoA[QUEUE_SIZE], stageNoE[QUEUE_SIZE];
int numAcoustic, numElectric; 

/* Semaphores */
sem_t *tShirtGuys;

/* Mutex */
pthread_mutex_t stageLock = PTHREAD_MUTEX_INITIALIZER;

/* CV */
pthread_cond_t noAStage = PTHREAD_COND_INITIALIZER;
pthread_cond_t noEStage = PTHREAD_COND_INITIALIZER;
pthread_cond_t anyStage = PTHREAD_COND_INITIALIZER;
pthread_cond_t anySinger = PTHREAD_COND_INITIALIZER;
pthread_cond_t stageOrPerf = PTHREAD_COND_INITIALIZER;

/* data about musician */
struct musicianData {
    char iCode;
    int time;
    char *name;
    bool singer;
    int stage;
};

/* wrapper function for sem_open */
void semInit(int val){
    tShirtGuys = sem_open("hi", O_CREAT, 0644, val);
    if (tShirtGuys == SEM_FAILED) {
        perror("Failed to open semaphore");
        exit(-1);
    }
}

int getStageNo(int *stageA, int len){
    for(int i = 0; i < len; i++){
      if(stageA[i] == 0){
        stageA[i] = 1;
        return i + 1;
      }
    } 
    return -1;
}

void removeStageNo(int *stageA, int no){
    stageA[no - 1] = 0;
}


/* get current time in nanosecond accuracy */
struct timespec getTime(int t) {
    struct timeval tv;
    struct timespec ts;
    gettimeofday(&tv, NULL);
    ts.tv_sec = time(NULL) + t / 1000;
    ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (t % 1000);
    ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
    ts.tv_nsec %= (1000 * 1000 * 1000);
    ts.tv_nsec = 0;
    return ts;
}

/* actual performance time for musician*/
bool perform(struct musicianData *data, int performTime, char * name) {
    struct timespec tt = getTime(performTime * 1000);
    /* performance is waiting until the time or a singer to come */
    int rc = pthread_cond_timedwait(&anySinger, &stageLock, &tt);
    bool singerJoined = false;
    if (rc != ETIMEDOUT) {
        /* a singer comes in and joins */
        /* get singers name from queue */
        strcpy(name, singersQueue[qReadPos++]);
        printf(RED "Singer %s joined %s's performance, performance extended by 2 secs\n" RESET,
               name,
               data->name
        );
        performanceNoSigner--;
        singerJoined = true;
        mutexUnlock(&stageLock);
        /* wait until normal performance time is over */
        sleep(tt.tv_sec - time(NULL) + 2);
        mutexLock(&stageLock);
    }
    if (singerJoined) {
        /* wait two more seconds if singer joined */
        //sleep(2);
    }
    return singerJoined;
}


void collectTShirt(struct musicianData *data){
    /* guarded by semaphore */
    sem_wait(tShirtGuys);
    printf(CYN "%s collecting t-shirt\n" RESET, data->name);
    sleep(2);
    printf(CYN "%s got a t-shirt! Leaving now..\n" RESET, data->name);
    sem_post(tShirtGuys);
}


void performanceHandler(struct musicianData *data, const int stage) {
    /* get the duration to perform */
    const int durationToPerform = randNum(tMin, tMax);
    /* start the performance according to type */
    int stageNo = 0;
    if (stage == ELECTRIC) {
        stageNo = getStageNo(stageNoE, numElectric);
        printf(YEL "%s will perform %c on Electric Stage %d for %d seconds\n" RESET,
               data->name,
               data->iCode,
               stageNo,
               durationToPerform);
        eStage--;
    } else if (stage == ACOUSTIC) {
        stageNo = getStageNo(stageNoA, numAcoustic);
        printf(BLU"%s will perform %c on Acoustic Stage %d for %d seconds\n" RESET,
               data->name,
               data->iCode,
               stageNo,
               durationToPerform);
        aStage--;
    }
    /* signal if any singers waiting */
    performanceNoSigner++;
    condSignal(&stageOrPerf);

    /* do the actual performance */
    char singerName[NAME_SIZE];
    bool singer = perform(data, durationToPerform, singerName);
    //printf("%ld", time(NULL));

    /* end the performance according to stage type */
    if (stage == ELECTRIC) {
        removeStageNo(stageNoE, stageNo);
        if(!singer)
          printf(YEL "%s's performance on Electric Stage %d is finished\n" RESET, data->name, stageNo);
        else {
           printf(YEL "%s's performance with Singer %s on Electric Stage %d is finished\n" RESET, data->name, singerName, stageNo);
        }
        condSignal(&noEStage);
        eStage++;
    } else if (stage == ACOUSTIC) {
        removeStageNo(stageNoA, stageNo);
        if(!singer)
        printf(BLU "%s's performance on Acoustic Stage %d is finished\n" RESET, data->name, stageNo);
        else {
        printf(BLU "%s's performance with Singer %s on Acoustic Stage %d is finished\n" RESET, data->name, singerName, stageNo);
        }
        condSignal(&noAStage);
        aStage++;
    }
    /* if no singer joined */
    if (!singer)
        performanceNoSigner--;

    /* signal all waiting artists */
    condSignal(&anyStage);
    condSignal(&stageOrPerf);
    mutexUnlock(&stageLock);
    
    /* collect tShirts */
    collectTShirt(data);
    if(singer){
      data->name = singerName;
      collectTShirt(data);
    }
    pthread_exit(NULL);
}

void waitForStage(pthread_cond_t *cv, struct timespec *timeLimit, struct musicianData *data) {
    /* wait until timeLimit or you get a stage */
    int rc = pthread_cond_timedwait(cv, &stageLock, timeLimit);
    if (rc == ETIMEDOUT) {
        /* limit crossed then leave */
        printf(RED "%s %c left due to impatience\n" RESET,
               data->name,
               data->iCode);
        mutexUnlock(&stageLock);
        pthread_exit(NULL);
    }
}


void performAcoustic(struct musicianData *data, struct timespec *timeLimit) {
    mutexLock(&stageLock);
    /* wait for Acoustic Stage */
    while (aStage <= 0) {
        waitForStage(&noAStage, timeLimit, data);
    }
    performanceHandler(data, ACOUSTIC);
}


void performElectric(struct musicianData *data, struct timespec *timeLimit) {
    mutexLock(&stageLock);
    /* wait for Electric Stage */
    while (eStage <= 0) {
        waitForStage(&noEStage, timeLimit, data);
    }
    performanceHandler(data, ELECTRIC);
}


void performAnyStage(struct musicianData *data, struct timespec *timeLimit) {
    mutexLock(&stageLock);
    /* wait for any stage*/
    while (eStage + aStage <= 0) {
        waitForStage(&anyStage, timeLimit, data);
    }
    if (eStage > 0 && aStage > 0) {
        /* if both are available then choose at random */
        if (randNum(1, 2) == 1) {
            performanceHandler(data, ELECTRIC);
        } else {
            performanceHandler(data, ACOUSTIC);
        }
    } else if (eStage > 0) {
        performanceHandler(data, ELECTRIC);
    } else {
        performanceHandler(data, ACOUSTIC);
    }
}

void joinAMusician(struct musicianData *data) {
    /* add name to singers queue */
    strcpy(singersQueue[qWritePos++], data->name);
    /* signal performances with no singer */
    condSignal(&anySinger);
    mutexUnlock(&stageLock);
    pthread_exit(NULL);
}


void goOnStage(struct musicianData *data) {
    if (eStage > 0 && aStage > 0) {
        if (randNum(1, 2) == 1) {
            /* if both are available then choose at random*/
            performanceHandler(data, ELECTRIC);
        } else {
            performanceHandler(data, ACOUSTIC);
        }
    } else if (eStage > 0) {
        performanceHandler(data, ELECTRIC);
    } else {
        performanceHandler(data, ACOUSTIC);
    }

}


void singersCase(struct musicianData *data, struct timespec *timeLimit) {
    mutexLock(&stageLock);
    /* wait until a performance or stage is available to join*/
    while ((eStage + aStage <= 0) && performanceNoSigner <= 0) {
        waitForStage(&stageOrPerf, timeLimit, data);
    }
    if ((eStage > 0 || aStage > 0) && performanceNoSigner > 0) {
        /* both performance and stage are available choose at random*/
        if (randNum(1, 2) == 1) {
            joinAMusician(data);
        } else {
            goOnStage(data);
        }
    } else if (performanceNoSigner > 0) {
        /* only performance available then join it*/
        joinAMusician(data);
    } else {
        /* only stage available then join stage */
        goOnStage(data);
    }
}

void *musician(void *arg) {
    /* get the data */
    struct musicianData *data = arg;
    /* sleep for required time */
    int stage = data->stage;
    char stageName[100];
    if(stage == ACOUSTIC) 
      strcpy(stageName, "Acoustic");
    else if(stage == ELECTRIC)
      strcpy(stageName, "Electric");
    else 
      strcpy(stageName, "any");
    sleep(data->time);
    if(data->singer){
        printf(GRN "Singer %s, has arrived and is waiting for %s stage or performance\n" RESET, data->name, stageName);
    } else {
        printf(GRN "%s who plays %c, has arrived and is waiting for %s stage\n" RESET, data->name, data->iCode, stageName);
    }
    /* store max wait time in timeLimit */
    struct timespec timeLimit = getTime(waitTime * 1000);

    /* perform according to stage */
    if (stage == ACOUSTIC) {
        performAcoustic(data, &timeLimit);
    } else if (stage == ELECTRIC) {
        performElectric(data, &timeLimit);
    } else {
        if (data->singer) {
            /* special case for singer */
            singersCase(data, &timeLimit);
        } else {
            performAnyStage(data, &timeLimit);
        }
    }
    return NULL;
}

int main() {
    srandom(time(0));
    /* input handling and declare some local variables */
    int cordCount, numberMusician;
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
    for (int i = 0; i < numberMusician; i++) {
        names[i] = malloc(NAME_SIZE);
        scanf("%s ", names[i]);
        scanf("%c ", &iCode[i]);
        scanf("%d ", &time[i]);
    }
    struct musicianData musicianArgs[numberMusician];
    aStage = numAcoustic;
    eStage = numElectric;

    /* initialize semaphore */
    //sem_init(&tShirtGuys, 0, cordCount);
    semInit(cordCount);
    printf("Fest has started\n");

    /* create all the musician threads */
    pthread_t musicians[numberMusician];
    for (int i = 0; i < numberMusician; i++) {
        /* mData points the ith musicians data */
        struct musicianData *mData = &musicianArgs[i];
        mData->iCode = iCode[i];
        mData->time = time[i];
        mData->name = names[i];
        /* assign the stage according to instrument */
        if (iCode[i] == 'p' || iCode[i] == 'g' || iCode[i] == 's') {
            mData->stage = ACOUSTIC_ELECTRIC;
        } else if (iCode[i] == 'v') {
            mData->stage = ACOUSTIC;
        } else {
            mData->stage = ELECTRIC;
        }
        mData->singer = (iCode[i] == 's');
        pthreadCreate(&musicians[i], NULL, &musician, &musicianArgs[i]);
    }

    /* wait for all musicians to exit */
    for (int i = 0; i < numberMusician; i++) {
        pthreadJoin(musicians[i], NULL);
    }
    /* destroy the only semaphore */
    semDestroy(tShirtGuys);
    printf(RED "Finished\n" RESET);

}

