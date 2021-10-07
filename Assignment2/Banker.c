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
int **requests;

int min(int x, int y){
    if(x < y) return x;
    return y;
}

void *P_x(void *args){
    int thread_num = *((int *)args);
    int arr[TOTAL_RESOURCES], save_temp[TOTAL_RESOURCES];
    while(1){
        int TRACK = TOTAL_RESOURCES;
        for(int i = 0; i < TOTAL_RESOURCES; i++){
            arr[i] = rand() % (arr_of_resources[i] + 1);
            if(!arr[i]) TRACK -= 1;
            save_temp[i] = arr[i];
            requests[thread_num][i] = arr[i];
        }
        // TRACK the requirement of resources, if none end looping
        while(TRACK > 0){
            int resource_for_now = rand() % TOTAL_RESOURCES;

            if(!arr[resource_for_now]) continue;

            //mutex lock part 1
            int taken = min(arr_of_resources[resource_for_now], arr[resource_for_now]);
            arr_of_resources[resource_for_now] -= taken;
            //mutex unlock

            arr[resource_for_now] -= taken;
            requests[thread_num][resource_for_now] -= taken;

            if(!arr[resource_for_now]){
                TRACK -= 1;
            }
            sleep(rand() % TIME_DELAY + 1);
        }
        // sleep for (0.7d, 1.5d);
        usleep((7 * TIME_DELAY + rand() % TIME_DELAY * 8) * 1e5);
        // run thread again like normal people

        //return the resources back
        for(int i = 0; i < TOTAL_RESOURCES; i++){
            //mutex lock 2
            arr_of_resources[i] += save_temp[i];
            //mutex unlock
        }
    }
    return NULL;
}

void *deadlock_detection(void *args){
    //run this thread infinitely

    //do something, I forgor 💀
    for(int i = 0; i < TOTAL_THREADS; i++){
        for(int j = 0; j < TOTAL_RESOURCES; j++){
            
        }
    }

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
    for(int i = 0; i < TOTAL_RESOURCES; i++){
        arr_of_resources[i] = stoi(argv[i + 2]);
    }
    TOTAL_THREADS = stoi(argv[TOTAL_RESOURCES + 2]);
    requests = (int **)malloc(sizeof(int *) * TOTAL_THREADS);
    for(int i = 0; i < TOTAL_THREADS; i++){
        requests[i] = (int *)malloc(sizeof(int) * TOTAL_RESOURCES);
    }
    TIME_DELAY = stoi(argv[TOTAL_RESOURCES + 3]);
}