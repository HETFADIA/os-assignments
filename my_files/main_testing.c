#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stddef.h>
#include <dlfcn.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
char separator = '?';
#define LISTEN_BACKLOG 100
#define MAX_REQUEST_THREADS 100
int memlimit;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct sockaddr_in IN;
typedef struct sockaddr SA;

char *queue[100];
int start = 0;
int end = 0;
int qsize = 100;
int currsize = 0;
bool enqueue(char *s)
{
    if (currsize >= qsize)
    {
        return 0;
    }
    int slen = strlen(s);
    queue[end] = (char *)malloc(sizeof(char) * (slen + 1));
    for (int i = 0; i <= slen; i++)
    {
        if (i < slen)
        {
            queue[end][i] = s[i];
        }
        else
        {
            queue[end][i] = '\0';
        }
    }
    end = (end + 1) % qsize;
    currsize++;
    return 1;
}
char *dequeue()
{
    if (currsize == 0)
    {
        return NULL;
    }
    currsize--;
    return queue[(start++) % qsize];
}

int thread_count = 0;

void *request_server(void *p_client);
bool DLL_handler_module(char *ch);
int getusage();
void *dispatcher_of_thread(void *arg);

int getusage()
{
    int who = RUSAGE_SELF;
    struct rusage usage;
    getrusage(who, &usage);
    return usage.ru_maxrss;
}

bool DLL_handler_module(char *ch)
{
    if (ch == NULL)
    {
        return 0;
    }
    char arr[501][501];
    int deserializelen = 0;
    int counter = 0;
    memset(arr, '\0', sizeof(arr));
    int serializelen = strlen(ch);

    for (int i = 0; i < serializelen; i++)
    {
        if (ch[i] == separator)
        {
            deserializelen++;
            counter = 0;
            continue;
        }
        arr[deserializelen][counter++] = ch[i];
    }
    deserializelen++;
    
    void *handle = NULL;
    char *err;
    char *filesstr = "files";
    while (handle == NULL)
    {
        handle = dlopen(arr[0], RTLD_LAZY);
        err = dlerror();
        
        if (!handle)
        {
            int len = strlen(err);
            bool many_files_open = 1;
            
            if (len >= 5)
            {

                for (int i = 0; i < 5; i++)
                {
                    if (filesstr[i] != err[len - 5 + i])
                    {
                        many_files_open = 0;
                    }
                }
            }
            
            if (many_files_open)
            {

                printf("Too many open files\n");
            }
            else
            {
                printf("%s\n", err);
                return 0;
            }
        }
    }
    
    //FILE * out=fopen("output.txt","w");
    double result;
    if (strcmp(arr[1], "hypot") != 0 && strcmp(arr[1], "pow") != 0)
    {
        double (*f1)(double);
        f1 = dlsym(handle, arr[1]);
        if (!f1)
        {
            printf("The requested function does not exist\n");
            return 0;
        }
        double var1 = atof(arr[2]);
        result = f1(var1);
    }
    else
    {
        double (*f2)(double, double);
        f2 = dlsym(handle, arr[1]);
        if (!f2)
        {
            printf("The requested function does not exist\n");
            return 0;
        }
        double var1 = atof(arr[2]);
        double var2 = atof(arr[3]);
        result = f2(var1, var2);
    }
    /*
    fprintf(out,"%s %f",ch,result);
    fclose(out);
    */
    printf("%f <- request response\n", result);
    dlclose(handle);
    return 1;
}

void *dispatcher_of_thread(void *arg)
{
    while (1)
    {
        int memory_curr_used = getusage();
        bool memory_more_used = memory_curr_used > memlimit;
        if (memory_more_used)
        {
            printf("Memory limit exceeded\n");
            continue;
        }
        pthread_mutex_lock(&mutex);
        char *request = dequeue();
        pthread_mutex_unlock(&mutex);
        if (request)
        {
            printf("%s\n", request);
            DLL_handler_module(request);
        }
    }
    return NULL;
}

