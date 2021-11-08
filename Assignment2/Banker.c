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
pthread_mutex_t mutex_for_arr_of_resource = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_deadlock_detection = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_alive_threads = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_time = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_time_stamp = PTHREAD_MUTEX_INITIALIZER;
int max_total_resources;
int total_no_of_threads;
int TIME_INTERVAL_d;
int time_stamp_counter = 0;
int ith_HEURISTICS = 1;
bool deadlock_resolved = true;
int arr_of_resources[1000], maxarr_of_resources[1000], time_stamp[1000], arr_of_resources_copy[1000];
bool alive_threads[1000];
int needed_requests[1000][1000], max_needed_requests[1000][1000];
int testing = 0;
int sum_of_array(int requests_needed_1d[], int len)
{
    //calculate the sum of array
    int res = 0;
    int i = 0;
    while (i < len)
    {
        res += requests_needed_1d[i];
        ++i;
    }
    return res;
}
int bool_array_sum(bool arr[], int len)
{
    //calculates the sum of boolean array
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
    //calculates the min number
    return x < y ? x : y;
}
int max(int x, int y)
{
    return x > y ? x : y;
}
int randint(int x, int y)
{
    //generates a random number between [x,y] both inclusive
    return x + rand() % (y - x + 1);
}
int randrange(int x, int y)
{
    //generates a random number [x,y) x inclusive and y exlculsive
    return x + rand() % (y - x);
}
void sleep_for_decided(int d)
{
    int select = (700000 + rand() % 800000); //selects random time (0.7d,1.5d)
    usleep(select * d);
}
//heurisitic function below
int heuristic_function1(bool arr_involved_in_deadlock[])
{
    //selects the resource having max sum of resources needed(max(sum_of_array(needed resources)))
    int thread_going_to_be_exitted = 0;
    int thread_going_to_be_exitted_sum = -1;
    int i = 0, variable_sum;
    for (i = 0; i < total_no_of_threads; ++i)
    {
        pthread_mutex_lock(&mutex_deadlock_detection);
        variable_sum = sum_of_array(needed_requests[i], max_total_resources);
        pthread_mutex_unlock(&mutex_deadlock_detection);
        bool can_go_inside = arr_involved_in_deadlock[i] && variable_sum > thread_going_to_be_exitted_sum;
        if (can_go_inside)
        {
            //updating the sum and value
            thread_going_to_be_exitted = i;
            thread_going_to_be_exitted_sum = variable_sum;
        }
    }
    //thread_going to be removed will be removed
    return thread_going_to_be_exitted;
}
int heuristic_function2(bool arr_involved_in_deadlock[])
{
    //selects the resource having max sum of resources max_needed(max(sum_of_array(max_needed)))
    int thread_going_to_be_exitted = 0;
    int thread_going_to_be_exitted_sum = -1;
    int i = 0, variable_sum;
    for (i = 0; i < total_no_of_threads; ++i)
    {
        pthread_mutex_lock(&mutex_deadlock_detection);
        variable_sum = sum_of_array(max_needed_requests[i], max_total_resources);
        pthread_mutex_unlock(&mutex_deadlock_detection);
        bool can_go_inside;
        can_go_inside = arr_involved_in_deadlock[i] && variable_sum > thread_going_to_be_exitted_sum;
        if (can_go_inside)
        {
            thread_going_to_be_exitted = i;
            thread_going_to_be_exitted_sum = variable_sum;
        }
    }
    return thread_going_to_be_exitted;
}
int heuristic_function3(bool arr_involved_in_deadlock[])
{
    //selects youngest thread i.e. having max time_stamp
    int thread_going_to_be_exitted = 0;
    int thread_going_to_be_exitted_time = -1;
    int i = 0;
    for (i = 0; i < total_no_of_threads; ++i)
    {
        pthread_mutex_lock(&mutex_time_stamp);
        int ts = time_stamp[i];
        pthread_mutex_unlock(&mutex_time_stamp);
        if (arr_involved_in_deadlock[i] && ts > thread_going_to_be_exitted_time)
        {
            thread_going_to_be_exitted = i;
            thread_going_to_be_exitted_time = ts;
        }
    }
    return thread_going_to_be_exitted;
}

