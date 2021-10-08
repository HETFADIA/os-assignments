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
pthread_mutex_t mutex_arr_of_resource = PTHREAD_MUTEX_INITIALIZER;
int TOTAL_RESOURCES, TOTAL_THREADS, TIME_DELAY;
int *arr_of_resources;
int *time_stamp;
int time_stamp_counter = 0;
bool *keep_alive;
int **requests;
int **max_requests;
int function_no ;
int min(int x, int y)
{
    if (x < y)
        return x;
    return y;
}

void *P_x(void *args)
{
    int thread_num = *((int *)args);
    int arr[TOTAL_RESOURCES], save_temp[TOTAL_RESOURCES];
    while (1)
    {
        time_stamp[thread_num] = ++time_stamp_counter;
        int TRACK = TOTAL_RESOURCES;
        for (int i = 0; i < TOTAL_RESOURCES; i++)
        {
            arr[i] = rand() % (arr_of_resources[i] + 1);
            if (!arr[i])
                TRACK -= 1;
            save_temp[i] = arr[i];
            requests[thread_num][i] = arr[i];
            max_requests[thread_num][i] = save_temp[i];
        }
        // TRACK the requirement of resources, if none end looping
        while (TRACK > 0 && keep_alive[thread_num])
        {
            int resource_for_now = rand() % TOTAL_RESOURCES;

            if (!arr[resource_for_now])
                continue;

            //mutex lock part 1
            pthread_mutex_lock(&mutex_arr_of_resource);
            int taken = min(arr_of_resources[resource_for_now], arr[resource_for_now]);
            arr_of_resources[resource_for_now] -= taken;
            pthread_mutex_unlock(&mutex_arr_of_resource);
            //mutex unlock

            arr[resource_for_now] -= taken;
            requests[thread_num][resource_for_now] -= taken;

            if (!arr[resource_for_now])
            {
                TRACK -= 1;
            }
            sleep(rand() % TIME_DELAY + 1);
        }
        keep_alive[thread_num]=1;
        // sleep for (0.7d, 1.5d);
        usleep((7 * TIME_DELAY + rand() % TIME_DELAY * 8) * 1e5);
        // run thread again like normal people

        //return the resources back
        for (int i = 0; i < TOTAL_RESOURCES; i++)
        {
            //mutex lock 2
            pthread_mutex_lock(&mutex_arr_of_resource);
            arr_of_resources[i] += (save_temp[i]-arr[i]);
            pthread_mutex_unlock(&mutex_arr_of_resource);
            //mutex unlock
        }
    }
    return NULL;
}
int sum(int *arr, int len)
{
    int res = 0;
    for (int i = 0; i < len; i++)
    {
        res += arr[i];
    }
    return res;
}
int heuristics1(bool *arr_involved_in_deadlock)
{
    //selects the resource having max sum of resources needed(max(sum(needed resources)))
    int to_be_removed = 0;
    int to_be_removed_sum = -1;
    for (int i = 0; i < TOTAL_THREADS; i++)
    {
        int temp_sum = sum(requests[i], TOTAL_RESOURCES);
        if (arr_involved_in_deadlock[i] && temp_sum > to_be_removed_sum)
        {
            to_be_removed = i;
            to_be_removed_sum = temp_sum;
        }
    }
    return to_be_removed;
}
int heuristics2(bool *arr_involved_in_deadlock)
{
    //selects youngest thread i.e. having max time_stamp
    int to_be_removed = 0;
    int to_be_removed_time = -1;
    for (int i = 0; i < TOTAL_THREADS; i++)
    {

        if (arr_involved_in_deadlock[i] && time_stamp[i] > to_be_removed_time)
        {
            to_be_removed = i;
            to_be_removed_time = to_be_removed_time;
        }
    }
    return to_be_removed;
}
int heuristics3(bool *arr_involved_in_deadlock)
{
    //selects the resource having max sum of resources max_needed(max(sum(max_needed)))
    int to_be_removed = 0;
    int to_be_removed_sum = -1;
    for (int i = 0; i < TOTAL_THREADS; i++)
    {
        int temp_sum = sum(max_requests[i], TOTAL_RESOURCES);
        if (arr_involved_in_deadlock[i] && temp_sum > to_be_removed_sum)
        {
            to_be_removed = i;
            to_be_removed_sum = temp_sum;
        }
    }
    return to_be_removed;
}
int heuristics4(bool *arr_involved_in_deadlock)
{
    //selects max resource
    int to_be_removed = 0;
    int to_be_removed_value = -1;
    for (int i = 0; i < TOTAL_THREADS; i++)
    {
        if (arr_involved_in_deadlock[i])
        {
            for (int j = 0; j < TOTAL_RESOURCES; j++)
            {
                if (requests[i][j] > to_be_removed_value)
                {
                    to_be_removed = i;
                    to_be_removed_value = requests[i][j];
                }
            }
        }
    }
    return to_be_removed;
}

