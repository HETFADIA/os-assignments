#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>

int r = 0, Ts = 0, N = 0, algorithm = 0;
int requests = 0;
double seeking_time = 0;
double time_for_sector = 0;
double sum_square = 0, sum_time = 0, count_times = 0;
double min_time = 1e9;
double max_time = -1e9;

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

typedef struct node
{
    int platter, cylinder, sector, number_of_sectors;
    struct node *next;
}(node);
node *new_node(int platter, int cylinder, int sector, int number_of_sectors)
{
    node *temp = (node *)malloc(sizeof(node));
    temp->platter = platter;
    temp->cylinder = cylinder;
    temp->sector = sector;
    temp->number_of_sectors = number_of_sectors;
    temp->next = NULL;
    return temp;
}
node *head = NULL;
node *tail = NULL;
int queue_size = 0;
void append(int platter, int cylinder, int sector, int number_of_sectors)
{
    node *newnode = new_node(platter, cylinder, sector, number_of_sectors);
    if (!tail)
    {
        head = tail = newnode;
    }
    else
    {
        tail->next = newnode;
        tail = newnode;
    }
    ++queue_size;
}
node *pop(int index)
{
    node *temp = head;
    if (index == 0)
    {
        if (head)
        {
            head = head->next;
            --queue_size;
        }
        return temp;
    }
    for (int i = 0; i < index - 1; i++)
    {
        temp = temp->next;
    }
    node *ret = temp->next;
    temp->next = ret->next;
    --queue_size;
    return ret;
}
void _remove(node *r)
{
    if (r == head)
    {
        head = head->next;
    }
    else
    {
        node *temp = head;
        while (temp->next != r)
        {
            temp = temp->next;
        }
        temp->next = r->next;
    }
    free(r);
    --queue_size;
}

void make_requests()
{
    for (int i = 0; i < requests; ++i)
    {
        append(randint(1, 4), randint(1, 25), randint(1, 20), randint(1, 20));
    }
}
double responce_time = 0;
void display_values(double t)
{
    responce_time += t;
    sum_time += responce_time;
    sum_square += responce_time * responce_time;
    count_times++;

    if (responce_time < min_time)
    {
        min_time = responce_time;
    }
    if (responce_time > max_time)
    {
        max_time = responce_time;
    }

    double stddev = sqrt(sum_square / count_times - (sum_time / count_times) * (sum_time / count_times));
    printf("\nBelow are the values:\n");
    printf("Time: %lf\n", t);
    printf("Responce time:%lf\n", responce_time);
    printf("Queue size: %d\n", queue_size);
    printf("Total time:%lf\n", sum_time);
    printf("Counts: %lf\n", count_times); // number of requests
    printf("Throughput %lf\n", count_times / responce_time * 1000.0);
    printf("Average Time: %lf\n", sum_time / count_times);
    printf("Minimum Time: %lf\n", min_time);
    printf("Maximum Time: %lf\n", max_time);
    printf("Stddev: %lf\n", stddev);
    if (queue_size == 0)
    {
        printf("\n");
        printf("Throughput %lf\n", count_times / responce_time * 1000.0);
        printf("%lf\n", sum_time / count_times);
        printf("%lf\n", min_time);
        printf("%lf\n", max_time);
        printf("%lf\n", stddev);
    }
}
int calc_sector(int diff)
{
    double time_req = diff * seeking_time;
    double rate = (double)r / 60000.0;
    double revs = rate * time_req;
    double secs = revs * 20.0;
    return ceil(secs);
}

void FIFO()
{
    int curr_cylinder = randint(1, 25);
    int curr_sector = randint(1, 20);
    int platter,cylinder,sector,number_of_sectors,track_diff;
    while (queue_size)
    {
        node *ptr = pop(0);
        platter = ptr->platter;
        cylinder = ptr->cylinder;
        track_diff = cylinder - curr_cylinder;
        if(track_diff<0){
            track_diff=-track_diff;
        }
        sector = ptr->sector;
        number_of_sectors = ptr->number_of_sectors;
        free(ptr);
        int secs = calc_sector(track_diff);
        curr_sector = (curr_sector - 1 + secs) % 20 ;
        ++curr_sector;
        int sec_diff = ((sector - curr_sector + 20) % 20);
        if(sec_diff<0){
            sec_diff+=20;
        }
        double time = track_diff * seeking_time + (sec_diff + number_of_sectors) * time_for_sector;
        curr_sector = (curr_sector - 1 + number_of_sectors) % 20 + 1;
        if(curr_sector<0){
            curr_sector+=20;
        }
        curr_cylinder = cylinder;
        display_values(time);
    }
}
void Random_scheduling_policy()
{
    int curr_cylinder = randint(1, 25);
    int curr_sector = randint(1, 20);
    int track_diff, secs;
    int number_of_sectors;
    while (queue_size)
    {

        int ind = randrange(0, queue_size);
        node *ptr = pop(ind);
        number_of_sectors = ptr->number_of_sectors;
        int platter = ptr->platter;
        int cylinder = ptr->cylinder;
        track_diff = abs(cylinder - curr_cylinder);
        int sector = ptr->sector;
        free(ptr);
        secs = calc_sector(track_diff);
        curr_sector = (curr_sector - 1 + secs) % 20 + 1;
        int sec_diff = ((sector - curr_sector + 20) % 20);
        if(sec_diff<0){
            sec_diff+=20;
        }
        double time = track_diff * seeking_time + (sec_diff + number_of_sectors) * time_for_sector;
        curr_sector = (curr_sector - 1 + number_of_sectors) % 20 + 1;
        if(curr_sector<0){
            curr_sector+=20;
        }
        curr_cylinder = cylinder;
        display_values(time);
    }
}


