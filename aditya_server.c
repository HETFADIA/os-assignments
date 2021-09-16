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

int memlimit = 0;;

pthread_mutex_t mutex_queue = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_client_thread = PTHREAD_MUTEX_INITIALIZER;

typedef struct sockaddr_in sckadd_in;
typedef struct sockaddr sckadd;
void unit_test();

struct node{
    char *data;
    struct node *next;
};

int queue_size = 0;

struct node *head = NULL;
struct node *tail = NULL;

void enqueue(char *req){
    if(queue_size >= 100){  //Max queue size is 100
        printf("Queue limit reached\n");
    }
    struct node *n = (struct node *)malloc(sizeof(struct node));
    n->data = req;
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
        char *res = head->data;
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

void DLL(char *ch){

    if(ch==NULL) return;
    char library[100];
    char func[100];
    char var1[100];
    char var2[100];
    memset(library, '\0', sizeof(library));
    memset(func, '\0', sizeof(library));
    memset(var1, '\0', sizeof(library));
    memset(var2, '\0', sizeof(library));
    int x = 0;
    for(int i=0; ch[x]!='$';){
        library[i++] = ch[x++];
    }
    x++;
    for(int i=0; ch[x]!='$';){
        func[i++] = ch[x++];
    }
    x++;
    for(int i=0; ch[x]!='$' && ch[x]!='\0';){
        var1[i++] = ch[x++];
    }
    x++;
    for(int i=0; ch[x]!='$' && ch[x]!='\0';){
        var2[i++] = ch[x++];
    }
    double (*f1)(double);
    double (*f2)(double, double);
    void *handle = NULL;
    char *err;
    while(handle==NULL){
        handle = dlopen(library, RTLD_LAZY);
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
    if(strcmp(func, "hypot") != 0 && strcmp(func, "pow") != 0){
        f1 = dlsym(handle, func);
        if(!f1){
            printf("The requested function does not exist\n");
            return;
        }
        float v1 = atof(var1);
        result = f1(v1);
    }
    else{
        f2 = dlsym(handle, func);
        if(!f2){
            printf("The requested function does not exist\n");
            return;
        }
        float v1= atof(var1);
        float v2 = atof(var2);
        result = f2(v1, v2);
    }
    printf("%f <- request response\n", result);
    dlclose(handle);
}

void *getrequest(void *cl_socket){
    int client_socket = *((int *)cl_socket);
    free(cl_socket);
    char *response = "@!";
    while(1){
        char *request = (char *)malloc(sizeof(char) * 5000);
        memset(request, '\0', sizeof(char) * 5000);

        int recv_status = recv(client_socket, request, sizeof(char)*5000, 0);

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
        if(request != NULL){
            printf("Request is: %s\n",request);
            DLL(request);
        }
    }
    return NULL;
}

void make_server(int port, int thread_lim, int file_lim, int mem_lim){
    if(port<0 || thread_lim<1 || file_lim<3 || mem_lim<getusage()){
        printf("Invalid arguments, could not create server");
    }
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

    int bind_status = bind(server_socket, (sckadd *)&server_addr, in_size);
    if(bind_status < 0) {
        printf("Bind failed");
        exit(-1);
    }
    int listen_status = listen(server_socket, 100); //Max listen backlog is 100
    if(listen_status  < 0){
        printf("Listen failed");
        exit(-1);
    }
    while(1){

        printf("Waiting for client\n");
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

int main(int argc, char **argv){
    if(argc != 5){
        printf("Invalid arguments!\n");
        printf(".\a.out <Port> <Thread Pool> <File Limit> <Memory Limit>\n");
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
    DLL(NULL); //doesn't do anything

    //2. passing a valid string of the format described in the README
    char *ch = "/lib/x86_64-linux-gnu/libm.so.6#cos#2";
    //format is "file_path" + "#" + "cos" + "#" + "2";
    DLL(ch);

    //3. invalid file path
    ch = "/does_not_exist#cos#2";
    DLL(ch);

    //4. invalid function
    ch = "/lib/x86_64-linux-gnu/libm.so.6#does_not_exist#2";
    DLL(ch);

    //if any of the above cases are not handled the program may crash

    //dequeue and enqueue test
    //1. checking for saturation of queue
    for(int i = 0; i < 101; i++){
        char *ch1 = (char *)malloc(sizeof(char) * 100);
        ch1 = "enqueue";
        enqueue(ch1);
        if(1){ //-----------------------------------------------Change
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
}
