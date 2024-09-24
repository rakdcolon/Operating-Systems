/*
 * Rohan Karamel (RAK218)
 * Sankar Gollapudi (SAG341)
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_t t1, t2, t3, t4;
pthread_mutex_t mutex;
int x = 0;
int loop = 10000;

void *add_counter(void *arg) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < loop; i++) x += 1;
    pthread_mutex_unlock(&mutex);
}

int main(int argc, char *argv[]) {
    
    if (argc != 2) {
        printf("Bad Usage: Must pass in an integer\n");
        exit(1);
    }

    loop = atoi(argv[1]);

    printf("Going to run four threads to increment x up to %d\n", 4 * loop);

    pthread_mutex_init(&mutex, NULL);

    pthread_create(&t1, NULL, add_counter, NULL);
    pthread_create(&t2, NULL, add_counter, NULL);
    pthread_create(&t3, NULL, add_counter, NULL);
    pthread_create(&t4, NULL, add_counter, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);
  
    pthread_mutex_destroy(&mutex);

    printf("The final value of x is %d\n", x);

    return 0;
}
