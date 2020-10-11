#include<stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX 10000

int waitingStudents;


int randNum(int lower, int upper) {
    return random() % (upper - lower + 1) + lower;
}

void Pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                    void *(*start_routine)(void *), void *arg) {
    if (pthread_create(thread, attr,
                       start_routine, arg) != 0) {
        perror("Thread was not created");
    }
}

void Pthread_join(pthread_t thread, void **retval) {
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

void condBroadcast(pthread_cond_t *cv){
    if(pthread_cond_broadcast(cv) != 0){
        perror("Cant broadcast");
    }
}

struct companyArgs {
    int id;
    double p;
};
struct studentArgs {
    int id;
    int vaccinations;
};
struct zoneArgs {
    int id;
    int slots;
    int vaccines;
    double p_success;
};
struct vaccineData {
    int batches;
    int batchSize;
    int company;
    int pSuccess;
};

/* Declaring the pthreads */
pthread_t students[MAX];
pthread_t zones[MAX];
pthread_t companies[MAX];

/* Conditional Variables */
pthread_cond_t vaccineNotUsed = PTHREAD_COND_INITIALIZER;
pthread_cond_t noVaccineAvailable = PTHREAD_COND_INITIALIZER;
pthread_cond_t noSlotsAvailable = PTHREAD_COND_INITIALIZER;


/* Mutex Locks */
pthread_mutex_t studentMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vaccineQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vaccineUsedLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t slotQueueMutex = PTHREAD_MUTEX_INITIALIZER;


/* Global data for all threads to access*/
int vaccineUsed[MAX];
#define QUEUE_MAX 20000
int numCompanies, numZones, numStudents;
struct vaccineData vaccineQueue[QUEUE_MAX];
int readPosVaccine, writePosVaccine;
int slotQueue[QUEUE_MAX];
int readPosSlot, writePosSlot;

int createVaccine(const int id, double probSuccess, int *bs) {
    int batches = randNum(1, 5);
    int time = randNum(2, 5);
    int batchSize = randNum(10, 20);
    *bs = batchSize;
    //printf("**%d**", batchSize);
    printf("Pharmaceutical Company %d is preparing %d batches of vaccines which have success probability %0.2lf\n",
           id,
           batches,
           probSuccess);
    sleep(time);
    printf("Pharmaceutical Company %d has prepared %d batches of vaccines which have success probability %0.2lf. Waiting"
           " for all the vaccines to be used to resume production\n",
           id,
           batches,
           probSuccess);
    return batches;
}

/*
 * COMPANY
 * p probability of vaccine working
 * make random number of batches (> 1)
 * each batch has p (10 -20) vaccines
 * takes random (2 - 5) seconds to prepare vaccine
 * Signal vaccination zones vaccine is ready
 * Resumes when all its vaccines are used by the zone
 */
void addToVaccineQueue(int id, int vaccines, int bs) {
    mutexLock(&vaccineQueueMutex);
    vaccineQueue[writePosVaccine].batchSize = bs;
    vaccineQueue[writePosVaccine].batches = vaccines;
    vaccineQueue[writePosVaccine].company = id;
    writePosVaccine = (writePosVaccine + 1) % QUEUE_MAX;
    condBroadcast(&noVaccineAvailable);
    mutexUnlock(&vaccineQueueMutex);
    // TODO add overfill condition
}


void *company(void *ptr) {
    struct companyArgs *args = ptr;
    int id = args->id;
    double probSuccess = args->p;
    int batchSize;
    printf("company %d created with p = %lf\n", args->id, args->p);
    int vaccines = createVaccine(id, probSuccess, &batchSize);
    addToVaccineQueue(id, vaccines, batchSize);
    mutexLock(&vaccineUsedLock);
    while (vaccineUsed[id] != vaccines) {
        condWait(&vaccineNotUsed, &vaccineUsedLock);
    }
    mutexUnlock(&vaccineUsedLock);
    return NULL;
}

/* ---------------------------------------------------------------------------------------------------------------*/
void addToSlotsQueue(int slots, int id) {
    mutexLock(&slotQueueMutex);
    for (int i = 0; i < slots; i++) {
        slotQueue[writePosSlot] = id;
        writePosSlot = (writePosSlot + 1) % QUEUE_MAX;
    }
    printf("Vaccination Zone %d is ready to vaccinate with %d slots\n", id, slots);
    condBroadcast(&noSlotsAvailable);
    mutexUnlock(&slotQueueMutex);
}

void getFromVaccineQueue(int *vaccines, int *company) {
    mutexLock(&vaccineQueueMutex);
    while (readPosVaccine == writePosVaccine) {
        condWait(&noVaccineAvailable, &vaccineQueueMutex);
    }
    *vaccines = vaccineQueue[readPosVaccine].batchSize;
    *company = vaccineQueue[readPosVaccine].company;
    vaccineQueue[readPosVaccine].batches -= 1;
    if (vaccineQueue[readPosVaccine].batches == 0) {
        readPosVaccine = (readPosVaccine + 1) % QUEUE_MAX;
    }
    mutexUnlock(&vaccineQueueMutex);
}


int min(int a, int b) {
    return (a < b) ? a : b;
}

int check = true;

/*
 * ZONE
 * receives batch from one of the companies
 * if it has vaccines it makes random number of slots
 * for students (slots = min(8, vaccines left, waiting students))
 * receive a batch only when all the vaccines are used
 * It can only assign the free slots when it is done with vaccination phase
 * The vaccination zone should be on the lookout for new students
 */
void *zone(void *ptr) {
    struct zoneArgs *x = ptr;
    const int id = x->id;
    printf("Zone %d created\n", id);
    int company, vaccines = 0;
    while (check) {
        getFromVaccineQueue(&vaccines, &company);
        printf("Zone %d got %d vaccine from company %d\n", id, vaccines, company);
        int slots = min(9, min(waitingStudents, vaccines));
        addToSlotsQueue(slots, id);
        sleep(10);
    }
    return NULL;
}

/* --------------------------------------------------------------------------------------------------------- */
int getASlot(int id) {
    mutexLock(&slotQueueMutex);
    if (readPosSlot == writePosSlot) {
        condWait(&noSlotsAvailable, &slotQueueMutex);
    }
    int zone = slotQueue[readPosSlot];
    readPosSlot = (readPosSlot + 1) % QUEUE_MAX;
    printf("Student %d got assigned vaccination zone %d\n", id, zone);
    mutexUnlock(&slotQueueMutex);
    return zone;
}


/*
 * STUDENT
 * they become available at various different times
 * if after vaccination no antibodies then start again immediately
 * checks all zones with empty slots
 * signal the zone that he has arrived at the zone
 * at greater than 3 vaccinations he goes home
 */

void *student(void *ptr) {
    struct studentArgs *x = ptr;
    int id = x->id;
    printf("Student %d has arrived for his %d round of vaccination\n", id, x->vaccinations);
    sleep(randNum(1, 10));
    printf("Student %d is waiting to be allocated a vaccination zone\n", id);
    mutexLock(&studentMutex);
    waitingStudents++;
    mutexUnlock(&studentMutex);
    int zone = getASlot(id);


    return NULL;
}


int main() {
    srandom(time(0));
    numZones = 4;
    numStudents = 5;
    numCompanies = 3;
    double pVaccine[numCompanies];
    for (int i = 1; i < numCompanies + 1; i++) {
        pVaccine[i - 1] = i / 6.00;
    }
    /* write and read pointer for the vaccineQueue */
    writePosVaccine = 0;
    readPosVaccine = 0;
    /* Creating all the threads */
    /* Students */
    struct companyArgs *companyData[MAX];
    struct studentArgs *studentData[MAX];
    struct zoneArgs *zoneData[MAX];
    for (int i = 0; i < numStudents; i++) {
        studentData[i] = malloc(sizeof(struct studentArgs));
        studentData[i]->id = i;
        studentData[i]->vaccinations = 1;
        Pthread_create(&students[i], NULL, &student, studentData[i]);

    }
    /* Zones */
    for (int i = 0; i < numZones; i++) {
        zoneData[i] = malloc(sizeof(struct zoneArgs));
        zoneData[i]->id = i;
        zoneData[i]->slots = 0;
        zoneData[i]->vaccines = 0;
        zoneData[i]->p_success = 0;
        Pthread_create(&zones[i], NULL, &zone, zoneData[i]);

    }
    /* Companies */
    for (int i = 0; i < numCompanies; i++) {
        companyData[i] = malloc(sizeof(struct companyArgs));
        companyData[i]->id = i;
        companyData[i]->p = pVaccine[i];
        Pthread_create(&companies[i], NULL, &company, companyData[i]);

    }




    /* Waiting and joining the threads */
    for (int i = 0; i < numStudents; i++) {
        Pthread_join(students[i], NULL);
    }
    for (int i = 0; i < numCompanies; i++) {
        Pthread_join(companies[i], NULL);
    }
    for (int i = 0; i < numZones; i++) {
        Pthread_join(zones[i], NULL);
    }

}

/*
 * Problem
 * How does company knows its batches are consumed totally?
 * Vaccination zone tells company i just had a batch
 * Company does batches--
 * What if two vaccination zone at the same time?
 * Introduce global variable ig for the condition
 * If condition then increment pending or something ig
 * Or make the company wait on a different variable than the vaccination zone
 * And VZ signals from locked mutex as soon as signal is sent
 * Ahh this is getting confusing
 * If batches are zero then start creating again
 * When creating
 * Store the batches with batch size in a queue
 * Store the manufactured batches in the stack
 */
