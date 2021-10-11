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
#include <math.h>
pthread_mutex_t mutex_arr_of_resource = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_deadlock_detection = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_keep_alive = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_time = PTHREAD_MUTEX_INITIALIZER;
int MAX_TOTAL_RESOURCES;
int TOTAL_THREADS;
int TIME_DELAY;
int time_stamp_counter = 0;
int function_no=1;
bool deadlock_resolved=true;
int *arr_of_resources, *maxarr_of_resources, *time_stamp;
bool *keep_alive;
int **requests, **max_requests;
int sum(int arr[], int len)
{
    int res = 0;
    int i=0;
    while(i<len){
        res+=arr[i++];
    }
    return res;
}
int min(int x,int y){
    return x<y?x:y;
}
int max(int x,int y){
    return x>y?x:y;
}
int randint(int x,int y){
    return x+ rand()%(y-x+1);
}
int randrange(int x,int y){
    return x+ rand()%(y-x);
}
void sleep_for_decided(int d){
    int select=(700000+rand()%800000);//selects random time (0.7d,1.5d)
    usleep(select*d);
}

int heuristics1(bool arr_involved_in_deadlock[])
{
    //selects the resource having max sum of resources needed(max(sum(needed resources)))
    int to_be_removed = 0;
    int to_be_removed_sum = -1;
    for (int i = 0; i < TOTAL_THREADS; i++)
    {
        pthread_mutex_lock(&mutex_deadlock_detection);
        int temp_sum = sum(requests[i], MAX_TOTAL_RESOURCES);
        pthread_mutex_unlock(&mutex_deadlock_detection);
        if (arr_involved_in_deadlock[i] && temp_sum > to_be_removed_sum)
        {
            to_be_removed = i;
            to_be_removed_sum = temp_sum;
        }
    }
    return to_be_removed;
}
int heuristics2(bool arr_involved_in_deadlock[])
{
    //selects the resource having max sum of resources max_needed(max(sum(max_needed)))
    int to_be_removed = 0;
    int to_be_removed_sum = -1;
    for (int i = 0; i < TOTAL_THREADS; i++)
    {
        pthread_mutex_lock(&mutex_deadlock_detection);
        int temp_sum = sum(max_requests[i], MAX_TOTAL_RESOURCES);
        pthread_mutex_unlock(&mutex_deadlock_detection);
        
        if (arr_involved_in_deadlock[i] && temp_sum > to_be_removed_sum)
        {
            to_be_removed = i;
            to_be_removed_sum = temp_sum;
        }
    }
    return to_be_removed;
}
int heuristics3(bool arr_involved_in_deadlock[])
{
    //selects youngest thread i.e. having max time_stamp
    int to_be_removed = 0;
    int to_be_removed_time = -1;
    for (int i = 0; i < TOTAL_THREADS; i++)
    {

        if (arr_involved_in_deadlock[i] && time_stamp[i] > to_be_removed_time)
        {
            to_be_removed = i;
            to_be_removed_time = time_stamp[i];
        }
    }
    return to_be_removed;
}

