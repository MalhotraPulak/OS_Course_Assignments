#include<stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

void Pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg) {
    if(pthread_create(thread, attr,
        start_routine, arg) != 0){
        perror("Thread was not created");
    }
}

void Pthread_join(pthread_t thread, void **retval){
    if(pthread_join(thread, retval) != 0){
       perror("Thread could not join");
    }

}
struct companyArgs{
    int id;
    double p;
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
void* company(void *ptr) {
    struct companyArgs *args = ptr;
    printf("company %d created with p = %lf\n", args-> id, args -> p);
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
void* zone(void *ptr) {
    int *x = ptr;
    printf("zone %d created\n", *x);
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

void* student(void *ptr) {
    int *x = ptr;
    printf("student %d created\n", *x);
    return NULL;
}


int main() {
    int num_company, num_zones, num_students;
    num_zones = 4, num_students = 5;
    num_company = 3;
    double p_vaccine[num_company];
    for (int i = 1; i < num_company + 1; i++) {
        p_vaccine[i - 1] = i / 6.00;
    }
    /* Declaring the pthreads */
    pthread_t students[num_students];
    pthread_t zones[num_zones];
    pthread_t companies[num_company];
    /* Creating all the threads */
    for (int i = 0; i < num_students; i++) {
        int *k = malloc(sizeof(int));
        *k = i;
        Pthread_create(&students[i], NULL, &student, k);

    }
    for (int i = 0; i < num_zones; i++) {
        int *k = malloc(sizeof(int));
        *k = i;
        Pthread_create(&zones[i], NULL, &zone, k);

    }
    for (int i = 0; i < num_company; i++) {
        struct companyArgs *c_arg = malloc(sizeof(struct companyArgs));
        c_arg -> id = i;
        c_arg -> p = p_vaccine[i];
        Pthread_create(&companies[i], NULL, &company, c_arg);

    }




    /* Waiting and joining the threads */
    for(int i = 0; i < num_students; i++){
        Pthread_join(students[i], NULL);
    }
    for(int i = 0; i < num_students; i++){
        Pthread_join(companies[i], NULL);
    }
    for(int i = 0; i < num_students; i++){
        Pthread_join(zones[i], NULL);
    }

}