#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


pthread_mutex_t mutex_arr_of_resource = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_deadlock_detection = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cycle = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_check_deadlock = PTHREAD_MUTEX_INITIALIZER;

int NUM_RESOURCES, TIME_DELAY, NUM_THREADS;
int *curr_resources;
int *max_resources;
int *time_stamp;
int curr_time = 1;
bool *run_thread;
int **requests;
int **max_requests;
int heuristic_type=1;
bool deadlock_resolved=1;
bool checking_deadlock = 0;
int semaphore = 0;

int sum(int *arr, int len)
{
    int res = 0;
    for (int i = 0; i < len; i++)
    {
        res += arr[i];
    }
    return res;
}
int heuristics1(bool *thread_in_deadlock)
{
    //selects the resource having max sum of resources needed(max(sum(needed resources)))
    int to_be_removed = 0;
    int to_be_removed_sum = -1;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_mutex_lock(&mutex_deadlock_detection);
        int temp_sum = sum(requests[i], NUM_RESOURCES);
        pthread_mutex_unlock(&mutex_deadlock_detection);
        if (thread_in_deadlock[i] && temp_sum > to_be_removed_sum)
        {
            to_be_removed = i;
            to_be_removed_sum = temp_sum;
        }
    }
    return to_be_removed;
}
int heuristics2(bool *thread_in_deadlock)
{
    //selects the resource having max sum of resources max_needed(max(sum(max_needed)))
    int to_be_removed = 0;
    int to_be_removed_sum = -1;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_mutex_lock(&mutex_deadlock_detection);
        int temp_sum = sum(max_requests[i], NUM_RESOURCES);
        pthread_mutex_unlock(&mutex_deadlock_detection);
        
        if (thread_in_deadlock[i] && temp_sum > to_be_removed_sum)
        {
            to_be_removed = i;
            to_be_removed_sum = temp_sum;
        }
    }
    return to_be_removed;
}
int heuristics3(bool *thread_in_deadlock)
{
    //selects youngest thread i.e. having max time_stamp
    int to_be_removed = 0;
    int to_be_removed_time = -1;
    for (int i = 0; i < NUM_THREADS; i++)
    {

        if (thread_in_deadlock[i] && time_stamp[i] > to_be_removed_time)
        {
            to_be_removed = i;
            to_be_removed_time = time_stamp[i];
        }
    }
    return to_be_removed;
}

int heuristics4(bool *thread_in_deadlock)
{
    //selects max resource
    int to_be_removed = 0;
    int to_be_removed_value = -1;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        if (thread_in_deadlock[i])
        {
            for (int j = 0; j < NUM_RESOURCES; j++)
            {
                pthread_mutex_lock(&mutex_deadlock_detection);
                int temp=requests[i][j];
                pthread_mutex_unlock(&mutex_deadlock_detection);
                if (temp > to_be_removed_value)
                {
                    to_be_removed = i;
                    to_be_removed_value = temp;
                }
            }
        }
    }
    return to_be_removed;
}

int thread_to_be_eliminated(bool *thread_in_deadlock){
    if(heuristic_type == 1)
        return heuristics1(thread_in_deadlock);

    else if(heuristic_type == 2)
        return heuristics2(thread_in_deadlock);

    else if(heuristic_type == 3)
        return heuristics3(thread_in_deadlock);
    
    else
        return heuristics4(thread_in_deadlock);
}

