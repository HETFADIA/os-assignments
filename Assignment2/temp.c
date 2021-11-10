#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <dlfcn.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/resource.h>
pthread_mutex_t mutex_memory_array = PTHREAD_MUTEX_INITIALIZER;
typedef struct node
{
    int duration, memory;
    clock_t time;
    struct node *next;
}(node);
typedef struct pair
{
    int duration, memory;
    clock_t time;
}(pair);
typedef struct triplet
{
    int duration;
    int memory;
    int index;
    clock_t start_time;
}(triplet);
node *new_node(int duration, int memory, clock_t time)
{
    node *newnode = (node *)malloc(sizeof(node));
    newnode->duration = duration;
    newnode->memory = memory;
    newnode->time = time;
    newnode->next = NULL;
    return newnode;
}
pair *new_pair(int duration, int memory, clock_t time)
{
    pair *newnode = (pair *)malloc(sizeof(pair));
    newnode->duration = duration;
    newnode->memory = memory;
    newnode->time = time;
    return newnode;
}
node *head = NULL;
node *tail = NULL;
int queue_size = 0;
void append(int duration, int memory)
{
    if (queue_size > 10)
    {
        return;
    }
    node *newnode = new_node(duration, memory, clock());
    if (!tail)
    {
        head = tail = newnode;
    }
    else
    {
        tail->next = newnode;
        tail = newnode;
    }
    queue_size++;
}
pair *dequeue()
{
    if (!head)
    {
        /* printf("head empty"); */
        return new_pair(-1, -1, clock());
    }
    else
    {
        pair *ret = new_pair(-1, -1, clock());
        ret->duration = head->duration;
        ret->memory = head->memory;
        ret->time = head->time;
        node *n = head;
        head = head->next;
        if (head == NULL)
        {
            tail = NULL;
        }
        free(n);
        queue_size--;
        return ret;
    }
}

