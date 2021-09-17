#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int memlimit = 0;

pthread_mutex_t mutex_queue = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_client_thread = PTHREAD_MUTEX_INITIALIZER;

typedef struct sockaddr_in sckadd_in;
typedef struct sockaddr sckadd;

int queue_size = 0;

struct node{
    char data[500];
    struct node *next;
};


struct node *head = NULL;
struct node *tail = NULL;

void enqueue(char *req){
    if(queue_size >= 100){  //Max queue size is 100
        printf("Queue limit reached\n");
        return;
    }
    struct node *n = (struct node *)malloc(sizeof(struct node));
    // n->data = req;
    strncpy(n->data, req, 500);
    n->next = NULL;
    if(tail == NULL){
        head = n;
        tail = n;
    }
    else{
        tail->next = n;
        tail = n;
    }
    queue_size++;
}
char *dequeue(){
    if(head == NULL){
        return NULL;
    }
    else{
        // char res = head->data;
        char *res = (char*)malloc(sizeof(char)*500);
        strncpy(res, head->data, 500);
        struct node *n = head;
        head = head->next;
        if(head == NULL){
            tail = NULL;
        }
        free(n);
        queue_size--;
        return res;
    }
}

int client_thread = 0;

int getusage(){ 
    struct rusage usage; 
    getrusage(RUSAGE_SELF,&usage);
    return usage.ru_maxrss;
}

void DLL(char *req){

    if(req==NULL) return;
     printf("Request dequeued: %s\n",req);
    char library[100];
    char func[100];
    char var1[100];
    char var2[100];
    memset(library, '\0', sizeof(library));
    memset(func, '\0', sizeof(library));
    memset(var1, '\0', sizeof(library));
    memset(var2, '\0', sizeof(library));
    int x = 0;
    for(int i=0; req[x]!='$';){
        library[i++] = req[x++];
    }
    x++;
    for(int i=0; req[x]!='$';){
        func[i++] = req[x++];
    }
    x++;
    for(int i=0; req[x]!='$' && req[x]!='\0';){
        var1[i++] = req[x++];
    }
    x++;
    for(int i=0; req[x]!='$' && req[x]!='\0';){
        var2[i++] = req[x++];
    }
    void *handle = NULL;
    char *err;
    while(handle==NULL){
        handle = dlopen(library, RTLD_LAZY);
        err = dlerror();
        int len;
        if(handle==NULL){
            len = strlen(err);
            if(err[len-5]=='f' && err[len-4]=='i' && err[len-3]=='l' && err[len-2]=='e' && err[len-1]=='s')
                printf("Too many files are open\n");
            else{
                printf("Request error: %s\n", err);
                return;
            }
        }
    }
    if(strcmp(func, "pow") == 0){
        double (*fn)(double, double);
        fn = dlsym(handle, func);
        if(fn==NULL){
            printf("Error in requested function: %s\n",func);
            return;
        }
        else{
            float result;
            float v1= atof(var1);
            float v2 = atof(var2);
            result = fn(v1, v2);
            printf("%s ------> %f\n", req, result);
        }
    }
    else if(strcmp(func, "remainder") == 0){
        double (*fn)(double, double);
        fn = dlsym(handle, func);
        if(fn==NULL){
            printf("Error in requested function: %s\n",func);
            return;
        }
        else{
            float result;
            float v1= atof(var1);
            float v2 = atof(var2);
            result = fn(v1, v2);
            printf("%s ------> %f\n", req, result);
        }
    }
    else{
        double (*fn)(double);
        fn = dlsym(handle, func);
        if(fn==NULL){
            printf("Error in requested function: %s\n",func);
            return;
        }
        else{
            float result;
            float v1 = atof(var1);
            result = fn(v1);
            printf("%s ------> %f\n", req, result);
        }
    }
    dlclose(handle);
}

void *getrequest(void *cl_socket){
    int client_socket = *((int *)cl_socket);
    free(cl_socket);
    char *response = "@!";
    char request[500];
    while(1){
        memset(request, '\0', sizeof(char) * 500);

        int recv_status = recv(client_socket, request, sizeof(char)*500, 0);

        if(recv_status <= 0){
            break;
        }
        if(strlen(request)==0)
            continue;

        pthread_mutex_lock(&mutex_queue);
        enqueue(request);
        pthread_mutex_unlock(&mutex_queue);

        write(client_socket, response, sizeof(response));
    }
    pthread_mutex_lock(&mutex_client_thread);
    client_thread++;
    pthread_mutex_unlock(&mutex_client_thread);
    close(client_socket);
    pthread_exit(NULL);
    return NULL;
}

