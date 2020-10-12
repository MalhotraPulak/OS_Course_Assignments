#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

int var;  //cookies in the jar
pthread_mutex_t mutex;

typedef struct s{
    int id;
} s;


void* t(void* inp){
	s* inputs = (s*) inp;
	
	sleep(2);  //2 secs to reach the jar

    pthread_mutex_lock(&mutex);
    if (var == 0){
		printf("Child %d didn't get to eat the cookie :(\n", inputs->id);
	} 
	else{
		printf("Child %d got to eat the cookie :)\n", inputs->id);
		var --;
	}
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(){
    var = 1;
	printf("Total cookies available: %d\n",var);

	pthread_t tid1;
    pthread_t tid2;

	s* thread1_input = (s*)malloc(sizeof(s));
    s* thread2_input = (s*)malloc(sizeof(s));
	thread1_input->id = 1;
    thread2_input->id = 2;

    pthread_mutex_init(&mutex, NULL);

    pthread_create(&tid1, NULL, t, (void*)(thread1_input));
    pthread_create(&tid2, NULL, t, (void*)(thread2_input));

    //join waits for the thread with id=tid to finish execution before proceeding
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    pthread_mutex_destroy(&mutex);
    printf("Cookies left in the jar: %d\n", var);
	return 0;
}
