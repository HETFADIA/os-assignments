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
void unit_test();

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

    if(!ch) return;
    char arr[10][501];
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
    double (*f2)(double, double);
    void *handle = NULL;
    char *err;
    while(handle==NULL){
        handle = dlopen(arr[0], RTLD_LAZY);
        err = dlerror();
        if(!handle){
            int len = strlen(err);
            if(err[len-1]=='s' && err[len-2]=='e' && err[len-3]=='l' && err[len-4]=='i' && err[len-5]=='f')
                printf("Too many open files\n");
            else{
                printf("%s\n", err);
                return;
            }
        }
    }
    float result;
    if(strcmp(arr[1], "hypot") != 0 && strcmp(arr[1], "pow") != 0){
        f1 = dlsym(handle, arr[1]);
        if(!f1){
            printf("The requested function does not exist\n");
            return;
        }
        float var1 = atof(arr[2]);
        result = f1(var1);
    }
    else{
        f2 = dlsym(handle, arr[1]);
        if(!f2){
            printf("The requested function does not exist\n");
            return;
        }
        float var1 = atof(arr[2]);
        float var2 = atof(arr[3]);
        result = f2(var1, var2);
    }
    printf("%f <- request response\n", result);
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
    while(true){
        if(getusage()>memlimit){
            printf("Memory limit exceeded\n");
            continue;
        }
        pthread_mutex_lock(&mutex);
        char *request = dequeue();
        pthread_mutex_unlock(&mutex);
        if(request != NULL){
            printf("%s\n", request);
            DLL_handler_module(request);
        }
    }
    return NULL;
}

void make_server(int PORT, int thread_limit_dispatcher, int file_limit, int memory_limit){
    if(PORT <= 0 || thread_limit_dispatcher <= 0 || file_limit <= 3 || memory_limit < 10000){
        msg("invalid arguments \n", true);
    }
    struct rlimit lim, new_lim;
    lim.rlim_cur = file_limit;
    lim.rlim_max = 1024;
    if(setrlimit(RLIMIT_NOFILE, &lim) == -1){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(-1);
    }

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
        printf(".\a.out [PORT and 0 for basic unit test run] [THREAD_LIMIT_DISPATCHER not for client] [FILE_LIMIT not for client] [MEMORY_LIMIT not for client]\n");
        exit(-1);
    }
    if(atoi(argv[1]) == 0){
        unit_test();
        exit(0);
    }
    make_server(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
}

void unit_test(){

    // wrong args in CLI
    // such as negative file number etc.
    // make_server(3000, 10, -1, 100000);


    //DLL_handler_module check
    //1. what if input NULL
    DLL_handler_module(NULL); //doesn't do anything

    //2. passing a valid string of the format described in the README
    char *ch = "/lib/x86_64-linux-gnu/libm.so.6#cos#2";
    //format is "file_path" + "#" + "cos" + "#" + "2";
    DLL_handler_module(ch);

    //3. invalid file path
    ch = "/does_not_exist#cos#2";
    DLL_handler_module(ch);

    //4. invalid function
    ch = "/lib/x86_64-linux-gnu/libm.so.6#does_not_exist#2";
    DLL_handler_module(ch);

    //if any of the above cases are not handled the program may crash

    //dequeue and enqueue test
    //1. checking for saturation of queue
    for(int i = 0; i < 101; i++){
        char *ch1 = (char *)malloc(sizeof(char) * 100);
        ch1 = "enqueue";
        int enqueue_status = enqueue(ch1);
        if(enqueue_status == -1){
            printf("Queue full as the queue has a max size of 100\n");
        }
        //enqueue status being 0 implies success
    }

    //2. check if queue_size == 100
    if(queue_size ==   100){
        printf("Queue size correctly identified\n");
    }
    else{
        printf("Queue size is incorrect\n");
    }

    //3. checking for dequeue and handling of underflow condition
    char *to_compare = "enqueue";
    for(int i = 0; i < 101; i++){
        char *ch1 = dequeue();
        if(!ch1){
            printf("Dequeue attempt of elements from an empty queue\n");
            continue;
        }
        if(strcmp(ch1, to_compare) != 0){
            printf("enqueue/dequeue failed");
        }
    }
    // this concludes all functions that can be check separate from the server

    //msg ends server if passed true
    msg("Check that it ends/kills the server", true);
}
