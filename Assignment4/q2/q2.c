#include<stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

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

#define QUEUE_MAX 200000
#define ZONES_MAX 1000
#define COMPANY_MAX 1000
#define STUDENT_MAX 10000
/* Global data for all threads to access*/
int waitingStudents;
int numCompanies, numZones, numStudents;
int vaccineUsed[COMPANY_MAX];
struct vaccineData vaccineQueue[QUEUE_MAX];
int readPosVaccine, writePosVaccine;
int slotQueue[QUEUE_MAX];
int zoneStudent[ZONES_MAX][STUDENT_MAX];
bool vaccinated[STUDENT_MAX] = {false};
double vaccineGiven[STUDENT_MAX];


/* Declaring the pthreads */
pthread_t students[STUDENT_MAX];
pthread_t zones[ZONES_MAX];
pthread_t companies[COMPANY_MAX];

/* Conditional Variables */
pthread_cond_t vaccineNotUsed = PTHREAD_COND_INITIALIZER;
pthread_cond_t noVaccineAvailable = PTHREAD_COND_INITIALIZER;
pthread_cond_t noSlotsAvailable = PTHREAD_COND_INITIALIZER;
pthread_cond_t notVaccinated = PTHREAD_COND_INITIALIZER;

/* Mutex Locks */
pthread_mutex_t vaccineQueueLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t slotQueueLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t studentLock = PTHREAD_MUTEX_INITIALIZER;

/* --------------------------------------------------------------------------------------------------------------*/
/*
 * COMPANY
 * p probability of vaccine working
 * make random number of batches (> 1)
 * each batch has p (10 -20) vaccines
 * takes random (2 - 5) seconds to prepare vaccine
 * Signal vaccination zones vaccine is ready
 * Resumes when all its vaccines are used by the zone
 */

