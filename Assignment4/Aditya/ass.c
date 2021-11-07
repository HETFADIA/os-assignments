#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>

int r, Ts, N, queue_size = 0;
int num_req = 0;
int type = 1;
double seek_time = 0;
double sec_time = 0;
double sum_square = 0;
double sum_normal = 0;
double cnt_normal = 0;
double min_normal = -1;
double max_normal = -1;

int cl(double val){
    if(val==(int)val)
        return val;
    return (int)val+1;
}

struct node{
    int plat;
    int cyl;
    int sec;
    int num;
    struct node *next;
};

struct node *head = NULL;
struct node *tail = NULL; 

void enqueue(int p, int c, int s, int nu){
    struct node *n = (struct node *)malloc(sizeof(struct node));
    n->plat = p;
    n->cyl = c;
    n->sec = s;
    n->num = nu;
    if(tail == NULL){
        head = n;
        tail = n;
    }
    else{
        tail->next = n;
        tail = n;
    }
    queue_size++;
}

void remove_node(struct node *r){
    if(head == NULL){
        return;
    }
    if(r==head){
        head = head->next;
        if(head == NULL)
            tail = NULL;
        free(r);
        queue_size--;
        return;
    }
    struct node *curr = head->next;
    struct node *prev = head;
    while(curr!=r){
        curr = curr->next;
        prev = prev->next;
    }
    prev->next = curr->next;
    if(tail == curr)
        tail = prev;
    free(curr);
    queue_size--;
}

void stats(double t){
    t = t+sum_normal;
    if(min_normal==-1){
        min_normal = t;
        max_normal = t;
    }
    sum_normal = t;
    sum_square += t*t;
    cnt_normal++;
    double stddev = sum_square/cnt_normal;
    stddev -= pow(sum_normal/cnt_normal, 2);
    stddev = pow(stddev, 0.5);
    if(t<min_normal)
        min_normal = t;
    if(t>max_normal)
        max_normal = t;
    printf("Time: %lf\n", t);
    printf("Min: %lf\n", min_normal);
    printf("Max: %lf\n", max_normal);
    printf("Avg: %lf\n", sum_normal/cnt_normal);
    printf("Stddev: %lf\n", stddev);
    printf("Queue size: %d\n", queue_size);
    printf("------------------------\n");
}

void generate_queue(){
    for(int i = 0; i<num_req; i++){
        int p = rand()%4 + 1;
        int c = rand()%25 + 1;
        int s = rand()%20 + 1;
        int nu = rand()%20 + 1;
        enqueue(p, c, s, nu);
    }
}

double calc_sec(int diff){
    double time_req = diff*seek_time;
    double rate = (double)r/60000;
    double revs = rate*time_req;
    double secs = revs*20;
    return cl(secs);
}

void random(){
    int curr_cyl = rand()%25 + 1;
    int curr_sec = rand()%20 + 1;
    while(queue_size>0){
        int ind = rand()%queue_size;
        struct node *ptr = head;
        for(int i=0;i<ind;i++){
            ptr = ptr->next;
        }
        int plat = ptr->plat;
        int cyl = ptr->cyl;
        int sec = ptr->sec;
        int num = ptr->num;
        remove_node(ptr);
        int track_diff = abs(cyl-curr_cyl);
        int secs = calc_sec(track_diff);
        curr_sec = (curr_sec-1+secs)%20 + 1;
        int sec_diff = (abs(sec-curr_sec+20)%20);
        double time = track_diff*seek_time + (sec_diff+num)*sec_time;
        curr_sec = (curr_sec-1+num)%20 + 1;
        curr_cyl = cyl;
        stats(time);
    }
}

void fifo(){
    int curr_cyl = rand()%25 + 1;
    int curr_sec = rand()%20 + 1;
    while(queue_size>0){
        int plat = head->plat;
        int cyl = head->cyl;
        int sec = head->sec;
        int num = head->num;
        remove_node(head);
        int track_diff = abs(cyl-curr_cyl);
        int secs = calc_sec(track_diff);
        curr_sec = (curr_sec-1+secs)%20 + 1;
        int sec_diff = (abs(sec-curr_sec+20)%20);
        double time = track_diff*seek_time + (sec_diff+num)*sec_time;
        curr_sec = (curr_sec-1+num)%20 + 1;
        curr_cyl = cyl;
        stats(time);
    }
}

void sstf(){
    int curr_cyl = rand()%25 + 1;
    int curr_sec = rand()%20 + 1;
    while(queue_size>0){
        struct node *ptr = head;
        struct node *closest = head;
        while(ptr!=NULL){
            if(abs(ptr->cyl-curr_cyl)<abs(closest->cyl-curr_cyl)){
                closest = ptr;
            }
            ptr = ptr->next;
        }
        int plat = closest->plat;
        int cyl = closest->cyl;
        int sec = closest->sec;
        int num = closest->num;
        remove_node(closest);
        int track_diff = abs(cyl-curr_cyl);
        int secs = calc_sec(track_diff);
        curr_sec = (curr_sec-1+secs)%20 + 1;
        int sec_diff = (abs(sec-curr_sec+20)%20);
        double time = track_diff*seek_time + (sec_diff+num)*sec_time;
        curr_sec = (curr_sec-1+num)%20 + 1;
        curr_cyl = cyl;
        stats(time);
    }
}