void SSTF()
{
    int curr_cylinder = randint(1, 25);
    int curr_sector = randint(1, 20);
    int platter,cylinder,sector,number_of_sectors;
    int track_diff,secs;
    while (queue_size)
    {
        struct node *ptr = head;
        struct node *closest = head;
        while (ptr)
        {
            if (abs(ptr->cylinder - curr_cylinder) < abs(closest->cylinder - curr_cylinder))
            {
                closest = ptr;
            }
            ptr = ptr->next;
        }
        platter = closest->platter;
        cylinder = closest->cylinder;
        sector = closest->sector;
        number_of_sectors = closest->number_of_sectors;
        _remove(closest);
        track_diff = abs(cylinder - curr_cylinder);
        secs = calc_sector(track_diff);
        curr_sector = (curr_sector - 1 + secs) % 20 + 1;
        if(curr_sector<sector){
            curr_sector+=20;
        }
        int sec_diff;
        sec_diff = (abs(sector - curr_sector + 20) % 20);
        if(sec_diff<0){
            sec_diff+=20;
        }
        double time = track_diff * seeking_time + (sec_diff + number_of_sectors) * time_for_sector;
        curr_sector = (curr_sector - 1 + number_of_sectors) % 20 + 1;
        if(curr_sector<0){
            curr_sector+=20;
        }
        curr_cylinder = cylinder;
        display_values(time);
    }
}
void SCAN()
{
    int curr_cylinder = randint(1, 25);
    int curr_sector = randint(1, 20);
    int platter,cylinder,sector,number_of_sectors,track_diff,secs;
    while (queue_size)
    {
        for (int i = curr_cylinder; i <= 25; i++)
        {
            node *ptr = head;
            while (ptr)
            {
                node *next = ptr->next;
                if (ptr->cylinder == i)
                {
                    platter = ptr->platter;
                    cylinder = ptr->cylinder;
                    sector = ptr->sector;
                    number_of_sectors = ptr->number_of_sectors;
                    _remove(ptr);
                    track_diff = abs(cylinder - curr_cylinder);
                    secs = calc_sector(track_diff);
                    curr_sector = (curr_sector - 1 + secs) % 20 + 1;
                    if(curr_sector<0){
                        curr_sector+=20;
                    }
                    int sec_diff = (abs(sector - curr_sector + 20) % 20);
                    if(sec_diff<0){
                        sec_diff+=20;
                    }
                    double time = track_diff * seeking_time + (sec_diff + number_of_sectors) * time_for_sector;
                    curr_sector = (curr_sector - 1 + number_of_sectors) % 20 + 1;
                    if(curr_sector<0){
                        curr_sector+=20;
                    }
                    curr_cylinder = cylinder;
                    display_values(time);
                }
                ptr = next;
            }
        }
        for (int i = 25; i >= 0 && queue_size; --i)
        {
            node *ptr = head;
            while (ptr)
            {
                node *next = ptr->next;
                if (ptr->cylinder == i)
                {

                    platter = ptr->platter;
                    cylinder = ptr->cylinder;
                    sector = ptr->sector;
                    number_of_sectors = ptr->number_of_sectors;
                    _remove(ptr);
                    int track_diff = abs(cylinder - curr_cylinder);
                    if(track_diff<0){
                        track_diff=-track_diff;
                    }
                    int secs = calc_sector(track_diff);
                    curr_sector = (curr_sector - 1 + secs) % 20 + 1;
                    if(curr_sector<=0){
                        curr_sector=20+curr_sector;
                    }
                    int sec_diff = (abs(sector - curr_sector + 20) % 20);
                    if(sec_diff<0){
                        sec_diff=-sec_diff;
                    }
                    double time = track_diff * seeking_time + (sec_diff + number_of_sectors) * time_for_sector;
                    curr_sector = (curr_sector - 1 + number_of_sectors) % 20 + 1;
                    if(curr_sector<0){
                        curr_sector+=20;
                    }
                    curr_cylinder = cylinder;
                    display_values(time);
                }
                ptr = next;
            }
        }
    }
}