int heuristics4(bool arr_involved_in_deadlock[])
{
    //selects max resource
    int to_be_removed = 0;
    int to_be_removed_value = -1;
    for (int i = 0; i < TOTAL_THREADS; i++)
    {
        if (arr_involved_in_deadlock[i])
        {
            for (int j = 0; j < MAX_TOTAL_RESOURCES; j++)
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
void *thread_process(void *args)
{
    int thread_num = *((int *)args);
    int arr[MAX_TOTAL_RESOURCES], save_temp[MAX_TOTAL_RESOURCES];
    while (true)
    {
        if(!deadlock_resolved){
            continue;
        }
        pthread_mutex_lock(&mutex_time);
        time_stamp[thread_num] = ++time_stamp_counter;
        pthread_mutex_unlock(&mutex_time);
        
        int TRACK = MAX_TOTAL_RESOURCES;
        for (int i = 0; i < MAX_TOTAL_RESOURCES; i++)
        {
            arr[i]=randrange(0,maxarr_of_resources[i] + 1);
            if (!arr[i]){
                --TRACK;
            }
            save_temp[i] = arr[i];
            pthread_mutex_lock(&mutex_deadlock_detection);
            requests[thread_num][i] = arr[i];
            max_requests[thread_num][i] = save_temp[i];
            pthread_mutex_unlock(&mutex_deadlock_detection);
        }
        pthread_mutex_lock(&mutex_keep_alive);
        keep_alive[thread_num] = 1;
        pthread_mutex_unlock(&mutex_keep_alive);
        
        // TRACK the requirement of resources, if none end looping
        while (TRACK > 0)
        {
            pthread_mutex_lock(&mutex_keep_alive);
            int break_from_loop=keep_alive[thread_num]==false;
            pthread_mutex_unlock(&mutex_keep_alive);
            if(break_from_loop){
                break;
            }
            
            int resource_for_now = randint(0,MAX_TOTAL_RESOURCES);
            

            if (!arr[resource_for_now])
                continue;

            //mutex lock part 1
            pthread_mutex_lock(&mutex_arr_of_resource);
            int taken = min(arr_of_resources[resource_for_now], arr[resource_for_now]);
            arr_of_resources[resource_for_now] -= taken;
            pthread_mutex_unlock(&mutex_arr_of_resource);
            //mutex unlock

            arr[resource_for_now] -= taken;
            pthread_mutex_lock(&mutex_deadlock_detection);
            requests[thread_num][resource_for_now] -= taken;
            pthread_mutex_unlock(&mutex_deadlock_detection);

            if (!arr[resource_for_now])
            {
                TRACK -= 1;
            }
            sleep(randint(1,TIME_DELAY));
        }
        
        pthread_mutex_lock(&mutex_keep_alive);
        int should_sleep_be_given=keep_alive[thread_num]==1;
        pthread_mutex_unlock(&mutex_keep_alive);
        if(should_sleep_be_given){
            sleep_for_decided(TIME_DELAY);
        }

        for (int i = 0; i < MAX_TOTAL_RESOURCES; i++)
        {
            //mutex lock 2
            pthread_mutex_lock(&mutex_arr_of_resource);
            arr_of_resources[i] += (save_temp[i] - arr[i]);
            pthread_mutex_unlock(&mutex_arr_of_resource);
            //mutex unlock
        }



        for (int i = 0; i < MAX_TOTAL_RESOURCES; i++)
        {
            pthread_mutex_lock(&mutex_deadlock_detection);
            requests[thread_num][i] = 0;
            pthread_mutex_unlock(&mutex_deadlock_detection);
        }
        pthread_mutex_lock(&mutex_keep_alive);
        keep_alive[thread_num] = 1;
        pthread_mutex_unlock(&mutex_keep_alive);
    }
    return NULL;
}
int times_deadlock_found=0;
int times_deadlock_checked=0;
void deadlock_detection()
{
    while (true)
    {
        ++times_deadlock_checked;
        printf("Detecting deadlock ....\n");
        
        bool deadlock_found = true;
        bool arr_involved_in_deadlock[TOTAL_THREADS];
        for(int i=0;i<TOTAL_THREADS;++i){
            arr_involved_in_deadlock[i] = false;
        }
        for (int i = 0; i < TOTAL_THREADS; i++)
        {
            bool ith_thread_can_go = true;
            
            for (int j = 0; j < MAX_TOTAL_RESOURCES; j++)
            {
                pthread_mutex_lock(&mutex_deadlock_detection);
                bool can_go_inside=arr_of_resources[j] < requests[i][j];
                pthread_mutex_unlock(&mutex_deadlock_detection);
                if (can_go_inside)
                {
                    ith_thread_can_go = false;
                    break;
                }
            }
            if (ith_thread_can_go)
            {
                deadlock_found = false;
            }
            else
            {
                arr_involved_in_deadlock[i] = true;
            }
        }
        if(deadlock_found==false){
            printf("DEADLOCK NOT FOUND\n");
        }
        else{
            printf("Deadlock found\n");
        }
        
        int to_be_removed = -1;
        deadlock_resolved=!deadlock_found;
        if (deadlock_found)
        {
            ++times_deadlock_found;
            if (function_no == 1)
            {
                to_be_removed = heuristics1(arr_involved_in_deadlock);
            }
            else if (function_no == 2)
            {
                to_be_removed = heuristics2(arr_involved_in_deadlock);
            }
            else if (function_no == 3)
            {
                to_be_removed = heuristics3(arr_involved_in_deadlock);
            }
            else if (function_no == 4)
            {
                to_be_removed = heuristics4(arr_involved_in_deadlock);
            }
            pthread_mutex_lock(&mutex_keep_alive);
            keep_alive[to_be_removed] = false;
            pthread_mutex_unlock(&mutex_keep_alive);
            
            
            printf("WE SKIPPED AND QUEUED THE PROCESS AT %dth THREAD\n", to_be_removed);
            
        }

        sleep(TIME_DELAY);
    }
}

signed main(int argc, char **argv)
{
    if (argc < 6)
    {
/*
gcc main.c -lpthread
./a.out 2 20 1 1 13 17
*/
/*
gcc main.c -lpthread
./a.out 2 2 1 1 13 17
*/
        printf("format of input = a.out || [Maximum number of instances available] || [Maximum number of threads to use in the simulation.] || [Deadlock detection check interval d in seconds.] || [function name for heurestic]|| name of the instances \n");
        exit(-1);
    }
    MAX_TOTAL_RESOURCES = atoi(argv[1]);
    if(MAX_TOTAL_RESOURCES==0){
        printf("Please provide at least 1 max instance\n");
        exit(-1);
    }
    if(MAX_TOTAL_RESOURCES+5!=argc){
        printf("incorrect input format\n");
        printf("The no of max instances is incorrect\n");
        exit(-1);
    }
    TOTAL_THREADS = atoi(argv[2]);
    if(TOTAL_THREADS==0){
        printf("Please provide at least 1 thread\n");
        exit(-1);
    }
    TIME_DELAY = atoi(argv[3]);
    function_no = atoi(argv[ 4]);
    arr_of_resources = (int *)malloc(sizeof(int) * MAX_TOTAL_RESOURCES);
    maxarr_of_resources=(int *)malloc(sizeof(int) * MAX_TOTAL_RESOURCES);
    for (int i = 0; i < MAX_TOTAL_RESOURCES; i++)
    {
        arr_of_resources[i] = atoi(argv[i + 4]);
        maxarr_of_resources[i]=arr_of_resources[i];
    }
    pthread_t arr_thread[TOTAL_THREADS];
    requests = (int **)malloc(sizeof(int *) * TOTAL_THREADS);
    max_requests = (int **)malloc(sizeof(int *) * TOTAL_THREADS);
    keep_alive = (bool *)malloc(sizeof(bool) * TOTAL_THREADS);
    time_stamp = (int *)malloc(sizeof(int) * TOTAL_THREADS);
    for (int i = 0; i < TOTAL_THREADS; i++)
    {
        requests[i] = (int *)malloc(sizeof(int) * MAX_TOTAL_RESOURCES);
        max_requests[i] = (int *)malloc(sizeof(int) * MAX_TOTAL_RESOURCES);
        keep_alive[i] = 1;
        time_stamp[i] = 0;
    }
    
    for (int i = 0; i < TOTAL_THREADS; i++)
    {
        int *thread_num = (int *)malloc(sizeof(int));
        *thread_num = i;
        pthread_create(&arr_thread[i], NULL, thread_process, thread_num);
    }
    printf("Threads=%d\n", TOTAL_THREADS);
    printf("Total resources=%d\n",MAX_TOTAL_RESOURCES);
    printf("Time delay= %ds\n",TIME_DELAY);
    printf("Function no= %d\n",function_no);
    deadlock_detection();
    
}