void make_server(int PORT, int thread_maxlimit, int openfile_limit, int memory_limit)
{

    struct rlimit lim;
    lim.rlim_cur = openfile_limit;
    lim.rlim_max = 1024;
    bool not_possible = (setrlimit(RLIMIT_NOFILE, &lim) == -1);
    struct rlimit new_lim;

    if (not_possible)
    {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(-1);
    }

    pthread_t arr[thread_maxlimit];
    memlimit = memory_limit;

    for (int i = 0; i < thread_maxlimit; ++i)
    {
        pthread_create(&arr[i], NULL, dispatcher_of_thread, NULL);
    }
    int _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket < 0)
    {
        printf("\nSOCKET CREATION UNSUCCESSFUL\n");
    }
    thread_count = MAX_REQUEST_THREADS;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int IN_size = sizeof(struct sockaddr_in);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    int _bind_status = bind(_socket, (SA *)&server_addr, sizeof(struct sockaddr_in));
    if (_bind_status < 0)
    {
        printf("\nthe bind could not be done\n");
    }
    int _listen_status = listen(_socket, LISTEN_BACKLOG);
    if (_listen_status < 0)
    {
        printf("\nlisten failed\n");
    }
    while (1)
    {

        printf("Waiting for the client to accept\n");
        int _client_socket = accept(_socket, (SA *)&client_addr, &IN_size);

        if (thread_count <= 0)
        {
            printf("\n");
            printf("Thread count at its limit\n");
            continue;
        }
        printf("\n the client accepted your request");
        printf("\n");
        pthread_mutex_lock(&mutex);
        --thread_count;
        pthread_mutex_unlock(&mutex);
        //mutex

        pthread_t new_thread;
        int sizeofint = sizeof(int);
        int *p_client = (int *)malloc(sizeofint);
        *p_client = _client_socket;
        pthread_create(&new_thread, NULL, request_server, p_client);
    }
}
void *request_server(void *p_client)
{
    char *request = (char *)malloc(sizeof(char) * 5001);
    char chararr[5001];
    int _client_socket = *((int *)p_client);
    free(p_client);
    char *USE = "?", *SE = "?";
    printf("\n");
    printf("enqueuing client requests begins..\n");
    while (1)
    {

        memset(request, '\0', sizeof(char) * 5001);
        memset(chararr, '\0', sizeof(chararr));
        int _recv_status = recv(_client_socket, chararr, sizeof(chararr), 0);
        if (_recv_status <= 0)
        {
            break;
        }
        int chararrlen = strlen(chararr);
        for (int i = 0; i < chararrlen; i++)
        {
            request[i] = chararr[i];
        }
        pthread_mutex_lock(&mutex);
        bool enqueue_status = enqueue(request);
        pthread_mutex_unlock(&mutex);
        //mutex

        printf("sending response about succesful or unsuccesful enqueue back to the client.\n");
        if (enqueue_status == 0)
        {
            write(_client_socket, USE, sizeof(USE));
        }
        else
        {
            write(_client_socket, SE, sizeof(SE));
        }
    }
    pthread_mutex_lock(&mutex);
    ++thread_count;
    pthread_mutex_unlock(&mutex);

    close(_client_socket);
    pthread_exit(NULL);
    return NULL;
}
void unit_testing(int thread_limit, int open_file_limit, int memory_limit);
int main(int argc, char **argv)
{
    if (argc < 5)
    {
        printf("\nToo less arguments\n");
    }
    else if (argc > 5)
    {
        printf("\nToo many arguments\n");
    }
    if (argc != 5)
    {

        printf(".\a.out [PORT] [THREAD_LIMIT_DISPATCHER] [openfile_limit] [MEMORY_LIMIT]\n");
        exit(-1);
    }
    char *testing = "test";
    int testinglen = strlen(testing);
    int argv1len = strlen(argv[1]);
    bool strequals = 1;
    if (testinglen == argv1len)
    {

        for (int i = 0; i < 4; i++)
        {
            if (argv[1][i] != testing[i])
            {
                strequals = 0;
            }
        }

        if (strequals)
        {
            int b = atoi(argv[2]), c = atoi(argv[3]), d = atoi(argv[4]);
            unit_testing(b, c, d);
        }
    }
    if (strequals)
    {
        return 0;
    }
    int a = atoi(argv[1]), b = atoi(argv[2]), c = atoi(argv[3]), d = atoi(argv[4]);
    if (b <= 0)
    {
        printf("\nError no threads available\n");
    }
    else if (c <= 5)
    {
        printf("\nplease provide as atleast 6 files must be specified\n");
    }
    else if (d <= 6000)
    {
        printf("\nPlease increase the memory for the program\n");
    }
    else
    {

        make_server(a, b, c, d);
    }
}
int ithtest = 1;
void passing_empty_in_dll()
{
    printf("Test %d", ithtest++);
    printf("\ncalling handler module with empty string\n");
    DLL_handler_module(NULL);

    printf("no error occured\n\n\n");
}
void testing_incorrect_inputpath(char *incorrect)
{
    
    printf("Test %d", ithtest++);
    printf("\nTesting incorrect path\n");
    DLL_handler_module(incorrect);
    printf("no error occured\n\n\n");
}
void testing_incorrect_inputfunctionname(char * incorrect){
    printf("Test %d", ithtest++);
    printf("\nTesting incorrect function name\n");
    DLL_handler_module(incorrect);
    printf("no error occured\n\n\n");
}
/*
void testing_incorrect_arguments(char * incorrect){
    printf("Test %d", ithtest++);
    printf("\nTesting incorrect arguments name\n");
    DLL_handler_module(incorrect);
    printf("\n\n");
}
*/
void testing_correct_input(char * correct){
    printf("Test %d", ithtest++);
    printf("\nTesting correct function name\n");
    DLL_handler_module(correct);
    printf("no error occured\n\n\n");
}
void opening_more_files(){
    
}
void unit_testing(int thread_limit, int open_file_limit, int memory_limit)
{
    passing_empty_in_dll();
    testing_incorrect_inputpath("error/lib/x86_64-linux-gnu/libm.so.6?cos?2");
    testing_incorrect_inputfunctionname("/lib/x86_64-linux-gnu/libm.so.6?errors?2");
    //testing_incorrect_inputfunctionname("/lib/x86_64-linux-gnu/libm.so.6?cos?a?b");
    testing_correct_input("/lib/x86_64-linux-gnu/libm.so.6?tan?2");

}