void *dispatcher(void *arg){
    while(1){
        if(getusage()>memlimit){
            printf("Memory limit exceeded, process waiting\n");
            continue;
        }
        pthread_mutex_lock(&mutex_queue);
        char *request = dequeue();
        pthread_mutex_unlock(&mutex_queue);   
        DLL(request);
    }
    return NULL;
}

void initialize(int port, int thread_lim, int file_lim, int mem_lim){

    struct rlimit lim, new_lim;
    lim.rlim_cur = file_lim;
    lim.rlim_max = 1024;
    if(setrlimit(RLIMIT_NOFILE, &lim) == -1){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(-1);
    }

    memlimit = mem_lim;

    client_thread = 100; //Max request threads

    pthread_t arr[thread_lim];

    for(int i = 0; i < thread_lim; i++){
        pthread_create(&arr[i], NULL, dispatcher, NULL);
    }
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0){
        printf("Socket could not be created\n");
        exit(-1);
    }
    int in_size = sizeof(sckadd_in);
    sckadd_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_socket, (sckadd *)&server_addr, in_size) < 0) {
        printf("Bind failed");
        exit(-1);
    }
     //Max listen backlog is 100
    if(listen(server_socket, 100) < 0){
        printf("Listen failed");
        exit(-1);
    }
    while(1){

        printf("Waiting for new client\n");
        int client_socket = accept(server_socket, (sckadd *) &client_addr, &in_size);

        if(client_thread <= 0){
            printf("Client connection limit reached\n");
            continue;
        }

        printf("Client connected\n");
        pthread_t thread;
        int *cl_socket = (int *)malloc(sizeof(int));
        *cl_socket = client_socket;

        pthread_mutex_lock(&mutex_client_thread);
        client_thread--;
        pthread_mutex_unlock(&mutex_client_thread);

        pthread_create(&thread, NULL, getrequest, cl_socket);
    }
}

void unit_test(){

    printf("\nTest 1: null value in DLL\n");
    //Pass NULL into DLL
    DLL(NULL);
    printf("Passed\n");

    printf("\nTest 2: Normal function with 1 argument\n");
    //Try valid function which takes one argument
    printf("sin(2), Expected: -0.756802\n");
    char *test2 = "/lib/x86_64-linux-gnu/libm.so.6$sin$4";
    DLL(test2);

    printf("\nTest 3\n");
    //Invalid File Path
    char *test3 = "bogus$sin$4";
    DLL(test3);

    printf("\nTest 4\n");
    //Invalid Function
    char *test4 = "/lib/x86_64-linux-gnu/libm.so.6$bogus$4";
    DLL(test4);

    printf("\nTest 5\n");
    //Invalid Values
    printf("log(-200), Expected: nan\n");
    char *test5 = "/lib/x86_64-linux-gnu/libm.so.6$log$-200";
    DLL(test5);

    printf("\nTest 6: Normal function with 2 arguments\n");
    //Try valid function which takes two arguments
    printf("pow(2,10), Expected: 1024\n");
    char *test6 = "/lib/x86_64-linux-gnu/libm.so.6$pow$2$10";
    DLL(test6);

    printf("\nTest 7: Test dequeue with empty queue(No output expected)\n");
    //Try to dequeue with empty queue
    char *test7 = dequeue();

    printf("\nTest 8: Overfill queue by 3\n");
    //Queue limit should be 100, it should not cross 100;
    for(int i=0;i<103;i++){
        char *test8 = "Bogus";
        enqueue(test8);
    }
    printf("Current queue size: %d\n", queue_size);

    printf("\n------------------------------\n");
    printf("Final verdict: All tests passed\n");
}

int main(int argc, char **argv){
    if(atoi(argv[1]) == 0){
        unit_test();
        exit(1);
    }
    if(argc != 5){
        printf("Invalid arguments!\n");
        printf("./server <Port> <Thread Pool> <File Limit> <Memory Limit>\n");
        exit(-1);
    }
    if(atoi(argv[1])<0 || atoi(argv[2])<1 || atoi(argv[3])<3 || atoi(argv[4])<getusage()){
        printf("Invalid arguments, could not create server\n");
        exit(-1);
    }
    printf("Making server\n");
    initialize(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
}
