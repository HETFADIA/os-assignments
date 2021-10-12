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
pthread_mutex_t mutex_time_stamp = PTHREAD_MUTEX_INITIALIZER;
int MAX_TOTAL_RESOURCES;
int TOTAL_THREADS;
int TIME_DELAY;
int time_stamp_counter = 0;
int function_no = 1;
bool deadlock_resolved = true;
int arr_of_resources[1000], maxarr_of_resources[1000], time_stamp[1000];
bool keep_alive[1000];
int requests[1000][1000], max_requests[1000][1000];
int testing=0;
int sum(int arr[], int len)
{
    int res = 0;
    int i = 0;
    while (i < len)
    {
        res += arr[i];
        ++i;
    }
    return res;
}
int min(int x, int y)
{
    return x < y ? x : y;
}
int max(int x, int y)
{
    return x > y ? x : y;
}
int randint(int x, int y)
{
    return x + rand() % (y - x + 1);
}
int randrange(int x, int y)
{
    return x + rand() % (y - x);
}
void sleep_for_decided(int d)
{
    int select = (700000 + rand() % 800000); //selects random time (0.7d,1.5d)
    usleep(select * d);
}

int heuristics1(bool arr_involved_in_deadlock[])
{
    //selects the resource having max sum of resources needed(max(sum(needed resources)))
    int to_be_removed = 0;
    int to_be_removed_sum = -1;
    int i = 0;
    for (i = 0; i < TOTAL_THREADS; ++i)
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
    int i = 0;
    for (i = 0; i < TOTAL_THREADS; ++i)
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
    int i = 0;
    for (i = 0; i < TOTAL_THREADS; ++i)
    {
        pthread_mutex_lock(&mutex_time_stamp);
        int ts = time_stamp[i];
        pthread_mutex_unlock(&mutex_time_stamp);
        if (arr_involved_in_deadlock[i] && ts > to_be_removed_time)
        {
            to_be_removed = i;
            to_be_removed_time = ts;
        }
    }
    return to_be_removed;
}