void *deadlock_detection(void *args)
{
    printf("detecting deadlock\n");
    //run this thread infinitely

    //do something, I forgor 💀
    bool deadlock_found = 1;
    bool arr_involved_in_deadlock[TOTAL_THREADS];

    for (int i = 0; i < TOTAL_THREADS; i++)
    {
        bool ith_thread_can_go = 1;
        arr_involved_in_deadlock[i] = 0;
        for (int j = 0; j < TOTAL_RESOURCES; j++)
        {
            if (arr_of_resources[j] < requests[i][j])
            {
                ith_thread_can_go = 0;
                break;
            }
        }
        if (ith_thread_can_go)
        {
            deadlock_found = 0;
        }
        else
        {
            arr_involved_in_deadlock[i] = 1;
        }
    }
    int to_be_removed=-1;
    if (function_no == 0)
    {
        to_be_removed = heuristics1(arr_involved_in_deadlock);
    }
    else if (function_no == 1)
    {
        to_be_removed = heuristics2(arr_involved_in_deadlock);
    }
    else if (function_no == 2)
    {
        to_be_removed = heuristics3(arr_involved_in_deadlock);
    }
    else if (function_no == 3)
    {
        to_be_removed = heuristics4(arr_involved_in_deadlock);
    }
    printf("to be removed is %d and deadlock is %d",to_be_removed,deadlock_found);
    keep_alive[to_be_removed]=0;
    sleep(TIME_DELAY);
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc < 5)
    {
        // ./a.out 1 1 5 4
        printf("Invalid input, insufficient command line arguments\n");
        exit(-1);
    }
    TOTAL_RESOURCES = atoi(argv[1]);
    arr_of_resources = (int *)malloc(sizeof(int) * TOTAL_RESOURCES);
    for (int i = 0; i < TOTAL_RESOURCES; i++)
    {
        arr_of_resources[i] = atoi(argv[i + 2]);
    }
    TOTAL_THREADS = atoi(argv[TOTAL_RESOURCES + 2]);
    pthread_t arr_thread[TOTAL_THREADS];
    requests = (int **)malloc(sizeof(int *) * TOTAL_THREADS);
    max_requests = (int **)malloc(sizeof(int *) * TOTAL_THREADS);
    keep_alive = (bool *)malloc(sizeof(bool) * TOTAL_THREADS);
    time_stamp = (int *)malloc(sizeof(int) * TOTAL_THREADS);
    for (int i = 0; i < TOTAL_THREADS; i++)
    {
        requests[i] = (int *)malloc(sizeof(int) * TOTAL_RESOURCES);
        max_requests[i] = (int *)malloc(sizeof(int) * TOTAL_RESOURCES);
        keep_alive[i] = 1;
        time_stamp[i] = 0;
    }
    pthread_t t_dash;
    function_no=0;
    pthread_create(&t_dash, NULL, deadlock_detection, NULL);
    for(int i = 0; i < TOTAL_THREADS; i++){
        int * thread_num=(int *)malloc(sizeof(int)*1);
        *thread_num=i;
        
        pthread_create(&arr_thread[i], NULL, P_x, thread_num);
    }
    TIME_DELAY = atoi(argv[TOTAL_RESOURCES + 3]);
    while(1){

    }
}