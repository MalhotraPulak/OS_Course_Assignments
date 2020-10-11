#include<stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>


/* wrapper functions */
int randNum(int lower, int upper) {
    return random() % (upper - lower + 1) + lower;
}

int min(int a, int b) {
    return (a < b) ? a : b;
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

void condBroadcast(pthread_cond_t *cv) {
    if (pthread_cond_broadcast(cv) != 0) {
        perror("Cant broadcast");
    }
}

/* structs */
struct companyArgs {
    int id;
    double p;
};
struct studentArgs {
    int id;
};
struct zoneArgs {
    int id;
};
struct vaccineData {
    int vaccineCount;
    int company;
    double pSuccess;
};

/* Global data for all threads to access*/
#define QUEUE_MAX 20000
#define MAX 1000
int waitingStudents;
int numCompanies, numZones, numStudents;
int vaccineUsed[MAX];
struct vaccineData vaccineQueue[QUEUE_MAX];
int readPosVaccine, writePosVaccine;
int slotQueue[MAX];
int zoneStudent[MAX][MAX];
bool vaccinated[MAX] = {false};
double vaccineGiven[MAX];
//int readPosSlot, writePosSlot;

/* Declaring the pthreads */
pthread_t students[MAX];
pthread_t zones[MAX];
pthread_t companies[MAX];

/* Conditional Variables */
pthread_cond_t vaccineNotUsed = PTHREAD_COND_INITIALIZER;
pthread_cond_t noVaccineAvailable = PTHREAD_COND_INITIALIZER;
pthread_cond_t noSlotsAvailable = PTHREAD_COND_INITIALIZER;
pthread_cond_t vaccinatedWait = PTHREAD_COND_INITIALIZER;

/* Mutex Locks */
pthread_mutex_t vaccineQueueLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t slotQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t studentMutex = PTHREAD_MUTEX_INITIALIZER;

/* --------------------------------------------------------------------------------------------------------------*/


int createVaccine(const int id, double probSuccess, int *bs) {
    int batches = randNum(1, 5);
    int time = randNum(2, 5);
    int batchSize = randNum(10, 20);
    *bs = batchSize;


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

int addToVaccineQueue(int id,  double probSuccess) {
    int batches = randNum(1, 5);
    int time = randNum(2, 5);
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
    mutexLock(&vaccineQueueLock);
    for (int i = 0; i < batches; i++) {
        vaccineQueue[writePosVaccine].vaccineCount = randNum(10, 20);
        vaccineQueue[writePosVaccine].company = id;
        vaccineQueue[writePosVaccine].pSuccess = probSuccess;
        writePosVaccine = (writePosVaccine + 1) % QUEUE_MAX;
    }
    condBroadcast(&noVaccineAvailable);
    mutexUnlock(&vaccineQueueLock);
    return batches;
}

int t = true;

void *company(void *ptr) {
    struct companyArgs *args = ptr;
    int companyId = args->id;
    double probSuccess = args->p;
    printf("Company %d created with p = %0.2lf\n", companyId, probSuccess);
    while (true) {
        const int batches = addToVaccineQueue(companyId, probSuccess);
        mutexLock(&vaccineQueueLock);
        while (vaccineUsed[companyId] != batches) {
            condWait(&vaccineNotUsed, &vaccineQueueLock);
        }
        printf("All the vaccines prepared by Pharmaceutical Company %d are emptied. Resuming production now.\n", companyId);
        mutexUnlock(&vaccineQueueLock);
        if (!t) {
            break;
        }
    }
    return NULL;
}

/* ---------------------------------------------------------------------------------------------------------------*/
void addToSlotsQueue(int vaccines, int id) {
    mutexLock(&slotQueueMutex);
    int slots = min(8, min(waitingStudents, vaccines));
    //printf("waitingStudents %d, vaccines %d \n", waitingStudents, vaccines);
    assert(slotQueue[id] == 0);
    slotQueue[id] = slots;
    printf("Vaccination Zone %d is ready to vaccinate with %d slots\n", id, slots);
    condBroadcast(&noSlotsAvailable);
    mutexUnlock(&slotQueueMutex);
}

void getFromVaccineQueue(int *vaccines, int *company, double *pSuccess, int zoneId) {
    mutexLock(&vaccineQueueLock);
    while (readPosVaccine == writePosVaccine) {
        condBroadcast(&vaccineNotUsed);
        condWait(&noVaccineAvailable, &vaccineQueueLock);
    }
    *vaccines = vaccineQueue[readPosVaccine].vaccineCount;
    *company = vaccineQueue[readPosVaccine].company;
    *pSuccess = vaccineQueue[readPosVaccine].pSuccess;
    readPosVaccine = (readPosVaccine + 1) % QUEUE_MAX;
    printf("Pharmaceutical Company %d is delivering a vaccine batch to VZ %d which has"
           " PSuccess of %0.2lf\n",
           *company, zoneId, *pSuccess);
    mutexUnlock(&vaccineQueueLock);
}

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
    const int zoneId = x->id;
    printf("Zone %d created\n", zoneId);
    int company, vaccines = 0;
    double pSuccess;
    while (true) {
        while(vaccines == 0) {
            printf("Zone %d is trying to get some medicines\n", zoneId);
            getFromVaccineQueue(&vaccines, &company, &pSuccess, zoneId);
        }
        sleep(randNum(1, 2));
        printf("Zone %d got %d vaccine from company %d\n", zoneId, vaccines, company);
        while (true) {
            addToSlotsQueue(vaccines, zoneId);
            sleep(10); // give 10 seconds for students to register
            int sToV[numStudents];
            int studentCount = 0;
            mutexLock(&slotQueueMutex);
            while (zoneStudent[zoneId][studentCount] != -1) {
                sToV[studentCount] = zoneStudent[zoneId][studentCount];
                zoneStudent[zoneId][studentCount] = -1;
                studentCount++;
            }
            slotQueue[zoneId] = 0;
            if (studentCount != 0)
                printf("Zone %d is entering vaccination phase with %d students \n", zoneId, studentCount);
            mutexUnlock(&slotQueueMutex);
            if(studentCount == 0){
                printf("No body applied for slots on Zone %d\n", zoneId);
                continue;
            }
            mutexLock(&studentMutex); // student mutex again
            for (int i = 0; i < studentCount; i++) {
                vaccinated[sToV[i]] = true;
                vaccineGiven[sToV[i]] = pSuccess;
                vaccines--;
                printf("Student %d on Vaccination Zone %d has been vaccinated which has success probability %0.2lf\n",
                       sToV[i],
                       zoneId, pSuccess);
            }
            condBroadcast(&vaccinatedWait);
            printf("Zone %d has exited vaccination phase after vaccinating %d students\n", zoneId, studentCount);
            mutexUnlock(&studentMutex);
            /* Signal Company you used a batch*/
            if (vaccines == 0) {
                mutexLock(&vaccineQueueLock);
                vaccineUsed[company] += 1;
                printf("Vaccination Zone %d has run out of vaccines\n", zoneId);
                mutexUnlock(&vaccineQueueLock);
                break;
            } else {
                continue;
            }
        }
        if (!t) {
            break;
        }
    }
    return NULL;
}


/* --------------------------------------------------------------------------------------------------------- */
int getVaccinated(int id) {
    mutexLock(&slotQueueMutex);
    int zone = -1; // Zone not defined
    while (zone == -1) {
        for (int i = 0; i < numCompanies; i++) {
            if (slotQueue[i] > 0) {
                slotQueue[i]--; // remove that slot from slot queue
                zone = i;
                break;
            }
        }
        if (zone == -1) {
            condWait(&noSlotsAvailable, &slotQueueMutex); // wait until slots available
        }
    }
    waitingStudents--; // after getting slot reduce waiting students
    printf("Student %d got assigned vaccination zone %d\n", id, zone);
    /* this mapping tells zone the assigned student */
    for (int i = 0; i < numStudents; i++) {
        if (zoneStudent[zone][i]  < 0) {
            zoneStudent[zone][i] = id;
            break;
        }
    }
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
    int studentId = x->id;
    int vaccinations = 1;
    printf("Student %d has entered the college gate\n", studentId);
    sleep(randNum(1, 10));
    while (true) {
        printf("Student %d has arrived for his %d round of vaccination\n", studentId, vaccinations);
        printf("Student %d is waiting to be allocated a vaccination zone\n", studentId);
        /* changing waiting students depends on the slot queue */
        mutexLock(&slotQueueMutex);
        waitingStudents++;
        mutexUnlock(&slotQueueMutex);
        getVaccinated(studentId);

        mutexLock(&studentMutex); // student MUTEX now, also in ZONE tho
        while (!vaccinated[studentId]) {
            condWait(&vaccinatedWait, &studentMutex);
        }
        vaccinated[studentId] = false; // make it false for next time
        mutexUnlock(&studentMutex);
        double s = randNum(0, 100)/ 100.00;
        sleep(randNum(0, 3));
        if (s < vaccineGiven[studentId]) {
            printf("Student %d has tested positive for antibodies! (%0.2lf)\n", studentId, s);
            pthread_exit(NULL);
        } else {
            vaccinations++;
            printf("Student %d has tested negative for antibodies\n", studentId);
            if (vaccinations >= 4) {
                printf("College gave up on student %d, have fun in online classes!!\n", studentId);
                pthread_exit(NULL);
            }
        }
    }
}


int main() {
    srandom(time(0));
    numZones = 10;
    numStudents = 20;
    numCompanies = 10;
    for(int i = 0; i < numZones; i++){
        for(int j = 0; j < numStudents + 1; j++){
            zoneStudent[i][j] = -1;
        }
    }
    double pVaccine[numCompanies];
    for (int i = 1; i < numCompanies + 1; i++) {
        pVaccine[i - 1] = (double) i / (numCompanies + 1);
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
        Pthread_create(&students[i], NULL, &student, studentData[i]);

    }
    /* Zones */
    for (int i = 0; i < numZones; i++) {
        zoneData[i] = malloc(sizeof(struct zoneArgs));
        zoneData[i]->id = i;
        Pthread_create(&zones[i], NULL, &zone, zoneData[i]);

    }
    /* Companies */
    for (int i = 0; i < numCompanies; i++) {
        companyData[i] = malloc(sizeof(struct companyArgs));
        companyData[i]->id = i;
        companyData[i]->p = pVaccine[i];
        Pthread_create(&companies[i], NULL, &company, companyData[i]);

    }
    /* Waiting for all students to return */
    for (int i = 0; i < numStudents; i++) {
        Pthread_join(students[i], NULL);

    }
    printf("Simulation Over. Fate of everyone decided.\n");
}

// Problem
/*
 * Company
 * To create vaccines
 * Lock in mutex for vaccineQueue
 * ADD THE BATCH
 * Signal Zones that newVaccineIsAdded
 * Release the lock for vaccineQueue
 * wait till medicine used
 * only 1 Zone can signal for vaccineUsed at once
 * Lock the student mutex
 * wait for signal
 * Unlock the student mutex
 *
 *
 *
 * Zones
 * Lock in mutex for vaccineQueue
 * Wait for newVaccine is added
 * If condition is false now wait again we get some vaccines
 * Unlock mutex
 * Now make some slots available
 * Add some slots to slots
 * Notify students there are some slots
 * Lock in slots
 * Add slots to the queue
 * Unlock the slots
 * Sleep
 * Lock slots
 * Delete my slots
 * Unlock slots
 * Enter vaccination phase
 * Lock the slots
 * Give vaccine to students which are in my zone
 * Unlock the slots
 * Lock the student mutex
 * Signal to Company that done
 * Unlock the student mutex
 * ...
 *
 *
 *
 * Student
 * Wait till slotsAvailable
 *
 *
 *
 */