int heuristic_function4(bool arr_involved_in_deadlock[])
{
    //selects max resource
    int thread_going_to_be_exitted = 0;
    int thread_going_to_be_exitted_value = -1;
    int i = 0, variable;
    bool can_go_inside;
    for (i = 0; i < total_no_of_threads; ++i)
    {
        if (arr_involved_in_deadlock[i])
        {
            //if it is involved in deadlock then only we check
            for (int j = 0; j < max_total_resources; ++j)
            {
                pthread_mutex_lock(&mutex_deadlock_detection);
                //saving locally as we have to use mutex everytime
                variable = needed_requests[i][j];
                pthread_mutex_unlock(&mutex_deadlock_detection);
                can_go_inside = variable > thread_going_to_be_exitted_value;
                if (can_go_inside)
                {
                    //as it is max 
                    thread_going_to_be_exitted = i;
                    thread_going_to_be_exitted_value = variable;
                }
            }
        }
    }
    return thread_going_to_be_exitted;
}
int call_heuristic(int switcher, bool arr_involved_in_deadlock[])
{
    //will call the heuristic according to the switcher
    int res = -1;
    if (switcher == 1)
    {
        //we return heuristic according to switcher
        return heuristic_function1(arr_involved_in_deadlock);
    }
    else if (switcher == 2)
    {
        return heuristic_function2(arr_involved_in_deadlock);
    }
    else if (switcher == 3)
    {
        return heuristic_function3(arr_involved_in_deadlock);
    }
    else if (switcher == 4)
    {
        return heuristic_function4(arr_involved_in_deadlock);
    }
    return res;
}
void *thread_process(void *args)
{
    // selecting ith thread no from the args
    int ith_thread_number;
    ith_thread_number = *((int *)args);
    int requests_needed_1d[max_total_resources];
    int max_requests_needed_1d[max_total_resources];
    while (true)
    {
        if (deadlock_resolved == 0)
        {
            //while deadlock is not resolved we do not spawn more threads
            continue;
        }
        pthread_mutex_lock(&mutex_time);
        //we save the time stamp
        time_stamp[ith_thread_number] = ++time_stamp_counter;
        pthread_mutex_unlock(&mutex_time);

        int non_zero_requests = max_total_resources;
        for (int i = 0; i < max_total_resources; ++i)
        {
            requests_needed_1d[i] = randrange(0, maxarr_of_resources[i] + 1);
            max_requests_needed_1d[i] = requests_needed_1d[i];
            pthread_mutex_lock(&mutex_deadlock_detection);

            needed_requests[ith_thread_number][i] = requests_needed_1d[i];
            pthread_mutex_unlock(&mutex_deadlock_detection);
            if (requests_needed_1d[i] == 0)
            {
                --non_zero_requests;
            }
            pthread_mutex_lock(&mutex_deadlock_detection);
            max_needed_requests[ith_thread_number][i] = max_requests_needed_1d[i];

            pthread_mutex_unlock(&mutex_deadlock_detection);
        }
        pthread_mutex_lock(&mutex_alive_threads);
        alive_threads[ith_thread_number] = 1;
        pthread_mutex_unlock(&mutex_alive_threads);

        // non_zero_requests the requirement of resources, if none end looping
        while (non_zero_requests > 0)
        {
            //while any resources is not provided to the thread we keep on executing
            pthread_mutex_lock(&mutex_alive_threads);
            //if it creates deadlock we break
            int break_from_loop = alive_threads[ith_thread_number] == false;
            pthread_mutex_unlock(&mutex_alive_threads);
            if (break_from_loop)
            {
                break;
            }
            //we give a random pause between [0,max_total_resources]
            int resource_for_now = randint(0, max_total_resources);

            if (requests_needed_1d[resource_for_now] == 0)
            {
                continue;
            }

            int taken;
            pthread_mutex_lock(&mutex_for_arr_of_resource);
            taken = min(arr_of_resources[resource_for_now], requests_needed_1d[resource_for_now]);
            arr_of_resources[resource_for_now] -= taken;
            requests_needed_1d[resource_for_now] -= taken;
            pthread_mutex_unlock(&mutex_for_arr_of_resource);

            pthread_mutex_lock(&mutex_deadlock_detection);
            needed_requests[ith_thread_number][resource_for_now] -= taken;
            pthread_mutex_unlock(&mutex_deadlock_detection);

            if (requests_needed_1d[resource_for_now] == 0)
            {
                --non_zero_requests;
            }
            sleep(randint(1, TIME_INTERVAL_d));
        }

        pthread_mutex_lock(&mutex_alive_threads);
        int should_sleep_be_given = alive_threads[ith_thread_number] == 1;
        pthread_mutex_unlock(&mutex_alive_threads);
        if (should_sleep_be_given)
        {
            // we sleep for time decided
            sleep_for_decided(TIME_INTERVAL_d);
        }

        for (int i = 0; i < max_total_resources; ++i)
        {
            //mutex for global vairbales
            pthread_mutex_lock(&mutex_for_arr_of_resource);
            arr_of_resources[i] += max_requests_needed_1d[i];
            arr_of_resources[i] -= requests_needed_1d[i];
            pthread_mutex_unlock(&mutex_for_arr_of_resource);
        }

        for (int i = 0; i < max_total_resources; ++i)
        {
            pthread_mutex_lock(&mutex_deadlock_detection);
            //as the process is complete it needs no resources
            needed_requests[ith_thread_number][i] = 0;
            pthread_mutex_unlock(&mutex_deadlock_detection);
        }
        pthread_mutex_lock(&mutex_alive_threads);
        //we make it alive if it was 0
        alive_threads[ith_thread_number] = 1;
        pthread_mutex_unlock(&mutex_alive_threads);
    }
    return NULL;
}
int times_deadlock_found[4];
int times_deadlock_checked[4];
int times_to_be_checked = 100;
//for measuring the time dealy
int start_time_deadlock[4];
int end_time_deadlock[4];

