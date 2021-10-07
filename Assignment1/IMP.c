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

#define LISTEN_BACKLOG 100
#define MAX_QUEUE_SIZE 100
#define MAX_REQUEST_THREADS 100
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct sockaddr_in IN;
typedef struct sockaddr SA;

struct node{
    char *data;
    struct node *next;
};

int queue_size = 0;
struct node *front = NULL;
struct node *back = NULL;

int enqueue(char *x){
    if(queue_size >= MAX_QUEUE_SIZE){
        return -1;
    }
    struct node *temp = (struct node *)malloc(sizeof(struct node));
    temp->data = x;
    temp->next = NULL;
    if(back == NULL)
        front = temp;
    else
        back->next = temp;
    back = temp;
    queue_size += 1;
    return 0;
}
char *dequeue(){
    if(front == NULL){
        return NULL;
    }
    struct node *temp = front;
    front = front->next;
    if(front == NULL)
        back = NULL;
    temp->next = NULL;
    queue_size -= 1;
    return temp;
}

int thread_count = 0;

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

void make_server(int PORT, int thread_limit_dispatcher, int file_limit, int memory_limit){

    pthread_t arr[thread_limit_dispatcher];
    for(int i = 0; i < thread_limit_dispatcher; i += 1){
        pthread_create(arr + i, NULL, dispatcher_function, NULL);
    }
    thread_count = MAX_REQUEST_THREADS;
    int _socket = socket(AF_INET, SOCK_STREAM, 0);
    if(_socket < 0){
        msg("SOCKET CREATION UNSUCCESSFUL", true);
    }
    int IN_size = sizeof(IN);
    IN server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int _bind_status = bind(_socket, (SA *)&server_addr, IN_size);
    if(_bind_status < 0) {
        msg("Bind failed", true);
    }
    int _listen_status = listen(_socket, LISTEN_BACKLOG);
    if(_listen_status  < 0){
        msg("listen failed", true);
    }
    while(1) {

        printf("Waiting.....\n");
        int _client_socket = accept(_socket, (SA *) &client_addr, (socklen_t *)IN_size);

        if(thread_count <= 0){
            printf("Thread count at its limit\n");
            continue;
        }
        pthread_mutex_lock(&mutex);
        thread_count -= 1;
        pthread_mutex_unlock(&mutex);
        //mutex

        pthread_t t;
        int *p_client = (int *)malloc(sizeof int);
        *p_client = _client_socket;
        pthread_create(&t, NULL, func, p_client);
    }
}

int main(int argc, char **argv){
    if(argc < 3){
        printf(".\a.out [PORT] [THREAD_LIMIT_DISPATCHER not for client] [FILE_LIMIT not for client] [MEMORY_LIMIT not for client]\n");
        exit(-1);
    }
    make_server(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
}