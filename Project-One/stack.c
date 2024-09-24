/*
 * Rohan Karamel (RAK218)
 * Sankar Gollapudi (SAG341)
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void signal_handle(int signalno) {

    int i = 0;
    printf("handling segmentation fault!\n");
    i = i + 1;
  
    int *p = &signalno;
    int offset = 15;
    int instruction_length = 2;

    void **pc_address = (void **)((int *)p + offset);
    *pc_address += instruction_length;
}

int main(int argc, char *argv[]) {

    int r2 = 0;

    signal(SIGSEGV, signal_handle);

    r2 = *( (int *) 0 );

    r2 = r2 + 1 * 45;
    printf("result after handling seg fault %d!\n", r2);

    return 0;
}