bool *memory_array; // 0 represents empty and 1 is filled
int p=0, q=0, n=0, m=0, t=0;
int total_size = 0;
int total_run_time = 0;
int type = 1;
double memory_arrayival_rate = -1;
int memory_utilization = 0;
int processes_executed = 0;
double total_time = 0;
// ind=0 1 2 3 4 5 6 7
// arr=1 1 0 0 1 0 0 0 
// size=30MB
int first_fit(int size)
{
    int last_start = 1e9;
    for (int i = 0; i < total_size; i++)
    {
        if (i - last_start >= size)
        {
            return last_start;
        }
        if (memory_array[i] == false && last_start == 1e9)
        {
            last_start = i;
        }
        if (memory_array[i] == true)
        {
            last_start = 1e9;
        }
    }
    return -1;
}
// 32 16 8 4 4
// 0 1 2 3 4 5 6 7 8 9 10 ...  1000
int best_fit(int size)
{
    int last_start = 1e9;
    int best_chunk_size = 1e9;
    int best_index = 1e9;
    for (int i = 0; i < total_size; i++)
    {
        if (memory_array[i] == 0 && last_start == 1e9)
        {
            last_start = i;
        }

        if (memory_array[i] == 1 && i - last_start >= size && i - last_start <= best_chunk_size)
        {
            best_index = last_start;
            best_chunk_size = i - last_start;
        }

        if (memory_array[i] == 1)
        {
            last_start = 1e9;
        }
        if (i == total_size - 1 && i - last_start >= size && i - last_start <= best_chunk_size)
        {
            best_index = last_start;
            best_chunk_size = i - last_start;
        }
    }
    if (best_index == 1e9)
    {
        return -1;
    }
    return best_index;
}
/* 1 2 3 4 5 6 7 8 9 10 */
int last_index = 0;
int next_fit(int size)
{

    int last_start = 1e9;

    for (int i = last_index + 1; i < total_size; i++)
    {
        if (i - last_start >= size)
        {
            last_index = last_start;
            return last_start;
        }
        if (memory_array[i] == 0 && last_start == 1e9)
        {
            last_start = i;
        }
        if (memory_array[i] == 1)
        {
            last_start = 1e9;
        }
    }
    last_start = 1e9;
    for (int i = 0; i < total_size; i++)
    {
        if (i - last_start >= size)
        {
            last_index = last_start;
            return last_start;
        }
        if (memory_array[i] == 0 && last_start == 1e9)
        {
            last_start = i;
        }
        if (memory_array[i] == 1)
        {
            last_start = 1e9;
        }
    }
    return -1;
}
int randint(int x, int y)
{
    // generates a random number between [x,y] both inclusive
    return x + rand() % (y - x + 1);
}
int randrange(int x, int y)
{
    // generates a random number [x,y) x inclusive and y exlculsive
    return x + rand() % (y - x);
}
int random_size()
{
    //calculating lower val
    int lower = 0.5 * m;
    //calculating higher val
    int high = 3.0 * m;
    while (lower % 10 != 0)
    {
        ++lower;
    }
    while (high % 10 != 0)
    {
        --high;
    }
    lower /= 10;
    high /= 10;
    int rand_no = randint(lower, high);
    return rand_no;
}
int random_duration()
{
    int lower = 0.5 * t;
    int high = 6.0 * t;
    while (lower % 5 != 0)
    {
        ++lower;
    }
    while (high % 5 != 0)
    {
        --high;
    }
    int possibilities = (high - lower) / 5 + 1;
    int rand_no = randrange(0, possibilities);

    return lower + rand_no * 5;
}
double random_memory_arrayival_rate()
{
    int lower = 1 * t;
    int high = 12 * t;
    double rand_no = randint(lower, high);
    rand_no /= 10;
    return rand_no;
}
void *simulate()
{
    clock_t _start = clock();
    while ((double)(clock() - _start) / CLOCKS_PER_SEC < total_run_time * 60)
    {

        int size_p = random_size();
        int dur_p = random_duration();
        /* printf("Size is %d and duration is %d enqueued \n", size_p, dur_p); */
        append(dur_p, size_p);
        memory_arrayival_rate = random_memory_arrayival_rate();
        usleep(1 / memory_arrayival_rate * (1e6));
    }
    return NULL;
}
void *run_process(void *v)
{
    triplet *val = (triplet *)v;
    int dur = val->duration;
    int size = val->memory;
    int index = val->index;
    sleep(dur);
    pthread_mutex_lock(&mutex_memory_array);
    for (int i = index; i < index + size; i++)
    {
        memory_array[i] = false;
        --memory_utilization;
    }
    pthread_mutex_unlock(&mutex_memory_array);

    free(val);
    return NULL;
}
int findindex(int type, pair *de)
{
    if (type == 1)
    {
        return first_fit(de->memory);
    }
    else if (type == 2)
    {
        return best_fit(de->memory);
    }
    else if (type == 3)
    {
        return next_fit(de->memory);
    }
    return -1;
}
void *dispatcher_function()
{
    pair *de;
    clock_t _start = clock();
    while ((double)(clock() - _start) / CLOCKS_PER_SEC < total_run_time * 60)
    {
        de = dequeue();
        if (de->memory == -1)
        {
            continue;
        }
        int index = -1;
        while (index == -1)
        {
            pthread_mutex_lock(&mutex_memory_array);
            index = findindex(type, de);
            if (index != -1)
            {
                for (int i = index; i < index + de->memory; i++)
                {
                    memory_array[i] = true;
                    ++memory_utilization;
                }
            }
            pthread_mutex_unlock(&mutex_memory_array);
        }
        // double time = de[2] + de[0];
        pthread_t thr;
        triplet *newnode = (triplet *)malloc(sizeof(triplet));
        newnode->duration = de->duration;
        newnode->memory = de->memory;
        newnode->index = index;
        newnode->start_time = de->time;
        printf("Duration of program  is %d, Memory is %d\nIndex at which it will take memory is %d\n", de->duration, 10*de->memory, index);
        pthread_create(&thr, NULL, run_process, newnode);
        processes_executed++;
        double time_start_end = (double)clock() / CLOCKS_PER_SEC - (double)de->time / CLOCKS_PER_SEC;

        total_time += time_start_end;
        pthread_mutex_lock(&mutex_memory_array);
        printf("The memory memory_arrayay is :\n");
        for (int i = 0; i < total_size; i++)
        {
            printf("%d ", memory_array[i]);
        }
        printf("\n");

        printf("Total time is %lf and processes executed are %d\n", total_time, processes_executed);
        printf("Turnaround time: %lf\n", (double)total_time / (double)processes_executed);
        printf("Memory utilization: %f%%\n", (double)((memory_utilization)*10 + q) / (double)(p)*100);
        pthread_mutex_unlock(&mutex_memory_array);
    }
    return NULL;
}
int main(int argc, char **argv)
{
    if (argc < 8)
    {
        /*
        gcc main.c -lpthread
        ./a.out 1000 200 10 10 10 10 1
        */
        printf("Too few arguments\n");
        printf("The arguments should be p q n m t total_run_time and type of the function\n");
        printf("One such example is ./a.out 1000 200 10 10 10 1 1\n");
        exit(-1);
    }
    p = atoi(argv[1]);
    printf("Total memory is %d\n",p);
    q = atoi(argv[2]);
    printf("Memory used by system is %d\n",q);
    if(p-q<=0 || p<0 || q<0){
        printf("Memory should be positive\n");
        exit(-1);
    }
    printf("Memory left = %d",p-q);
    n = atoi(argv[3]);

    memory_arrayival_rate = random_memory_arrayival_rate();
    printf("memory_arrayival rate is %lf\n", memory_arrayival_rate);
    total_size = (p - q) / 10;
    m = atoi(argv[4]);
    t = atoi(argv[5]);
    printf("t is %d\n",t);
    total_run_time = atoi(argv[6]);
    if(total_run_time<=0){
        printf("The total run time should be positive \n");
        exit(-1);
    }
    printf("The process will run for %d minutes\n",total_run_time);
    type = atoi(argv[7]);
    printf("The type of the function is %d\n",type);

    memory_array = (bool *)malloc(sizeof(bool) * total_size);
    for (int i = 0; i < total_size; i++)
    {
        memory_array[i] = false;
    }
    pthread_t t_dash;
    pthread_create(&t_dash, NULL, simulate, NULL);

    pthread_t dispatcher_function_thread;
    pthread_create(&dispatcher_function_thread, NULL, dispatcher_function, NULL);


    pthread_join(t_dash, NULL);
    pthread_join(dispatcher_function_thread, NULL);
}