int heuristics4(bool arr_involved_in_deadlock[])
{
    //selects max resource
    int to_be_removed = 0;
    int to_be_removed_value = -1;
    int i = 0;
    for (i = 0; i < TOTAL_THREADS; ++i)
    {
        if (arr_involved_in_deadlock[i])
        {
            for (int j = 0; j < MAX_TOTAL_RESOURCES; ++j)
            {
                pthread_mutex_lock(&mutex_deadlock_detection);
                int temp = requests[i][j];
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
int call_heuristic(int switcher, bool arr_involved_in_deadlock[])
{
    int res = -1;
    if (switcher == 1)
    {
        return heuristics1(arr_involved_in_deadlock);
    }
    else if (switcher == 2)
    {
        return heuristics2(arr_involved_in_deadlock);
    }
    else if (switcher == 3)
    {
        return heuristics3(arr_involved_in_deadlock);
    }
    else if (switcher == 4)
    {
        return heuristics4(arr_involved_in_deadlock);
    }
    return res;
}
void *thread_process(void *args)
{
    int thread_num;
    thread_num = *((int *)args);
    int arr[MAX_TOTAL_RESOURCES];
    int save_temp[MAX_TOTAL_RESOURCES];
    while (true)
    {
        if (deadlock_resolved == 0)
        {
            continue;
        }
        pthread_mutex_lock(&mutex_time);
        time_stamp[thread_num] = ++time_stamp_counter;
        pthread_mutex_unlock(&mutex_time);

        int TRACK = MAX_TOTAL_RESOURCES;
        for (int i = 0; i < MAX_TOTAL_RESOURCES; ++i)
        {
            arr[i] = randrange(0, maxarr_of_resources[i] + 1);
            if (!arr[i])
            {
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
            int break_from_loop = keep_alive[thread_num] == false;
            pthread_mutex_unlock(&mutex_keep_alive);
            if (break_from_loop)
            {
                break;
            }

            int resource_for_now = randint(0, MAX_TOTAL_RESOURCES);

            if (arr[resource_for_now] == 0)
            {
                continue;
            }

            int taken;
            pthread_mutex_lock(&mutex_arr_of_resource);
            taken = min(arr_of_resources[resource_for_now], arr[resource_for_now]);
            arr_of_resources[resource_for_now] -= taken;
            arr[resource_for_now] -= taken;
            pthread_mutex_unlock(&mutex_arr_of_resource);

            pthread_mutex_lock(&mutex_deadlock_detection);
            requests[thread_num][resource_for_now] -= taken;
            pthread_mutex_unlock(&mutex_deadlock_detection);

            if (arr[resource_for_now] == 0)
            {
                --TRACK;
            }
            sleep(randint(1, TIME_DELAY));
        }

        pthread_mutex_lock(&mutex_keep_alive);
        int should_sleep_be_given = keep_alive[thread_num] == 1;
        pthread_mutex_unlock(&mutex_keep_alive);
        if (should_sleep_be_given)
        {
            sleep_for_decided(TIME_DELAY);
        }

        for (int i = 0; i < MAX_TOTAL_RESOURCES; ++i)
        {
            //mutex lock 2
            pthread_mutex_lock(&mutex_arr_of_resource);
            arr_of_resources[i] += save_temp[i];
            arr_of_resources[i] -= arr[i];
            pthread_mutex_unlock(&mutex_arr_of_resource);
            //mutex unlock
        }

        for (int i = 0; i < MAX_TOTAL_RESOURCES; ++i)
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
int times_deadlock_found[4];
int times_deadlock_checked[4];
int times_to_be_checked=5;
void deadlock_detection()
{
    int counter = 0;
    while (true)
    {
        if (counter == times_to_be_checked && testing)
        {
            return;
        }
        ++counter;
        ++times_deadlock_checked[function_no - 1];
        printf("Detecting deadlock ....\n");

        bool deadlock_found = true;
        bool arr_involved_in_deadlock[TOTAL_THREADS];
        for (int i = 0; i < TOTAL_THREADS; ++i)
        {
            arr_involved_in_deadlock[i] = false;
        }
        for (int i = 0; i < TOTAL_THREADS; ++i)
        {
            bool ith_thread_can_go = true;

            for (int j = 0; j < MAX_TOTAL_RESOURCES; ++j)
            {
                pthread_mutex_lock(&mutex_deadlock_detection);
                bool can_go_inside = arr_of_resources[j] < requests[i][j];
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
        if (deadlock_found == false)
        {
            printf("DEADLOCK NOT FOUND\n");
        }
        else
        {
            printf("Deadlock found\n");
        }

        int to_be_removed = -1;
        deadlock_resolved = !deadlock_found;
        if (deadlock_found)
        {
            ++times_deadlock_found[function_no - 1];
            to_be_removed = call_heuristic(function_no, arr_involved_in_deadlock);
            pthread_mutex_lock(&mutex_keep_alive);
            keep_alive[to_be_removed] = false;
            pthread_mutex_unlock(&mutex_keep_alive);

            printf("WE SKIPPED AND QUEUED THE PROCESS AT %dth THREAD\n", to_be_removed);
        }

        sleep(TIME_DELAY);
    }
}
void unit_test();

signed main(int argc, char **argv)
{
    
    if (argc == 2)
    {
        char *test = "test";
        if (strcmp(test, argv[1]) == 0)
        {
            unit_test();
            return 0;
        }
    }
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
    if (MAX_TOTAL_RESOURCES == 0)
    {
        printf("Please provide at least 1 max instance\n");
        exit(-1);
    }
    if (MAX_TOTAL_RESOURCES + 5 != argc)
    {
        printf("incorrect input format\n");
        printf("The no of max instances is incorrect\n");
        exit(-1);
    }
    TOTAL_THREADS = atoi(argv[2]);
    if (TOTAL_THREADS == 0)
    {
        printf("Please provide at least 1 thread\n");
        exit(-1);
    }
    TIME_DELAY = atoi(argv[3]);
    function_no = atoi(argv[4]);

    memset(arr_of_resources, 0, 1000 * sizeof(int));
    memset(maxarr_of_resources, 0, 1000 * sizeof(int));
    memset(time_stamp, 0, 1000 * sizeof(int));
    for (int i = 0; i < 1000; i++)
    {
        keep_alive[i] = true;
    }

    for (int i = 0; i < MAX_TOTAL_RESOURCES; ++i)
    {
        arr_of_resources[i] = atoi(argv[i + 4]);
        maxarr_of_resources[i] = arr_of_resources[i];
    }
    pthread_t arr_thread[TOTAL_THREADS];

    for (int i = 0; i < TOTAL_THREADS; ++i)
    {
        for (int j = 0; j < MAX_TOTAL_RESOURCES; ++j)
        {
            requests[i][j] = 0;
            max_requests[i][j] = 0;
        }
    }
    int *thread_num[TOTAL_THREADS];
    printf("Threads=%d\n", TOTAL_THREADS);
    printf("Total resources=%d\n", MAX_TOTAL_RESOURCES);
    printf("Time delay= %ds\n", TIME_DELAY);
    printf("Function no= %d\n", function_no);
    for (int i = 0; i < TOTAL_THREADS; ++i)
    {
        thread_num[i] = (int *)malloc(sizeof(int));
        *thread_num[i] = i;
        pthread_create(&arr_thread[i], NULL, thread_process, thread_num[i]);
    }
    deadlock_detection();
}
void unit_test()
{
    testing=1;
    MAX_TOTAL_RESOURCES = 2;
    TOTAL_THREADS = 2;
    TIME_DELAY = 1;
    function_no=1;
    times_to_be_checked=100;
    memset(arr_of_resources, 0, 1000 * sizeof(int));
    memset(maxarr_of_resources, 0, 1000 * sizeof(int));
    memset(time_stamp, 0, 1000 * sizeof(int));
    for (int i = 0; i < 1000; i++)
    {
        keep_alive[i] = true;
    }

    for (int i = 0; i < MAX_TOTAL_RESOURCES; ++i)
    {
        if(i==0){
            arr_of_resources[i] = 13;
        }
        else if(i==1){
            arr_of_resources[i] = 17;
        }
        maxarr_of_resources[i] = arr_of_resources[i];
    }
    pthread_t arr_thread[TOTAL_THREADS];

    for (int i = 0; i < TOTAL_THREADS; ++i)
    {
        for (int j = 0; j < MAX_TOTAL_RESOURCES; ++j)
        {
            requests[i][j] = 0;
            max_requests[i][j] = 0;
        }
    }
    int *thread_num[TOTAL_THREADS];
    printf("Threads=%d\n", TOTAL_THREADS);
    printf("Total resources=%d\n", MAX_TOTAL_RESOURCES);
    printf("Time delay= %ds\n", TIME_DELAY);
    printf("Function no= %d\n", function_no);
    for (int i = 0; i < TOTAL_THREADS; ++i)
    {
        thread_num[i] = (int *)malloc(sizeof(int));
        *thread_num[i] = i;
        pthread_create(&arr_thread[i], NULL, thread_process, thread_num[i]);
    }
    for (int i = 1; i <= 4; i++)
    {
        function_no=i;
        deadlock_detection();
        printf("\n\nWe check deadlock for %d\n",times_deadlock_checked[function_no-1]);
        printf("And the deadlock occured for %d time(s)\n",times_deadlock_found[function_no-1]);
        if(times_deadlock_found[function_no-1]){
            float deadlock_occurs_after=(float)times_deadlock_checked[function_no-1]/times_deadlock_found[function_no-1];
            deadlock_occurs_after*=TIME_DELAY;
            printf("The time delay is %f seconds\n",deadlock_occurs_after);
        }
    }
    FILE * fp;
    fp=fopen("data.txt","w");
    for(int i=1;i<=4;++i){
        function_no=i;
        fprintf(fp,"We check deadlock for %d\n",times_deadlock_checked[function_no-1]);
        fprintf(fp,"And the deadlock occured for %d time(s)\n",times_deadlock_found[function_no-1]);
        if(times_deadlock_found[function_no-1]){
            float deadlock_occurs_after=(float)times_deadlock_checked[function_no-1]/times_deadlock_found[function_no-1];
            deadlock_occurs_after*=TIME_DELAY;
            fprintf(fp,"The time delay is %f seconds\n",deadlock_occurs_after);
        }
    }
}