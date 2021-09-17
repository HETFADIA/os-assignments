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
//separator <dll>separator <function name> separator <arguments>
char separator = '?';

int max_threads = 100;
int max_memory_limit;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//making array based queue
char *queue[100];
int start = 0;
int end = 0;
int qsize = 100;
int currsize = 0;
//enqueue in the cirucular based queue
bool enqueue(char *s)
{
    if (currsize >= qsize)
    {
        return 0;
    }
    int slen = strlen(s);
    // queue[end] sotres the s
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
//dequeue the request from the circular array based queue
char *dequeue()
{
    if (currsize == 0)
    {
        //queue size is 0 so it is empty
        return NULL;
    }
    currsize--;
    return queue[(start++) % qsize];
}
//counter of the threads
int threads_counter = 0;

void *request_server(void *p_client);
bool dll_handler_function(char *array_of_character);
int get_memory_usage();
void *dispatcher_of_thread(void *arg);

int get_memory_usage()
{
    //gives the memory usage of the program
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
    // splits the library function name and arguments separated by ?
    char deserialize[501][501];
    int deserializelen = 0;
    int counter = 0;
    memset(deserialize, '\0', sizeof(deserialize));
    int serializelen = strlen(array_of_character);

    for (int i = 0; i < serializelen; i++)
    {
        //splitting it at separator
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
                //too many files are open so we leave
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
        //length is 3 so library  funciton name and funciton arugment (only one argumetn)
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
        double (*function1)(double, double);
        function1 = dlsym(handle, deserialize[1]);
        if (!function1)
        {
            printf("The requested function does not exist\n");
            return 0;
        }
        //calling the function
        result = function1(atof(deserialize[2]), atof(deserialize[3]));
    }
    else if (deserializelen == 5)
    {
        double (*function1)(double, double, double);
        function1 = dlsym(handle, deserialize[1]);
        if (!function1)
        {
            printf("The requested function does not exist\n");
            return 0;
        }

        result = function1(atof(deserialize[2]), atof(deserialize[3]), atof(deserialize[4]));
    }
    else if (deserializelen == 6)
    {
        double (*function1)(double, double, double, double);
        function1 = dlsym(handle, deserialize[1]);
        if (!function1)
        {
            printf("The requested function does not exist\n");
            return 0;
        }
        //calling teh function
        result = function1(atof(deserialize[2]), atof(deserialize[3]), atof(deserialize[4]), atof(deserialize[5]));
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
        bool memory_more_used = memory_curr_used > max_memory_limit;
        //if memory is more we wait for the memory to get freed up
        if (memory_more_used)
        {
            printf("Memory limit exceeded\n");
            continue;
        }
        pthread_mutex_lock(&mutex);
        //as deuqueue contians global variables we have to execute it in mutex
        char *request = dequeue();
        pthread_mutex_unlock(&mutex);
        if (!request)
        {
            continue;
        }
        printf("%s\n", request);
        dll_handler_function(request);
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
    // making the thread pool
    pthread_t thread_pool[thread_maxlimit];
    max_memory_limit = memory_limit;

    for (int i = 0; i < thread_maxlimit; ++i)
    {
        pthread_create(&thread_pool[i], NULL, dispatcher_of_thread, NULL);
    }
    int _socket;
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket < 0)
    {
        printf("\nsocket could not be created\n");
    }
    threads_counter = max_threads;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int IN_size = sizeof(struct sockaddr_in);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    int _bind_status = bind(_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
    //chekcing the bind status
    if (_bind_status < 0)
    {
        printf("\nthe bind could not be done\n");
    }
    int _listen_status = listen(_socket, 100);
    //chekcing the listen status
    if (_listen_status < 0)
    {
        printf("\nlisten was not successful\n");
    }
    while (1)
    {

        printf("Waiting for the client to accept\n");
        int socket_accepted = accept(_socket, (struct sockaddr *)&client_addr, &IN_size);

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
        *p_client = socket_accepted;
        pthread_create(&new_thread, NULL, request_server, p_client);
    }
}
void *request_server(void *p_client)
{
    char *request = (char *)malloc(sizeof(char) * 7000);
    char request_array[7000];
    int socket_accepted = *((int *)p_client);
    free(p_client);
    char *written = "?";
    printf("\n");

    while (1)
    {

        memset(request, '\0', sizeof(char) * 7000);
        memset(request_array, '\0', sizeof(request_array));
        int _recv_status = recv(socket_accepted, request_array, sizeof(request_array), 0);
        if (_recv_status <= 0)
        {
            break;
        }
        int request_arraylen = strlen(request_array);
        for (int i = 0; i < request_arraylen; i++)
        {
            request[i] = request_array[i];
        }
        printf("sending message to the client.\n");
        pthread_mutex_lock(&mutex);
        bool enqueue_status = enqueue(request);
        pthread_mutex_unlock(&mutex);
        //mutex

        write(socket_accepted, written, sizeof(written));
    }
    pthread_mutex_lock(&mutex);
    //as it is golbal pointer we have to execute it in mutex
    ++threads_counter;
    pthread_mutex_unlock(&mutex);

    close(socket_accepted);
    pthread_exit(NULL);
    return NULL;
}
void unit_testing(int thread_limit, int open_file_limit, int memory_limit);
int main(int argc, char **argv)
{
    // to run the program
    /*
    gcc main.c -lpthread -ldl -o main
    ./main.out [PORT] [Thread limit] [limit of the number of files that can be opened] [max_memory]
    */
    // to run test use
    /*
    gcc main.c -lpthread -ldl
    ./a.out test [Thread limit] [limit of the number of files that can be opened] [max_memory]
   */
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
        // checking if it is for testing argv[1]=="test"
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
        //need at least one thread
        printf("\nError no threads available\n");
    }
    else if (c <= 5)
    {
        //chekcing if there are enough files
        //three files are initially open namley stdin,stdout,stderr
        //another file is establised on connection with the clinet
        printf("\nplease provide as atleast 6 files \n");
    }
    else if (d <= 6000)
    {
        //need at least 6 MB memory to funciton smoothly
        printf("\nPlease increase the memory for the program\n");
    }
    else
    {
        //if all conditions are met we start the server
        make_server(a, b, c, d);
    }
}
int ithtest = 1;
void passing_empty_in_dll()
{
    //testing the program when empty string is given
    printf("Test %d", ithtest++);
    printf("\ncalling handler module with empty string\n");
    dll_handler_function(NULL);

    printf("no error occured\n\n\n");
}
void testing_incorrect_inputpath(char *incorrect)
{
    //testing the program on the incorrect input
    printf("Test %d", ithtest++);
    printf("\nTesting incorrect path\n");
    dll_handler_function(incorrect);
    printf("no error occured\n\n\n");
}
void testing_incorrect_inputfunctionname(char *incorrect)
{
    //testing the program on the incorrect input
    printf("Test %d", ithtest++);
    printf("\nTesting incorrect function name\n");
    dll_handler_function(incorrect);
    printf("no error occured\n\n\n");
}

void testing_correct_input(char *correct)
{
    //testing the program on correct input
    printf("Test %d", ithtest++);
    printf("\nTesting correct function name\n");
    dll_handler_function(correct);
    printf("no error occured\n\n\n");
}

void queue_limit_checking()
{
    //checking what happens on 101 enqueue
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
    //checing the dqueue status
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
void checking_file_limit(){
    printf("Test %d", ithtest++);
    struct rlimit lim;
    lim.rlim_cur = 3;
    printf("\nFile limit set to 3\n");
    lim.rlim_max = 1024;
    bool not_possible = (setrlimit(RLIMIT_NOFILE, &lim) == -1);
    struct rlimit new_lim;

    if (not_possible)
    {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(-1);
    }
    FILE * fp= fopen("input3.txt","w");
    if(fp==NULL){
        //as the file limit was 3 we could not open this file
        //so it is successful
        printf("\nsuccessful: could not open the fourth file as the file open limit is 3\n");
    }
    else{
        printf("\nfailure\n");
        return;
    }
    
    lim.rlim_cur = 4;
    printf("\nFile limit set to 4\n");
    FILE * fp2= fopen("input2.txt","w");
    if(fp2==NULL){
        printf("\nsuccessful: could not open the 5th file as the file open limit is 4\n");
    }
    else{
        printf("Error occured");
    }
    

    printf("no error occured\n\n\n");
}

void unit_testing(int thread_limit, int open_file_limit, int memory_limit)
{
    passing_empty_in_dll();
    testing_incorrect_inputpath("error/lib/x86_64-linux-gnu/libm.so.6?cos?2");
    testing_incorrect_inputfunctionname("/lib/x86_64-linux-gnu/libm.so.6?errors?2");

    testing_correct_input("/lib/x86_64-linux-gnu/libm.so.6?tan?2");
    queue_limit_checking();
    check_dequeue();
    checking_file_limit();
}


// to run the program
    /*
    gcc main.c -lpthread -ldl -o main
    ./main.out [PORT] [Thread limit] [limit of the number of files that can be opened] [max_memory]
    */


    // to run test use


    /*
    gcc main.c -lpthread -ldl
    ./a.out test [Thread limit] [limit of the number of files that can be opened] [max_memory]
   */