void scan(){
    int curr_cyl = rand()%25 + 1;
    int curr_sec = rand()%20 + 1;
    bool direction = 0;
    if(curr_cyl>13)
        direction = 1;
    while(queue_size>0){
        struct node *ptr = head;
        struct node *closest = NULL;
        if(direction==1){
            while(ptr!=NULL){
                if(ptr->cyl>=curr_cyl){
                    closest = ptr;
                    break;
                }
                ptr = ptr->next;
            }
        }
        else{
           while(ptr!=NULL){
                if(ptr->cyl<=curr_cyl){
                    closest = ptr;
                    break;
                }
                ptr = ptr->next;
            } 
        }
        if(closest==NULL){
            if(direction==1){
                int track_diff = abs(25-curr_cyl);
                int secs = calc_sec(track_diff);
                curr_sec = (curr_sec-1+secs)%20 + 1;
                double time = track_diff*seek_time;
                curr_cyl = 25;
                stats(time);
                direction = !direction;
                continue;
            }
            else{
                int track_diff = abs(0-curr_cyl);
                int secs = calc_sec(track_diff);
                curr_sec = (curr_sec-1+secs)%20 + 1;
                double time = track_diff*seek_time;
                curr_cyl = 25;
                stats(time);
                direction = !direction;
                continue;
            }
        }
        ptr = head;
        while(ptr!=NULL){
            if(direction==1 && ptr->cyl>=curr_cyl){
                if(ptr->cyl<closest->cyl){
                    closest = ptr;
                }
            }
            if(direction==0 && ptr->cyl<=curr_cyl){
                if(ptr->cyl>closest->cyl){
                    closest = ptr;
                }
            }
            ptr = ptr->next;
        }
        int plat = closest->plat;
        int cyl = closest->cyl;
        int sec = closest->sec;
        int num = closest->num;
        remove_node(closest);
        int track_diff = abs(cyl-curr_cyl);
        int secs = calc_sec(track_diff);
        curr_sec = (curr_sec-1+secs)%20 + 1;
        int sec_diff = (abs(sec-curr_sec+20)%20);
        double time = track_diff*seek_time + (sec_diff+num)*sec_time;
        curr_sec = (curr_sec-1+num)%20 + 1;
        curr_cyl = cyl;
        stats(time);
    }
}

void cscan(){
    int curr_cyl = rand()%25 + 1;
    int curr_sec = rand()%20 + 1;
    while(queue_size>0){
        struct node *ptr = head;
        struct node *closest = NULL;
        while(ptr!=NULL){
            if(ptr->cyl>=curr_cyl){
                closest = ptr;
                break;
            }
            ptr = ptr->next;
        }
        if(closest==NULL){
            int track_diff = abs(25-curr_cyl);
            int secs = calc_sec(track_diff);
            curr_sec = (curr_sec-1+secs)%20 + 1;
            double time = track_diff*seek_time;
            curr_cyl = 25;
            track_diff = abs(25-1);
            secs = calc_sec(track_diff);
            curr_sec = (curr_sec-1+secs)%20 + 1;
            time += track_diff*seek_time;
            curr_cyl = 1;
            stats(time);
            continue;
        }
        ptr = head;
        while(ptr!=NULL){
            if(ptr->cyl>=curr_cyl){
                if(ptr->cyl<closest->cyl){
                    closest = ptr;
                }
            }
            ptr = ptr->next;
        }
        int plat = closest->plat;
        int cyl = closest->cyl;
        int sec = closest->sec;
        int num = closest->num;
        remove_node(closest);
        int track_diff = abs(cyl-curr_cyl);
        int secs = calc_sec(track_diff);
        curr_sec = (curr_sec-1+secs)%20 + 1;
        int sec_diff = (abs(sec-curr_sec+20)%20);
        double time = track_diff*seek_time + (sec_diff+num)*sec_time;
        curr_sec = (curr_sec-1+num)%20 + 1;
        curr_cyl = cyl;
        stats(time);
    }
}

void process(){
    if(type==1)
        random();
    else if(type==2)
        fifo();
    else if(type==3)
        sstf();
    else if(type==4)
        scan();
    else if(type==5)
        cscan();
    else{
        printf("Invalid type\n");
        exit(0);
    }
}

int main(int argc, char **argv){
    r = atoi(argv[1]);
    Ts = atoi(argv[2]);
    N = atoi(argv[3]);
    num_req = atoi(argv[4]);
    type = atoi(argv[5]);
    seek_time = Ts*3.0/25.0;
    sec_time = r/60000.0;
    sec_time = 1.0/sec_time;
    sec_time /= 20.0;
    printf("Seek time: %f", seek_time);
    generate_queue();
    process();
    printf("Program ended!\n");
}