int addToVaccineQueue(int id, double probSuccess) {
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

_Noreturn void *company(void *ptr) {
    /* get args */
    struct companyArgs *args = ptr;
    int companyId = args->id;
    double probSuccess = args->p;
    printf(GRN "Company %d created with p = %0.2lf\n" RESET, companyId, probSuccess);
    /* do  until all students exit */
    while (true) {
        const int batches = addToVaccineQueue(companyId, probSuccess);
        mutexLock(&vaccineQueueLock);
        while (vaccineUsed[companyId] != batches) {
            condWait(&vaccineNotUsed, &vaccineQueueLock);
        }
        printf(MAG "All the vaccines prepared by Pharmaceutical Company %d are emptied. Resuming production now.\n" RESET,
               companyId);
        mutexUnlock(&vaccineQueueLock);
    }
}

/* ---------------------------------------------------------------------------------------------------------------*/
void addToSlotsQueue(int vaccines, int id) {
    mutexLock(&slotQueueLock);
    int newSlots = min(8, min(waitingStudents, vaccines));
    if (newSlots != 0) {
        assert(slotQueue[id] == 0);
        slotQueue[id] = newSlots;
        printf("Vaccination Zone %d is ready to vaccinate with %d newSlots\n", id, newSlots);
        condBroadcast(&noSlotsAvailable);
    }
    mutexUnlock(&slotQueueLock);
    if(newSlots == 0){
        sleep(3);
        addToSlotsQueue(vaccines, id);
    }
}

void getFromVaccineQueue(int *vaccines, int *company, double *pSuccess, int zoneId) {
    mutexLock(&vaccineQueueLock);
    while (readPosVaccine == writePosVaccine) { /* no vaccine available */
        condBroadcast(&vaccineNotUsed); /* signal companies to check if all their batches have been used */
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

_Noreturn void *zone(void *ptr) {
    struct zoneArgs *x = ptr;
    const int zoneId = x->id;
    printf("Zone %d created\n", zoneId);
    int company, vaccines = 0;
    double pSuccess;
    while (true) {
        while (vaccines == 0) {
            printf("Zone %d is trying to get some medicines\n", zoneId);
            getFromVaccineQueue(&vaccines, &company, &pSuccess, zoneId);
        }
        sleep(randNum(1, 2));
        printf(CYN "Zone %d got %d vaccine from company %d\n" RESET, zoneId, vaccines, company);
        while (true) {
            addToSlotsQueue(vaccines, zoneId);
            sleep(10); // give 10 seconds for students to register
            int registeredStudents[numStudents];
            int studentCount = 0;
            mutexLock(&slotQueueLock);
            /* get registered students at this zone and close registrations*/
            while (zoneStudent[zoneId][studentCount] != -1) {
                registeredStudents[studentCount] = zoneStudent[zoneId][studentCount];
                zoneStudent[zoneId][studentCount] = -1;
                studentCount++;
            }
            slotQueue[zoneId] = 0; // no more registrations
            mutexUnlock(&slotQueueLock);
            if (studentCount == 0) {
                printf("Nobody applied for slots on Zone %d\n", zoneId);
                continue;
            }
            /* Vaccination Phase */
            sleep(1);
            mutexLock(&studentLock);
            printf(CYN "Zone %d is entering vaccination phase with %d students \n" RESET, zoneId, studentCount);
            for (int i = 0; i < studentCount; i++) {
                vaccinated[registeredStudents[i]] = true; // student is now vaccinated
                vaccineGiven[registeredStudents[i]] = pSuccess; // prob of vaccination success
                vaccines--; // zone lost one vaccine
                printf(MAG "Student %d on Vaccination Zone %d has been vaccinated which has success probability %0.2lf\n" RESET,
                       registeredStudents[i],
                       zoneId, pSuccess);
            }
            condBroadcast(&notVaccinated); // not vaccinated students should update
            printf("Zone %d has exited vaccination phase after vaccinating %d students\n", zoneId, studentCount);
            mutexUnlock(&studentLock);
            /* If no vaccines left get more vaccines else continue making slots*/
            if (vaccines == 0) {
                mutexLock(&vaccineQueueLock);
                vaccineUsed[company] += 1;
                printf(RED "Vaccination Zone %d has run out of vaccines\n" RESET, zoneId);
                mutexUnlock(&vaccineQueueLock);
                break;
            } else {
                continue;
            }
        }
    }
}


/* --------------------------------------------------------------------------------------------------------- */
int getRegistered(int id) {
    mutexLock(&slotQueueLock);
    int zone = -1; // Zone not defined
    while (zone == -1) {
        for (int aZone = 0; aZone < numZones; aZone++) {
            if (slotQueue[aZone] > 0) {
                slotQueue[aZone]--; // remove that slot from slot queue
                zone = aZone;
                break;
            }
        }
        if (zone == -1) {
            condWait(&noSlotsAvailable, &slotQueueLock); // wait until slots available
        }
    }
    waitingStudents--; // after getting slot reduce waiting students
    printf(BLU "Student %d got assigned vaccination zone %d\n" RESET, id, zone);
    /* this mapping tells zone the assigned student */
    for (int i = 0; i < numStudents; i++) {
        if (zoneStudent[zone][i] < 0) {
            zoneStudent[zone][i] = id;
            break;
        }
    }
    mutexUnlock(&slotQueueLock);
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
    sleep(randNum(1, 5));
    printf("Student %d has entered the college gate\n", studentId);
    sleep(randNum(1, 10));
    while (true) {
        printf(GRN "Student %d has arrived for his %d round of vaccination\n" RESET, studentId, vaccinations);
        printf(GRN "Student %d is waiting to be allocated a vaccination zone\n" RESET, studentId);
        /* changing waiting students depends on the slot queue */
        mutexLock(&slotQueueLock);
        waitingStudents++;
        mutexUnlock(&slotQueueLock);
        getRegistered(studentId);
        /* lock studentMutex*/
        mutexLock(&studentLock);
        while (!vaccinated[studentId]) {
            /* waits until a student is vaccinated and then checks*/
            condWait(&notVaccinated, &studentLock);
        }
        vaccinated[studentId] = false; // make it false for next time
        mutexUnlock(&studentLock);
        /* unlock studentMutex */
        double s = randNum(0, 100) / 100.00;
        /* Antibody test begins */
        sleep(randNum(0, 3));
        if (s < vaccineGiven[studentId]) {
            /* Test passed */
            printf(YEL "Student %d has tested positive for antibodies! (%0.2lf)\n" RESET, studentId, s);
            pthread_exit(NULL);
        } else {
            /* Test failed */
            vaccinations++;
            printf(RED "Student %d has tested negative for antibodies\n" RESET, studentId);
            if (vaccinations > 3) {
                /* Max vaccinate 3 times */
                printf(RED "College gave up on student %d, have fun in online classes!!\n" RESET, studentId);
                pthread_exit(NULL);
            }
        }
    }
}

/* --------------------------------------------------------------------------------------------- */

int main() {
    srandom(time(0));
    /* numZones = 10;
    numStudents = 20;
    numCompanies = 10;*/
    scanf("%d %d %d", &numCompanies, &numZones, &numStudents);
    /* stores all the pSuccess for companies */
    if(numCompanies == 0){
        printf("No companies\n");
        return 0;
    } else if(numZones == 0){
        printf("No Zones\n");
        return 0;
    } else if(numStudents == 0){
        printf("No Students\n");
        return 0;
    }
    double pVaccine[numCompanies];
    /* for (int i = 1; i < numCompanies + 1; i++) {
        pVaccine[i - 1] = (double) i / (numCompanies + 1);
    }*/
    for (int i = 0; i < numCompanies; i++) {
        scanf("%lf", &pVaccine[i]);
    }
    /* initialize zoneStudent to -1 */
    for (int i = 0; i < numZones; i++) {
        for (int j = 0; j < numStudents + 1; j++) {
            zoneStudent[i][j] = -1;
        }
    }
    /* write and read pointer for the vaccineQueue */
    writePosVaccine = 0;
    readPosVaccine = 0;
    /* args for all the threads */
    struct companyArgs *companyData[numCompanies];
    struct studentArgs *studentData[numStudents];
    struct zoneArgs *zoneData[numZones];
    /* Creating all the threads */
    /* Students */
    for (int i = 0; i < numStudents; i++) {
        studentData[i] = malloc(sizeof(struct studentArgs));
        studentData[i]->id = i;
        pthreadCreate(&students[i], NULL, &student, studentData[i]);

    }
    /* Zones */
    for (int i = 0; i < numZones; i++) {
        zoneData[i] = malloc(sizeof(struct zoneArgs));
        zoneData[i]->id = i;
        pthreadCreate(&zones[i], NULL, &zone, zoneData[i]);

    }
    /* Companies */
    for (int i = 0; i < numCompanies; i++) {
        companyData[i] = malloc(sizeof(struct companyArgs));
        companyData[i]->id = i;
        companyData[i]->p = pVaccine[i];
        pthreadCreate(&companies[i], NULL, &company, companyData[i]);

    }
    /* Waiting for all students to return and exit*/
    for (int i = 0; i < numStudents; i++)
        pthreadJoin(students[i], NULL);
    printf("Simulation Over. Fate of everyone decided.\n");
    for (int i = 0; i < numCompanies; i++)
        pthread_cancel(companies[i]);
    for (int i = 0; i < numZones; i++)
        pthread_cancel(zones[i]);
}