void copy_arr()
{
    //we copy the array
    for (int i = 0; i < max_total_resources; ++i)
    {
        arr_of_resources_copy[i] = arr_of_resources[i];
    }
}
void *deadlock_detection(void *arg)
{
    int counter = 0;
    while (true)
    {
        //we copy array each time
        copy_arr();
        if (counter == times_to_be_checked)
        {
            if (testing)
            {
                //if we are testing for times_to_be_checked times we break
                break;
            }
            else
            {
                //printing the data
                printf("We check deadlock for %d times\n", times_deadlock_checked[ith_HEURISTICS - 1]);
                printf("And the deadlock occured for %d time(s)\n", times_deadlock_found[ith_HEURISTICS - 1]);
                if (times_deadlock_found[ith_HEURISTICS - 1])
                {
                    float deadlock_percentage = (float)times_deadlock_found[ith_HEURISTICS - 1] / times_deadlock_checked[ith_HEURISTICS - 1];
                    deadlock_percentage *= 100;
                    //printing the percentage
                    printf("The percentage occurence of deadlock is %f%%\n", deadlock_percentage);
                }
            }
        }
        //incrementing the times deadlock checked
        ++counter;
        ++times_deadlock_checked[ith_HEURISTICS - 1];
        printf("Detecting deadlock ....\n");
        //taking deadlcok found = 0 initially
        bool deadlock_found = false;
        bool arr_involved_in_deadlock[total_no_of_threads];
        for (int i = 0; i < total_no_of_threads; ++i)
        {
            arr_involved_in_deadlock[i] = false;
        }
        for (int i = 0; i < total_no_of_threads; ++i)
        {
            bool ith_thread_can_go = true;

            for (int j = 0; j < max_total_resources; ++j)
            {
                pthread_mutex_lock(&mutex_deadlock_detection);
                bool can_go_inside = arr_of_resources_copy[j] < needed_requests[i][j];
                pthread_mutex_unlock(&mutex_deadlock_detection);
                if (can_go_inside)
                {
                    ith_thread_can_go = false;
                    break;
                }
            }
            if (ith_thread_can_go)
            {

                for (int j = 0; j < max_total_resources; ++j)
                {
                    pthread_mutex_lock(&mutex_deadlock_detection);
                    arr_of_resources_copy[j] += (max_needed_requests[i][j] - needed_requests[i][j]);
                    pthread_mutex_unlock(&mutex_deadlock_detection);
                }
            }
            else
            {
                deadlock_found = true;
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

        int thread_going_to_be_exitted = -1;
        deadlock_resolved = !deadlock_found;
        if (bool_array_sum(arr_involved_in_deadlock, total_no_of_threads))
        {
            //we print the list of threads involved in deadlock
            printf("The list of threads involved in deadlock are:\n");
            for (int i = 0; i < total_no_of_threads; ++i)
            {
                if (arr_involved_in_deadlock[i])
                {
                    //printing the thread as it is involved in deadlock
                    printf("%d\n", i);
                }
            }
            printf("\n");
        }
        else
        {
            //no threads involved in deadlock
            printf("No thread is involved in deadlock\n");
        }

        if (deadlock_found)
        {
            if (start_time_deadlock[ith_HEURISTICS - 1] == -1)
            {
                //we initialize the start time if not inizlized
                start_time_deadlock[ith_HEURISTICS - 1] = counter;
            }
            //we update the end time
            end_time_deadlock[ith_HEURISTICS - 1] = counter;
            ++times_deadlock_found[ith_HEURISTICS - 1];
            thread_going_to_be_exitted = call_heuristic(ith_HEURISTICS, arr_involved_in_deadlock);
            pthread_mutex_lock(&mutex_alive_threads);
            //as it creates deadlock we make it false to eliminate the thread
            alive_threads[thread_going_to_be_exitted] = false;
            pthread_mutex_unlock(&mutex_alive_threads);

            printf("WE SKIPPED AND QUEUED THE PROCESS AT %dth THREAD\n", thread_going_to_be_exitted);
        }
        //sleep time between each deadlock checks
        sleep(TIME_INTERVAL_d);
    }
    return NULL;
}
void unit_test();

signed main(int argc, char **argv)
{

    if (argc == 2)
    {
        //for testing
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
        printf("format of input = ./a.out [Number of Resources] [Number of threads to use in the simulation] || [Deadlock detection check interval d in seconds.] || [Heuristic function to be executed] [ max instances of <Resource1> <Resource2> ...  ] \n");
        exit(-1);
    }
    max_total_resources = atoi(argv[1]);
    if (max_total_resources == 0)
    {
        //resources should be non zero
        printf("Please provide at least 1 max instance\n");
        exit(-1);
    }
    if (max_total_resources + 5 != argc)
    {
        //checking the input
        printf("incorrect input format\n");
        printf("The no of max instances is incorrect\n");
        exit(-1);
    }
    total_no_of_threads = atoi(argv[2]);
    if (total_no_of_threads == 0)
    {
        //threads should be non zero
        printf("Please provide at least 1 thread\n");
        exit(-1);
    }
    TIME_INTERVAL_d = atoi(argv[3]);
    ith_HEURISTICS = atoi(argv[4]);
    //initializing the arrays
    memset(arr_of_resources, 0, 1000 * sizeof(int));
    memset(maxarr_of_resources, 0, 1000 * sizeof(int));
    memset(time_stamp, 0, 1000 * sizeof(int));
    for (int i = 0; i < 1000; i++)
    {
        //all threads should work
        alive_threads[i] = true;
    }

    for (int i = 0; i < max_total_resources; ++i)
    {
        //taking input from argvs
        arr_of_resources[i] = atoi(argv[i + 4]);
        maxarr_of_resources[i] = arr_of_resources[i];
    }
    pthread_t arr_thread[total_no_of_threads];

    for (int i = 0; i < total_no_of_threads; ++i)
    {
        for (int j = 0; j < max_total_resources; ++j)
        {
            //initializing the arrays
            needed_requests[i][j] = 0;
            max_needed_requests[i][j] = 0;
        }
    }
    for (int i = 0; i < 4; i++)
    {
        //initializing the start and end time
        start_time_deadlock[i] = -1;
        end_time_deadlock[i] = -1;
    }
    int *ith_thread_number[total_no_of_threads];
    printf("Threads=%d\n", total_no_of_threads);
    printf("Total resources=%d\n", max_total_resources);
    printf("Time delay= %ds\n", TIME_INTERVAL_d);
    printf("Function no= %d\n", ith_HEURISTICS);
    for (int i = 0; i < total_no_of_threads; ++i)
    {
        ith_thread_number[i] = (int *)malloc(sizeof(int));
        *ith_thread_number[i] = i;
        //calling each processes using threads
        pthread_create(&arr_thread[i], NULL, thread_process, ith_thread_number[i]);
    }
    pthread_t tdash;
    pthread_create(&tdash, NULL, deadlock_detection, NULL);
    //waiting for the tdash thread to create so that the program does not exit
    pthread_join(tdash, NULL);
}
void unit_test()
{
    testing = 1;
    max_total_resources = 2;
    total_no_of_threads = 2;
    TIME_INTERVAL_d = 1;
    ith_HEURISTICS = 100;
    times_to_be_checked = 100;
    memset(arr_of_resources, 0, 1000 * sizeof(int));
    memset(maxarr_of_resources, 0, 1000 * sizeof(int));
    memset(time_stamp, 0, 1000 * sizeof(int));
    for (int i = 0; i < 4; i++)
    {
        start_time_deadlock[i] = -1;
        end_time_deadlock[i] = -1;
    }
    for (int i = 0; i < 1000; i++)
    {
        alive_threads[i] = true;
    }
    FILE *fp;
    fp = fopen("Data.txt", "w");
    fprintf(fp, "Threads=%d\n", total_no_of_threads);
    fprintf(fp, "Total resources=%d\n", max_total_resources);
    fprintf(fp, "Time delay= %ds\n\n", TIME_INTERVAL_d);
    for (int i = 0; i < max_total_resources; ++i)
    {
        if (i == 0)
        {
            arr_of_resources[i] = 13;
        }
        else if (i == 1)
        {
            arr_of_resources[i] = 17;
        }
        maxarr_of_resources[i] = arr_of_resources[i];
    }
    //file printing the data
    fprintf(fp, "The array of resources is \n");
    for (int i = 0; i < max_total_resources; i++)
    {
        fprintf(fp, "%d ", arr_of_resources[i]);
    }
    fprintf(fp, "\n\n");
    pthread_t arr_thread[total_no_of_threads];

    for (int i = 0; i < total_no_of_threads; ++i)
    {
        for (int j = 0; j < max_total_resources; ++j)
        {
            //initializing the resources
            needed_requests[i][j] = 0;
            max_needed_requests[i][j] = 0;
        }
    }
    int *ith_thread_number[total_no_of_threads];
    printf("Threads=%d\n", total_no_of_threads);
    printf("Total resources=%d\n", max_total_resources);
    printf("Time delay= %ds\n", TIME_INTERVAL_d);
    printf("The array of resources is \n");
    for (int i = 0; i < max_total_resources; i++)
    {
        printf("%d ", arr_of_resources[i]);
    }
    printf("\n\n");
    for (int i = 0; i < total_no_of_threads; ++i)
    {
        ith_thread_number[i] = (int *)malloc(sizeof(int));
        *ith_thread_number[i] = i;
        pthread_create(&arr_thread[i], NULL, thread_process, ith_thread_number[i]);
    }
    for (int i = 1; i <= 4; i++)
    {
        ith_HEURISTICS = i;
        printf("Function no= %d\n", ith_HEURISTICS);
        pthread_t tdash;
        pthread_create(&tdash, NULL, deadlock_detection, NULL);
        pthread_join(tdash, NULL);
        printf("\n\nWe checked the deadlock for %d\n", times_deadlock_checked[ith_HEURISTICS - 1]);
        printf("And the deadlock occured for %d time(s)\n", times_deadlock_found[ith_HEURISTICS - 1]);
        if (times_deadlock_checked[ith_HEURISTICS - 1])
        {

            float deadlock_percentage = (float)times_deadlock_found[ith_HEURISTICS - 1] / times_deadlock_checked[ith_HEURISTICS - 1];
            deadlock_percentage *= 100;
            printf("The percentage occurence of deadlock is %f%%\n", deadlock_percentage);
        }
        if (times_deadlock_found[ith_HEURISTICS - 1] - 1 > 0)
        {
            float deadlock_delay = (end_time_deadlock[ith_HEURISTICS - 1] - start_time_deadlock[ith_HEURISTICS - 1]);
            deadlock_delay *= TIME_INTERVAL_d;
            deadlock_delay /= (times_deadlock_found[ith_HEURISTICS - 1] - 1);
            printf("The avg time interval between deadlocks is %f seconds\n", deadlock_delay);
        }
    }

    for (int i = 1; i <= 4; ++i)
    {
        //executing each heurisitics for testing
        ith_HEURISTICS = i;
        fprintf(fp, "We check deadlock for %d times\n", times_deadlock_checked[ith_HEURISTICS - 1]);
        fprintf(fp, "And the deadlock occured for %d time(s)\n", times_deadlock_found[ith_HEURISTICS - 1]);
        if (times_deadlock_checked[ith_HEURISTICS - 1])
        {
            // we print the percentage of deadlock found
            float deadlock_percentage = (float)times_deadlock_found[ith_HEURISTICS - 1] / times_deadlock_checked[ith_HEURISTICS - 1];
            deadlock_percentage *= 100;
            fprintf(fp, "The percentage occurence of deadlock is %f%%\n", deadlock_percentage);
        }
        if (times_deadlock_found[ith_HEURISTICS - 1] - 1 > 0)
        {
            float deadlock_delay = (end_time_deadlock[ith_HEURISTICS - 1] - start_time_deadlock[ith_HEURISTICS - 1]);
            deadlock_delay *= TIME_INTERVAL_d;
            deadlock_delay /= (times_deadlock_found[ith_HEURISTICS - 1] - 1);
            //printing the avg dealy between deadlocks
            fprintf(fp, "The avg time interval between deadlocks is %f seconds\n", deadlock_delay);
        }
        fprintf(fp, "\n");
    }
}
