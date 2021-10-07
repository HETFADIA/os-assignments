#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <errno.h>
#include <time.h>
int main(){
    printf("hi1\n");
    usleep(0.9*1000000);
    printf("hi\n");
}