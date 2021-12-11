#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <dlfcn.h>
#include <sys/resource.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int TOTAL_RESOURCES, TOTAL_THREADS, TIME_DELAY;
int *arr_of_resources;
bool *isRunning;
int array_req[1000][1000];
void *func(void *args,int ){
    int arr[TOTAL_RESOURCES];

    while(1){
        for(int i = 0; i < TOTAL_RESOURCES; i++){
            arr[i] = rand() % (arr_of_resources[i] + 1);
        }

        //some_func returns false for all 0 in arr
        while(some_func(arr)){
            int resource_for_now = rand() % TOTAL_RESOURCES;
            if(arr)
            int request = rand() % arr[resource_for_now] + 1;
            
            if(arr_of_resource[resource_for_now] < request){
                //do some additional stuff
                continue;
            }

            //mutex lock
            arr_of_resources[resource_for_now] -= request;
            //mutex unlock

            arr[resource_for_now] -= request;

            sleep(rand() % TIME_DELAY + 1);
        }
        // sleep for (0.7d, 1.5d);

        usleep(700 * TIME_DELAY + (rand() % TIME_DELAY) * 800);
        
        // run thread again like normal people

        //vapas karo resource NOW
    }
    return NULL;
}

void *deadlock_detection(void *args){
    //run this thread infinitely

    //do something, I forgor 😭
    bool isDeadlock=0;


    sleep(TIME_DELAY);
    return NULL;
}

int main(int argc, char **argv){
    if(argc < 5){
        // ./a.out 1 1 5 4
        printf("Invalid input, insufficient command line arguments\n");
        exit(-1);
    }
    TOTAL_RESOURCES = stoi(argv[1]);
    arr_of_resources = (int *)malloc(sizeof(int) * TOTAL_RESOURCES);
    isRunning=(bool *)malloc(sizeof(bool)*TOTAL_RESOURCES);
    for(int i = 0; i < TOTAL_RESOURCES; i++){
        arr_of_resources[i] = stoi(argv[i + 2]);
        isRunning[i]=0;
    }
    TOTAL_THREADS = stoi(argv[TOTAL_RESOURCES + 2]);
    TIME_DELAY = stoi(argv[TOTAL_RESOURCES + 3]);
}