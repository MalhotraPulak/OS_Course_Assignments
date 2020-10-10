#include<stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

int waitingStudents;

int rand_num(int lower, int upper) {
    return random() % (upper - lower + 1) + 1;
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

/*
 * COMPANY
 * p probability of vaccine working
 * make random number of batches (> 1)
 * each batch has p (10 -20) vaccines
 * takes random (2 - 5) seconds to prepare vaccine
 * Signal vaccination zones vaccine is ready
 * Resumes when all its vaccines are used by the zone
 */

void createVaccine(const int id, const double probSuccess, int *batches, int *vaccinePerBatch) {
    *batches = rand_num(1, 5);
    int time = rand_num(2, 5);
    *vaccinePerBatch = rand_num(10, 20);
    printf("Pharmaceutical Company %d is preparing %d batches of vaccines which have success probability %0.2lf\n",
           id,
           *batches,
           probSuccess);
    sleep(time);
    printf("Pharmaceutical Company %d has prepared %d batches of vaccines which have success probability %0.2lf. Waiting"
           " for all the vaccines to be used to resume production\n",
           id,
           *batches,
           probSuccess);

}

void *company(void *ptr) {
    struct companyArgs *args = ptr;
    int id = args->id;
    double probSuccess = args->p;
    int batches, vaccinePerBatch;
    printf("company %d created with p = %lf\n", args->id, args->p);
    createVaccine(id, probSuccess, &batches, &vaccinePerBatch);

    return NULL;
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

void getVaccines() {

}

int min(int a, int b) {
    return (a < b) ? a : b;
}

_Noreturn void *zone(void *ptr) {
    struct zoneArgs *x = ptr;
    printf("zone %d created\n", x->id);
    while (true) {
        if (x->vaccines == 0) {
            getVaccines();
        }
        x->slots = min(8, min(x->vaccines, waitingStudents));
    }
    return NULL;
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
    printf("student %d created\n", x->id);

    return NULL;
}


int main() {
    srandom(time(0));
    int numCompanies, numZones, numStudents;
    numZones = 4, numStudents = 5;
    numCompanies = 3;
    double pVaccine[numCompanies];
    for (int i = 1; i < numCompanies + 1; i++) {
        pVaccine[i - 1] = i / 6.00;
    }
    /* Declaring the pthreads */
    pthread_t students[numStudents];
    pthread_t zones[numZones];
    pthread_t companies[numCompanies];
    /* Creating all the threads */
    /* Students */
    for (int i = 0; i < numStudents; i++) {
        struct studentArgs *sArg = malloc(sizeof(struct studentArgs));
        sArg->id = i;
        sArg->vaccinations = 0;
        Pthread_create(&students[i], NULL, &student, sArg);

    }
    /* Zones */
    for (int i = 0; i < numZones; i++) {
        struct zoneArgs *zArg = malloc(sizeof(struct zoneArgs));
        zArg->id = i;
        zArg->slots = 0;
        zArg->vaccines = 0;
        zArg->p_success = 0;
        Pthread_create(&zones[i], NULL, &zone, zArg);

    }
    /* Companies */
    for (int i = 0; i < numCompanies; i++) {
        struct companyArgs *cArg = malloc(sizeof(struct companyArgs));
        cArg->id = i;
        cArg->p = pVaccine[i];
        Pthread_create(&companies[i], NULL, &company, cArg);

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