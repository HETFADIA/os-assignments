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


typedef struct sockaddr_in IN;
typedef struct sockaddr SA;


void msg(char *msg, bool exit){
    printf("%s\n", msg);
    if(exit) exit(-1);
}

void *func(void *p_client){
    //client socket == *arg
    int _client_socket = *((int *)p_client);
    free(p_client);
    char *USE = "Unsuccesful enqueue\n";
    char *SE = "Succesful enqueue\n";
    while(true){
        char *request = (char *)malloc(sizeof(char) * 5001);
        memset(request, '\0', sizeof request);
        int _recv_status = recv(_client_socket, request, strlen(request), 0);
        if(_recv_status < 0){
            break;
        }
        pthread_mutex_lock(&mutex);
        int enqueue_status = enqueue(request);
        pthread_mutex_unlock(&mutex);
        //mutex

        if(enqueue_status < 0){
            write(_client_socket, USE, strlen(USE));
        }
        else{
            write(_client_socket, SE, strlen(SE));
        }
    }
    pthread_mutex_lock(&mutex);
    thread_count += 1;
    pthread_mutex_unlock(&mutex);
    //mutex
    pthread_exit(NULL);
    return NULL;
}

void *dispatcher_function(void *arg){
    while(true){
        char *request = dequeue();
        if(request != NULL){
           printf("%s\n", request);
           free(request);
        }
    }
    return NULL;
}

void make_server(int PORT){

    int _socket = socket(AF_INET, SOCK_STREAM, 0);
    if(_socket < 0){
        msg("SOCKET CREATION UNSUCCESSFUL", true);
    }
    int IN_size = sizeof(IN);
    IN server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int _connect_status = connect(_socket, (SA *)&server_addr, IN_size);
    if(_connect_status < 0) {
        msg("Connect failed", true);
    }
    for(int i=0;i<10;i++){
        char buff[500];
        strncpy(buff,"Hello World", sizeof(buff));
        write(_socket, buff, sizeof(buff));
    }
    
}

int main(int argc, char **argv){
    if(argc < 2){
        printf(".\a.out [PORT] [THREAD_LIMIT_DISPATCHER not for client] [FILE_LIMIT not for client] [MEMORY_LIMIT not for client]\n");
        exit(-1);
    }
    make_server(atoi(argv[1]));
}
