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
int * array_of_resources;
int max_resources,max_threads,deadlock_detection_interval;
int requests[200][1000];
int min(int x,int y){
    return x<y?x:y;
}
int max(int x,int y){
    return x>y?x:y;
}
int randint(int x,int y){
    return x+ rand()%(y-x+1);
}
int randrange(int x,int y){
    return x+ rand()%(y-x);
}
void * thread_process(void * args){
    int *thread_no_pointer=(int *)args;
    int thread_no=*thread_no_pointer;
    int arr[max_resources];
    int total_resource_requested_by_thread[max_resources];
    while(1){
        int total_non_zero_resouce_for_thread = max_resources;
        for(int i = 0; i < max_resources; i++){
            arr[i] = rand() % (arr_of_resources[i] + 1);
            if(!arr[i]) total_non_zero_resouce_for_thread -= 1;
            total_resource_requested_by_thread[i] = arr[i];
            requests[thread_no][i] = arr[i];
        }
        // total_non_zero_resouce_for_thread the requirement of resources, if none end looping
        while(total_non_zero_resouce_for_thread > 0){
            int resource_for_now = rand() % max_resources;

            if(!arr[resource_for_now]) continue;

            //mutex lock part 1
            int taken = min(arr_of_resources[resource_for_now], arr[resource_for_now]);
            arr_of_resources[resource_for_now] -= taken;
            //mutex unlock

            arr[resource_for_now] -= taken;
            requests[thread_no][resource_for_now] -= taken;

            if(!arr[resource_for_now]){
                total_non_zero_resouce_for_thread -= 1;
            }
            sleep(rand() % deadlock_detection_interval + 1);
        }
        // sleep for (0.7d, 1.5d);
        usleep((7 * deadlock_detection_interval + rand() % deadlock_detection_interval * 8) * 1e5);
        // run thread again like normal people

        //return the resources back
        for(int i = 0; i < max_resources; i++){
            //mutex lock 2
            arr_of_resources[i] += total_resource_requested_by_thread[i];
            //mutex unlock
        }
    }
    return NULL;
}
void sleep_for_decided(int d){
    int select=1000*(700+rand()%800);//selects random time (0.7d,1.5d)
    usleep(select*d);
}
int main(int argc, char **argv){
    //a.out || [Maximum number of instances available] || [Maximum number of threads to use in the simulation.] || [Deadlock detection check interval d in seconds.] || name of the instances
    if(argc<5){
        printf("format of input = a.out || [Maximum number of instances available] || [Maximum number of threads to use in the simulation.] || [Deadlock detection check interval d in seconds.] || name of the instances ");
        exit(-1);
    }
    max_resources=stoi(argv[1]);
    max_threads=stoi(argv[2]);
    deadlock_detection_interval=stoi(argv[3]);
    array_of_resources=(int *)malloc(sizeof(int)*max_resources);
    if(max_resources+4!=argc){
        printf("incorrect input format\n");
        printf("The no of max instances is incorrect\n");
        exit(-1);
    }
    if(max_resources==0){
        printf("Please provide at least 1 max instance\n");
        exit(-1);
    }
    for(int i=4;i<argc;i++){
        array_of_resources[i-4]=stoi(argv[i]);
    }
}