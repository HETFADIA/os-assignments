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

int r = 0, Ts = 0, N = 0, type = 0;
int num_req = 0;
double seek_time = 0;
double sec_time = 0;
double sum_square = 0, sum_normal = 0, cnt_normal = 0;
double min_normal = 1e9;
double max_normal = -1e9;

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
    int plat, cyl, sec, num;
    struct node *next;
}(node);
node *new_node(int plat, int cyl, int sec, int num)
{
    node *temp = (node *)malloc(sizeof(node));
    temp->plat = plat;
    temp->cyl = cyl;
    temp->sec = sec;
    temp->num = num;
    temp->next = NULL;
    return temp;
}
node *head = NULL;
node *tail = NULL;
int queue_size = 0;
void append(int plat, int cyl, int sec, int num)
{
    node *newnode = new_node(plat, cyl, sec, num);
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
    while (temp->next != head)
    {
        temp = temp->next;
    }
    node *ret = temp->next;
    temp->next = ret->next;
    --queue_size;
    return ret;
}
void generate_queue()
{
    for (int i = 0; i < num_req; ++i)
    {
        append(randint(1, 4), randint(1, 25), randint(1, 20), randint(1, 20));
    }
}
double p=0;
void stats(double t)
{
    p+=t;
    sum_normal+=p;
    sum_square+=p*p;
    cnt_normal++;
    if(cnt_normal>100){
        return;
    }
    if(p<min_normal){
        min_normal=p;
    }
    if(p>max_normal){
        max_normal=p;
    }
    int a=sum_square/cnt_normal - (sum_normal/cnt_normal)*(sum_normal/cnt_normal);
    printf("sum sq is %lf sum normal is %lf p is %lf t is %lf\n",sum_square,sum_normal,p,t);
    printf("avg is %lf",sum_normal/cnt_normal);
    double stddev = sqrt(sum_square/cnt_normal - (sum_normal/cnt_normal)*(sum_normal/cnt_normal));
    // t = t + sum_normal;
    // if (min_normal == -1)
    // {
    //     min_normal = t;
    //     max_normal = t;
    // }
    // sum_normal += t;
    // sum_square += t * t;
    // cnt_normal++;
    // double stddev = sum_square / cnt_normal;
    // stddev -= pow(sum_normal / cnt_normal, 2);
    // stddev = pow(stddev, 0.5);
    // if (t < min_normal)
    // {
    //     min_normal = t;
    // }
    // if (t > max_normal)
    // {
    //     max_normal = t;
    // }
    // printf("Total Time: %lf\n", t);
    // printf("Minimum Time: %lf\n", min_normal);
    // printf("Maximum Time: %lf\n", max_normal);
    // printf("Average Time: %lf\n", sum_normal / cnt_normal);
    // printf("Stddev: %lf\n", stddev);
    // printf("Queue size: %d\n", queue_size);

}
int calc_sec(int diff)
{
    double time_req = diff * seek_time;
    double rate = (double)r / 60000.0;
    double revs = rate * time_req;
    double secs = revs * 20.0;
    return ceil(secs);
}
void Random_scheduling_policy()
{
    int curr_cyl = randint(1, 25);
    int curr_sec = 1;
    while (queue_size)
    {
        int ind = randrange(0, queue_size);
        node *ptr = pop(ind);
        int plat = ptr->plat;
        int cyl = ptr->cyl;
        int sec = ptr->sec;
        int num = ptr->num;
        free(ptr);
        int track_diff = abs(cyl - curr_cyl);
        int secs = calc_sec(track_diff);
        curr_sec = (curr_sec - 1 + secs) % 20 + 1;
        int sec_diff = ((sec - curr_sec + 20) % 20);
        double time = track_diff * seek_time + (sec_diff + num) * sec_time;
        curr_sec = (curr_sec - 1 + num) % 20 + 1;
        curr_cyl = cyl;
        stats(time);
    }
}