void *Execute_Thread(void *args){

    int thread_id = *((int *)args);
    while (1)
    {
        run_thread[thread_id] = 1;
        while(!deadlock_resolved){}

        time_stamp[thread_id] = ++curr_time;
        run_thread[thread_id] = 1;
        
        int TRACK = 0;
        for (int i = 0; i < NUM_RESOURCES; i++){
            requests[thread_id][i] = rand()%(max_resources[i]) + rand()%2;
            if (requests[thread_id][i])
                TRACK++;
            pthread_mutex_lock(&mutex_deadlock_detection);
            max_requests[thread_id][i] = requests[thread_id][i];
            pthread_mutex_unlock(&mutex_deadlock_detection);
        }

        while (TRACK != 0 && run_thread[thread_id]){

            pthread_mutex_lock(&mutex_cycle);
            semaphore--;
            pthread_mutex_unlock(&mutex_cycle);
            int i = rand() % NUM_RESOURCES;

            if (!requests[thread_id][i]){
                pthread_mutex_lock(&mutex_cycle);
                semaphore++;
                pthread_mutex_unlock(&mutex_cycle);
                continue;
            }
            pthread_mutex_lock(&mutex_arr_of_resource);
            pthread_mutex_lock(&mutex_deadlock_detection);
            int borrow_resource;
            borrow_resource = (curr_resources[i] < requests[thread_id][i])?curr_resources[i]:requests[thread_id][i];
            curr_resources[i] -= borrow_resource;
            requests[thread_id][i] -= borrow_resource;
            pthread_mutex_unlock(&mutex_arr_of_resource);
            pthread_mutex_unlock(&mutex_deadlock_detection);

            if (!requests[thread_id][i])
                TRACK--;
            
            pthread_mutex_lock(&mutex_cycle);
            semaphore++;
            pthread_mutex_unlock(&mutex_cycle);

            usleep(rand() % (int)(TIME_DELAY*1e5));
            pthread_mutex_lock(&mutex_check_deadlock);
            pthread_mutex_unlock(&mutex_check_deadlock);
        }

        if(run_thread[thread_id]==1 && TRACK==0)
            usleep(7*(TIME_DELAY*1e5) + rand()%(int)((TIME_DELAY*1e5)*8));

        pthread_mutex_lock(&mutex_arr_of_resource);
        for (int i = 0; i < NUM_RESOURCES; i++)
            curr_resources[i] += (max_requests[thread_id][i] - requests[thread_id][i]);
        pthread_mutex_unlock(&mutex_arr_of_resource);

        pthread_mutex_lock(&mutex_deadlock_detection);
        for (int i = 0; i < NUM_RESOURCES; i++)
            requests[thread_id][i] = 0;
        pthread_mutex_unlock(&mutex_deadlock_detection);       
    }
    return NULL;
}
int deadlock_counter = 0;
int deadlock_check_counter = 0;
void deadlock_detection(){
    deadlock_check_counter++;
    printf("Checking for deadlock\n");
    checking_deadlock = 1;
    bool deadlock_found = 1;
    bool thread_in_deadlock[NUM_THREADS];
    for(int i=0;i<NUM_THREADS;i++)
        thread_in_deadlock[i]=0;
    while(semaphore!=NUM_THREADS){}
    for (int i = 0; i < NUM_THREADS; i++){
        bool can_thread_execute = 1;
        for (int j = 0; j < NUM_RESOURCES; j++){
            pthread_mutex_lock(&mutex_deadlock_detection);
            bool is_thread_deadlocked = curr_resources[j] < requests[i][j]?1:0;
            pthread_mutex_unlock(&mutex_deadlock_detection);
            if(is_thread_deadlocked){
                can_thread_execute = 0;
                break;
            }
        }
        if (can_thread_execute){
            deadlock_found = 0;
            thread_in_deadlock[i] = 0;
        }
        else
            thread_in_deadlock[i] = 1;
    }
    printf("++++++++++++Deadlock status: %d\n", deadlock_found);
    deadlock_resolved=!deadlock_found;
    if (deadlock_found){
        ++deadlock_counter;
        int thread_eliminate = thread_to_be_eliminated(thread_in_deadlock);
        printf("----------------Thread being eliminated is %d\n", thread_eliminate);
        run_thread[thread_eliminate] = 0;
    }
}

int main(int argc, char **argv)
{
    if (argc < 5){
        printf("Format: ./a.out <Resource 1> <Resource 2> .... <Resource n> <Number of Threads> <Heuristic type> <Time delay(s)>\n");
        exit(-1);
    }

    NUM_RESOURCES = argc-4;
    curr_resources = (int*)malloc(sizeof(int) * NUM_RESOURCES);
    max_resources=(int*)malloc(sizeof(int) * NUM_RESOURCES);
    for (int i = 1; i <= NUM_RESOURCES; i++){
        max_resources[i] = atoi(argv[i]);
        curr_resources[i] = max_resources[i];
    }

    NUM_THREADS = atoi(argv[argc-3]);
    semaphore = NUM_THREADS;
    requests = (int**)malloc(sizeof(int *) * NUM_THREADS);
    max_requests = (int**)malloc(sizeof(int *) * NUM_THREADS);
    time_stamp = (int*)malloc(sizeof(int) * NUM_THREADS);
    run_thread = (bool*)malloc(sizeof(bool) * NUM_THREADS);

    for (int i = 0; i < NUM_THREADS; i++){
        requests[i] = (int*)malloc(sizeof(int) * NUM_RESOURCES);
        max_requests[i] = (int*)malloc(sizeof(int) * NUM_RESOURCES);
    }

    heuristic_type = atoi(argv[argc-2]);
    TIME_DELAY = atoi(argv[argc-1]);
    pthread_t thread_array[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++)
    {
        time_stamp[i] = 1;
        run_thread[i] = 1;
        int *thread_id = (int*)malloc(sizeof(int));
        *thread_id = i;
        pthread_create(&thread_array[i], NULL, Execute_Thread, thread_id);
    }
    printf("%d %d %d %d %d\n", NUM_THREADS, NUM_RESOURCES, TIME_DELAY, heuristic_type,heuristic_type);
    while(1){
        checking_deadlock = 0;
        usleep(TIME_DELAY*1e6);
        deadlock_detection();
    }
}
