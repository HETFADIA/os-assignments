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

int main(int argc, char **argv){
    //a.out || [Maximum number of instances available] || [Maximum number of threads to use in the simulation.] || [Deadlock detection check interval d in seconds.] || name of the instances
    if(argc<5){
        printf("format of input = a.out || [Maximum number of instances available] || [Maximum number of threads to use in the simulation.] || [Deadlock detection check interval d in seconds.] || name of the instances ");
        exit(-1);
    }
    max_instances=stoi(argv[0]);
    max_threads=stoi(argv[1]);
    deadlock_detection_interval=stoi(argv[2]);
    array_of_resources=(int *)malloc(sizeof(int)*max_instances);
}