void FIFO()
{
    int curr_cyl = randint(1, 25);
    int curr_sec = randint(1,20);
    while (queue_size)
    {
        node *ptr = pop(0);
        int plat = ptr->plat;
        int cyl = ptr->cyl;
        int sec = ptr->sec;
        int num = ptr->num;
        free(ptr);
        int track_diff = abs(cyl - curr_cyl);
        int secs = calc_sec(track_diff);
        curr_sec = (curr_sec - 1 + secs) % 20 + 1;
        int sec_diff = ((sec - curr_sec + 20) % 20);
        double time = track_diff * seek_time + (sec_diff + num) * sec_time;
        curr_sec = (curr_sec - 1 + num) % 20 + 1;
        curr_cyl = cyl;
        stats(time);
    }
}
void SSTF()
{
    int curr_cyl = rand() % 25 + 1;
    int curr_sec = 1;
    while (queue_size > 0)
    {
        struct node *ptr = head;
        struct node *closest = head;
        while (ptr != NULL)
        {
            if (abs(ptr->cyl - curr_cyl) < abs(closest->cyl - curr_cyl))
            {
                closest = ptr;
            }
            ptr = ptr->next;
        }
        int plat = closest->plat;
        int cyl = closest->cyl;
        int sec = closest->sec;
        int num = closest->num;
        _remove(closest);
        int track_diff = abs(cyl - curr_cyl);
        int secs = calc_sec(track_diff);
        curr_sec = (curr_sec - 1 + secs) % 20 + 1;
        int sec_diff = (abs(sec - curr_sec + 20) % 20);
        double time = track_diff * seek_time + (sec_diff + num) * sec_time;
        curr_sec = (curr_sec - 1 + num) % 20 + 1;
        curr_cyl = cyl;
        stats(time);
    }
}
void SCAN()
{
    int curr_cyl = rand() % 25 + 1;
    int curr_sec = 1;
    bool direction = 0;
    if (curr_cyl > 13)
        direction = 1;
    while (queue_size > 0)
    {
        struct node *ptr = head;
        struct node *closest = NULL;
        if (direction == 1)
        {
            while (ptr != NULL)
            {
                if (ptr->cyl >= curr_cyl)
                {
                    closest = ptr;
                    break;
                }
                ptr = ptr->next;
            }
        }
        else
        {
            while (ptr != NULL)
            {
                if (ptr->cyl <= curr_cyl)
                {
                    closest = ptr;
                    break;
                }
                ptr = ptr->next;
            }
        }
        if (closest == NULL)
        {
            if (direction == 1)
            {
                int track_diff = abs(25 - curr_cyl);
                int secs = calc_sec(track_diff);
                curr_sec = (curr_sec - 1 + secs) % 20 + 1;
                double time = track_diff * seek_time;
                curr_cyl = 25;
                stats(time);
                direction = !direction;
                continue;
            }
            else
            {
                int track_diff = abs(0 - curr_cyl);
                int secs = calc_sec(track_diff);
                curr_sec = (curr_sec - 1 + secs) % 20 + 1;
                double time = track_diff * seek_time;
                curr_cyl = 25;
                stats(time);
                direction = !direction;
                continue;
            }
        }
        ptr = head;
        while (ptr != NULL)
        {
            if (direction == 1 && ptr->cyl >= curr_cyl)
            {
                if (ptr->cyl < closest->cyl)
                {
                    closest = ptr;
                }
            }
            if (direction == 0 && ptr->cyl <= curr_cyl)
            {
                if (ptr->cyl > closest->cyl)
                {
                    closest = ptr;
                }
            }
            ptr = ptr->next;
        }
        int plat = closest->plat;
        int cyl = closest->cyl;
        int sec = closest->sec;
        int num = closest->num;
        _remove(closest);
        int track_diff = abs(cyl - curr_cyl);
        int secs = calc_sec(track_diff);
        curr_sec = (curr_sec - 1 + secs) % 20 + 1;
        int sec_diff = (abs(sec - curr_sec + 20) % 20);
        double time = track_diff * seek_time + (sec_diff + num) * sec_time;
        curr_sec = (curr_sec - 1 + num) % 20 + 1;
        curr_cyl = cyl;
        stats(time);
    }
}

void CSCAN()
{
    int curr_cyl = rand() % 25 + 1;
    int curr_sec = 1;
    while (queue_size > 0)
    {
        struct node *ptr = head;
        struct node *closest = NULL;
        while (ptr != NULL)
        {
            if (ptr->cyl >= curr_cyl)
            {
                closest = ptr;
                break;
            }
            ptr = ptr->next;
        }
        if (closest == NULL)
        {
            int track_diff = abs(25 - curr_cyl);
            int secs = calc_sec(track_diff);
            curr_sec = (curr_sec - 1 + secs) % 20 + 1;
            double time = track_diff * seek_time;
            curr_cyl = 25;
            track_diff = abs(25 - 1);
            secs = calc_sec(track_diff);
            curr_sec = (curr_sec - 1 + secs) % 20 + 1;
            time += track_diff * seek_time;
            curr_cyl = 1;
            stats(time);
            continue;
        }
        ptr = head;
        while (ptr != NULL)
        {
            if (ptr->cyl >= curr_cyl)
            {
                if (ptr->cyl < closest->cyl)
                {
                    closest = ptr;
                }
            }
            ptr = ptr->next;
        }
        int plat = closest->plat;
        int cyl = closest->cyl;
        int sec = closest->sec;
        int num = closest->num;
        _remove(closest);
        int track_diff = abs(cyl - curr_cyl);
        int secs = calc_sec(track_diff);
        curr_sec = (curr_sec - 1 + secs) % 20 + 1;
        int sec_diff = (abs(sec - curr_sec + 20) % 20);
        double time = track_diff * seek_time + (sec_diff + num) * sec_time;
        curr_sec = (curr_sec - 1 + num) % 20 + 1;
        curr_cyl = cyl;
        stats(time);
    }
}
void simulate()
{
    if (type == 0)
    {
        Random_scheduling_policy();
    }
    else if (type == 1)
    {
        FIFO();
    }
    else if (type == 2)
    {
        SSTF();
    }
    else if (type == 3)
    {
        SCAN();
    }
    else if (type == 4)
    {
        CSCAN();
    }
    else
    {
        printf("Error: Type must an integer between [0,5]\n");
        exit(-1);
    }
}
int main(int argc, char **argv)
{
    if (argc < 5)
    {
        printf("Usage: %s <r> <Ts> <N> <queue_size> <type>\n", argv[0]);
        printf("Below is an example\n");
        printf("%s 7500 512 4 5000 1 \n ", argv[0]);
        return -1;
    }
    r = atoi(argv[1]);
    N = atoi(argv[2]);
    Ts = atoi(argv[3]);
    num_req = atoi(argv[4]);
    if (num_req < 0)
    {
        printf("Error: num_req must be non negative\n");
        exit(-1);
    }
    type = atoi(argv[5]);
    if (type < 0 || type > 4)
    {
        printf("Error: Type must an integer between [0,5]\n");
        exit(-1);
    }
    printf("r=%d, Ts=%d, N=%d, queue_size=%d, type=%d\n", r, Ts, N, queue_size, type);

    seek_time = Ts * 3.0 / 25.0;
    sec_time = r / 60000.0;
    sec_time = 1.0 / sec_time;
    sec_time /= 20.0;
    printf("Seek time: %f", seek_time);
    generate_queue();
    simulate();
    return 0;
}