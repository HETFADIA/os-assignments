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
int max_instances,max_threads,deadlock_detection_interval;
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
}
int main(int argc, char **argv){
    //a.out || [Maximum number of instances available] || [Maximum number of threads to use in the simulation.] || [Deadlock detection check interval d in seconds.] || name of the instances
    if(argc<5){
        printf("format of input = a.out || [Maximum number of instances available] || [Maximum number of threads to use in the simulation.] || [Deadlock detection check interval d in seconds.] || name of the instances ");
        exit(-1);
    }
    max_instances=stoi(argv[1]);
    max_threads=stoi(argv[2]);
    deadlock_detection_interval=stoi(argv[3]);
    array_of_resources=(int *)malloc(sizeof(int)*max_instances);
    if(max_instances+4!=argc){
        printf("incorrect input format\n");
        printf("The no of max instances is incorrect\n");
        exit(-1);
    }
    if(max_instances==0){
        printf("Please provide at least 1 max instance\n");
        exit(-1);
    }
    for(int i=4;i<argc;i++){
        array_of_resources[i-4]=stoi(argv[i]);
    }
}