void CSCAN()
{
    int curr_cylinder = randint(1, 25);
    int curr_sector = randint(1, 20);

    while (queue_size)
    {
        for (int i = curr_cylinder; i <= 25; i++)
        {
            node *ptr = head;
            while (ptr)
            {
                node *next = ptr->next;
                if (ptr->cylinder == i)
                {
                    int platter = ptr->platter,cylinder = ptr->cylinder,sector = ptr->sector,number_of_sectors = ptr->number_of_sectors;
                    _remove(ptr);
                    int track_diff = abs(cylinder - curr_cylinder);
                    if(track_diff<0){
                        track_diff=-track_diff;
                    }
                    int secs = calc_sector(track_diff);
                    curr_sector = (curr_sector - 1 + secs) % 20 + 1;
                    if(curr_sector<0){
                        curr_sector=20+curr_sector;
                    }
                    int sec_diff = (abs(sector - curr_sector + 20) % 20);
                    if(sec_diff<0){
                        sec_diff=-sec_diff;
                    }
                    double time = track_diff * seeking_time + (sec_diff + number_of_sectors) * time_for_sector;
                    curr_sector = (curr_sector - 1 + number_of_sectors) % 20 + 1;
                    if(curr_sector<0){
                        curr_sector=20+curr_sector;
                    }
                    curr_cylinder = cylinder;
                    display_values(time);
                }
                ptr = next;
            }
        }
        for (int i = 0; i < curr_cylinder; ++i)
        {
            node *ptr = head;
            while (ptr)
            {
                node *next = ptr->next;
                if (ptr->cylinder == i)
                {

                    int platter = ptr->platter,cylinder = ptr->cylinder,sector = ptr->sector,number_of_sectors = ptr->number_of_sectors;
                    _remove(ptr);
                    int track_diff = abs(cylinder - curr_cylinder);
                    if(track_diff<0){
                        track_diff=-track_diff;
                    }
                    int secs = calc_sector(track_diff);
                    curr_sector = (curr_sector - 1 + secs) % 20 + 1;
                    if(curr_sector<0){
                        curr_sector=-curr_sector;
                    }
                    int sec_diff = (abs(sector - curr_sector + 20) % 20);
                    if(sec_diff<0){
                        sec_diff=-sec_diff;
                    }
                    double time = track_diff * seeking_time + (sec_diff + number_of_sectors) * time_for_sector;
                    curr_sector = (curr_sector - 1 + number_of_sectors) % 20 + 1;
                    if(curr_sector<0){
                        curr_sector=20+curr_sector;
                    }
                    curr_cylinder = cylinder;
                    display_values(time);
                }
                ptr = next;
            }
        }
    }
}
void simulate()
{
    if (algorithm == 0)
    {
        Random_scheduling_policy();
        return;
    }
    if (algorithm == 1)
    {
        FIFO();
        return;
    }
    if (algorithm == 2)
    {
        SSTF();
        return;
    }
    if (algorithm == 3)
    {
        SCAN();
        return;
    }
    if (algorithm == 4)
    {
        CSCAN();
        return;
    }

    printf("Error: algorithm must an integer between [0,5]\n");
    exit(-1);
}
int main(int argc, char **argv)
{
    if (argc < 5)
    {
        printf("Usage: %s <r> <N> <Ts> <queue_size> <algorithm>\n", argv[0]);
        printf("Below is an example\n");
        printf("%s 7500 512 4 1000 0 \n ", argv[0]);
        return -1;
    }
    r = atoi(argv[1]);
    if(r<=0)
    {
        printf("Error: r must be a positive integer\n");
        return -1;
    }
    N = atoi(argv[2]);
    if(N<0){
        printf("Error: N must be a positive integer\n");
        exit(-1);
    }
    Ts = atoi(argv[3]);
    requests = atoi(argv[4]);
    if (requests < 0)
    {
        printf("Error: requests must be non negative\n");
        exit(-1);
    }
    algorithm = atoi(argv[5]);
    if (algorithm < 0 || algorithm > 4)
    {
        printf("Error: algorithm must an integer between [0,5]\n");
        exit(-1);
    }
    printf("r=%d, Ts=%d, N=%d, queue_size=%d, algorithm=%d\n", r, Ts, N, queue_size, algorithm);

    seeking_time = Ts * 3.0 / 25.0;
    time_for_sector = 60000.0 / (20.0 * r);

    printf("Seek time: %f\n", seeking_time);
    printf("sector time: %f\n", time_for_sector);
    make_requests();
    simulate();
    return 0;
}