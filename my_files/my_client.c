#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

int main(int argc, char ** argv){
    if(argc==1){
        printf("Write ./a.out then [PORT] of the server");
    }
    else if(argc>2){
        printf("Write ./a.out then [PORT] of the server");
    }
    else{
        
    }

}
