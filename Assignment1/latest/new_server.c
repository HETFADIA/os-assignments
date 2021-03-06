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

#define LISTEN_BACKLOG 100
#define MAX_QUEUE_SIZE 100
#define MAX_REQUEST_THREADS 100
int memlimit;
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
    return temp->data;
}

int thread_count = 0;

void msg(char *msg, bool ex){
    printf("%s\n", msg);
    if(ex) exit(-1);
}

int getusage(){
    int who = RUSAGE_SELF; 
    struct rusage usage; 
    getrusage(who,&usage);
    return usage.ru_maxrss;
}

void DLL_handler_module(char *ch){
    printf("execute %s\n", ch);
    char arr[5][501];
    memset(arr, '\0', sizeof(arr));
    int i = 0, tot = 0;
    while(true){
        int j = 0;
        while(ch[tot] != '\0' && ch[tot] != '#'){
            arr[i][j] = ch[tot];
            tot += 1;
            j += 1;
        }
        i += 1;
        if(ch[tot] == '\0') break;
        tot += 1;
    }

    double (*f1)(double);
    void *handle = NULL;
    char *err;
    while(handle==NULL){
        handle = dlopen(arr[0], RTLD_LAZY);
        err = dlerror();
        if(handle==NULL){
            int len = strlen(err);
            if(err[len-1]=='s' && err[len-2]=='e' && err[len-3]=='l' && err[len-4]=='i' && err[len-5]=='f')
                printf("Too many open files\n");
            else{
                return;
            }
        }
    }
    f1 = dlsym(handle, arr[1]);
    printf("Reached before if\n");
    if(f1==NULL){
        printf("f1 is NULL, %s\n", strerror(errno));
        return;
    }
    float nicevar = atof(arr[2]);
    float anothervar = f1(nicevar);
    printf("%f request response\n", anothervar);
    dlclose(handle);
}

void *func(void *p_client){
    char chararr[5001];
    int _client_socket = *((int *)p_client);
    free(p_client);
    char *USE = "#";
    char *SE = "#";
    printf("enqueuing client requests begins..\n");

    while(true){
        //read the requests from a client one by one and start queuing them up for the dispatcher

        char *request = (char *)malloc(sizeof(char) * 5001);
        memset(chararr, '\0', sizeof(chararr));
        memset(request, '\0', sizeof(char) * 5001);
        int _recv_status = recv(_client_socket, chararr, sizeof(chararr), 0);

        for(int i = 0; i < strlen(chararr); i++){
            request[i] = chararr[i];
        }
        if(_recv_status <= 0){
            break;
        }
        pthread_mutex_lock(&mutex);
        int enqueue_status = enqueue(request);
        pthread_mutex_unlock(&mutex);
        //mutex

        printf("sending response about succesful or unsuccesful enqueue back to the client.\n");
        if(enqueue_status < 0){
            write(_client_socket, USE, sizeof(USE));
        }
        else{
            write(_client_socket, SE, sizeof(SE));
        }
    }
    pthread_mutex_lock(&mutex);
    thread_count += 1;
    pthread_mutex_unlock(&mutex);
    //mutex
    close(_client_socket);
    pthread_exit(NULL);
    return NULL;
}

void *dispatcher_function(void *arg){
    printf("%d\n",getusage());
    while(true){
        if(getusage()>memlimit){
            printf("Memory limit exceeded\n");
            continue;
        }
        pthread_mutex_lock(&mutex);
        char *request = dequeue();
        pthread_mutex_unlock(&mutex);
        if(request != NULL){
            DLL_handler_module(request);
        }
    }
    return NULL;
}

void make_server(int PORT, int thread_limit_dispatcher, int file_limit, int memory_limit){
    printf("%d %d %d %d", PORT, thread_limit_dispatcher, file_limit, memory_limit);
    struct rlimit lim, new_lim;
    lim.rlim_cur = file_limit;
    lim.rlim_max = 1024;
    if(setrlimit(RLIMIT_NOFILE, &lim) == -1)
        fprintf(stderr, "%s\n", strerror(errno));

    memlimit = memory_limit;

    pthread_t arr[thread_limit_dispatcher];
    for(int i = 0; i < thread_limit_dispatcher; i += 1){
        pthread_create(&arr[i], NULL, dispatcher_function, NULL);
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
        int _client_socket = accept(_socket, (SA *) &client_addr, &IN_size);

        if(thread_count <= 0){
            printf("Thread count at its limit\n");
            continue;
        }
        printf("Client accepted\n");
        pthread_mutex_lock(&mutex);
        thread_count -= 1;
        pthread_mutex_unlock(&mutex);
        //mutex

        pthread_t t;
        int *p_client = (int *)malloc(sizeof(int));
        *p_client = _client_socket;
        pthread_create(&t, NULL, func, p_client);
    }
}

int main(int argc, char **argv){
    if(argc != 5){
        printf(".\a.out [PORT] [THREAD_LIMIT_DISPATCHER not for client] [FILE_LIMIT not for client] [MEMORY_LIMIT not for client]\n");
        exit(-1);
    }
    make_server(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
}
