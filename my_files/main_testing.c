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

int threads_counter = 0;

void *request_server(void *p_client);
bool dll_handler_function(char *array_of_character);
int get_memory_usage();
void *dispatcher_of_thread(void *arg);

int get_memory_usage()
{
    struct rusage usage;
    int who = RUSAGE_SELF;
    getrusage(who, &usage);
    return usage.ru_maxrss;
}

bool dll_handler_function(char *array_of_character)
{
    if (array_of_character == NULL)
    {
        return 0;
    }
    char deserialize[501][501];
    int deserializelen = 0;
    int counter = 0;
    memset(deserialize, '\0', sizeof(deserialize));
    int serializelen = strlen(array_of_character);

    for (int i = 0; i < serializelen; i++)
    {
        if (array_of_character[i] == separator)
        {
            deserializelen++;
            counter = 0;
            continue;
        }
        deserialize[deserializelen][counter++] = array_of_character[i];
    }
    deserializelen++;

    void *handle = NULL;
    char *err;
    char *filesstr = "files";
    while (handle == NULL)
    {
        handle = dlopen(deserialize[0], RTLD_LAZY);
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
    if (deserializelen == 3)
    {
        double (*function1)(double);
        function1 = dlsym(handle, deserialize[1]);
        if (!function1)
        {
            printf("The requested function does not exist\n");
            return 0;
        }
    
        result = function1(atof(deserialize[2]));
    }
    else if (deserializelen == 4)
    {
        double (*function2)(double, double);
        function2 = dlsym(handle, deserialize[1]);
        if (!function2)
        {
            printf("The requested function does not exist\n");
            return 0;
        }
        result = function2(atof(deserialize[2]), atof(deserialize[3]));
    }
    else if (deserializelen == 5)
    {
        double (*function3)(double, double, double);
        function3 = dlsym(handle, deserialize[1]);
        if (!function3)
        {
            printf("The requested function does not exist\n");
            return 0;
        }
        
        result = function3(atof(deserialize[2]), atof(deserialize[3]),atof(deserialize[4]));
    }
    
    printf("%f is obtained as the responce of the request\n", result);
    dlclose(handle);
    return 1;
}

void *dispatcher_of_thread(void *arg)
{
    while (1)
    {
        int memory_curr_used = get_memory_usage();
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
            dll_handler_function(request);
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

    pthread_t thread_pool[thread_maxlimit];
    memlimit = memory_limit;

    for (int i = 0; i < thread_maxlimit; ++i)
    {
        pthread_create(&thread_pool[i], NULL, dispatcher_of_thread, NULL);
    }
    int _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket < 0)
    {
        printf("\nsocket could not be created\n");
    }
    threads_counter = MAX_REQUEST_THREADS;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int IN_size = sizeof(struct sockaddr_in);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    int _bind_status = bind(_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
    if (_bind_status < 0)
    {
        printf("\nthe bind could not be done\n");
    }
    int _listen_status = listen(_socket, LISTEN_BACKLOG);
    if (_listen_status < 0)
    {
        printf("\nlisten was not successful\n");
    }
    while (1)
    {

        printf("Waiting for the client to accept\n");
        int _client_socket = accept(_socket, (struct sockaddr *)&client_addr, &IN_size);

        if (threads_counter <= 0)
        {
            printf("\n");
            printf("Thread count at its limit\n");
            continue;
        }
        printf("\n the client accepted your request");
        printf("\n");
        pthread_mutex_lock(&mutex);
        --threads_counter;
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
    char request_array[5001];
    int _client_socket = *((int *)p_client);
    free(p_client);
    char *USE = "?", *SE = "?";
    printf("\n");
    printf("enqueuing client requests begins..\n");
    while (1)
    {

        memset(request, '\0', sizeof(char) * 5001);
        memset(request_array, '\0', sizeof(request_array));
        int _recv_status = recv(_client_socket, request_array, sizeof(request_array), 0);
        if (_recv_status <= 0)
        {
            break;
        }
        int request_arraylen = strlen(request_array);
        for (int i = 0; i < request_arraylen; i++)
        {
            request[i] = request_array[i];
        }
        pthread_mutex_lock(&mutex);
        bool enqueue_status = enqueue(request);
        pthread_mutex_unlock(&mutex);
        //mutex

        printf("sending message to the client.\n");
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
    ++threads_counter;
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

        printf(".\a.out [PORT] [limit of the thread] [openfile_limit] [maximum memory of the program]\n");
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
        printf("\nplease provide as atleast 6 files \n");
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
    dll_handler_function(NULL);

    printf("no error occured\n\n\n");
}
void testing_incorrect_inputpath(char *incorrect)
{

    printf("Test %d", ithtest++);
    printf("\nTesting incorrect path\n");
    dll_handler_function(incorrect);
    printf("no error occured\n\n\n");
}
void testing_incorrect_inputfunctionname(char *incorrect)
{
    printf("Test %d", ithtest++);
    printf("\nTesting incorrect function name\n");
    dll_handler_function(incorrect);
    printf("no error occured\n\n\n");
}
/*
void testing_incorrect_arguments(char * incorrect){
    printf("Test %d", ithtest++);
    printf("\nTesting incorrect arguments name\n");
    dll_handler_function(incorrect);
    printf("\n\n");
}
*/
void testing_correct_input(char *correct)
{
    printf("Test %d", ithtest++);
    printf("\nTesting correct function name\n");
    dll_handler_function(correct);
    printf("no error occured\n\n\n");
}

void queue_limit_checking()
{
    printf("Test %d", ithtest++);
    printf("\nQueue limit checking\n");
    for (int i = 1; i <= 101; i++)
    {
        bool _enqueued = enqueue("hey");
        if (_enqueued == 0)
        {
            if (i != 101)
            {
                printf("error queue size full after %d enqueue\n", i);
                return;
            }
            else
            {
                printf("correct: queue size is full after 100 enqueue\n");
            }
        }
    }
    for (int i = 1; i <= 100; i++)
    {
        dequeue();
    }
    printf("no error occured\n\n\n");
}
void check_dequeue()
{
    printf("Test %d", ithtest++);
    printf("\nDequeueing empty queue\n");
    char *s = dequeue();
    if (s == NULL)
    {
        printf("Empty dequeue successful\n");
    }
    else
    {
        printf("Error occured");
        return;
    }
    for (int i = 0; i < 1000; i++)
    {
        bool en = enqueue("hey");
        char *deq = dequeue();
        if (en == 0 || deq == NULL)
        {
            printf("error\n");
        }
    }
    printf("no error in 1000 enqueue dequeue\n");
    printf("no error occured\n\n\n");
}
void unit_testing(int thread_limit, int open_file_limit, int memory_limit)
{
    passing_empty_in_dll();
    testing_incorrect_inputpath("error/lib/x86_64-linux-gnu/libm.so.6?cos?2");
    testing_incorrect_inputfunctionname("/lib/x86_64-linux-gnu/libm.so.6?errors?2");
    //testing_incorrect_inputfunctionname("/lib/x86_64-linux-gnu/libm.so.6?cos?a?b");
    testing_correct_input("/lib/x86_64-linux-gnu/libm.so.6?tan?2");
    //opening_more_files(open_file_limit);
    queue_limit_checking();
    check_dequeue();
}
