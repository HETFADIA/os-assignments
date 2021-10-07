#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <errno.h>
#include <time.h>
void sleep_for_decided(int d){
    int select=1000*(700+rand()%800);//selects random time (0.7d,1.5d)
    usleep(select*d);
}
int main(){
    printf("hi1\n");
    usleep(0.9*1000000);
    printf("hi\n");
}