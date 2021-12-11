#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

int RPM, AVG_SEEK_T, SEC_SZ, TOT_REQUESTS, SCHEDULING_TYPE, TR_POS = 12, SEC_POS = 0;
double TT_T, SS_T, RPMS;

struct quad_{
    int platter, cylinder, sector, num;
};
typedef struct quad_ quad_;
struct node{
    quad_ data;
    struct node *next;
};
typedef struct node node;

int ll_size = 0;
node *front = NULL;
node *back = NULL;

node *create_node(quad_ x){
    node *temp = (node *)malloc(sizeof(struct node));
    temp->data = x;
    temp->next = NULL;
    return temp;
}

void insert(quad_ x){
    node *temp = create_node(x);
    //in empty queue we need to define front too
    if(back == NULL)
        front = temp;
    else
        back->next = temp;
    back = temp;
    ll_size += 1;
}

void kill_node(node *del){
    if(del == NULL) return;
    node *temp = front;
    node *prev = NULL;
    while(temp != NULL){
        if(temp == del){
            if(prev != NULL){
                prev->next = temp->next;
            }
            else{
                front = temp->next;
            }
            if(temp->next == NULL){
                back = prev;
            }
            temp->next = NULL;
            free(temp);
            break;
        }
        prev = temp;
        temp = temp->next;
    }
    ll_size -= 1;
}

double SUM_T = 0.0, SUM_S_T = 0.0, MIN_T = 10000000.0, MAX_T = 0.0;
int CNT = 0;
double resp_time = 0.0;
void calc_stats(double adj_time){
    // adj_time = adj_time + SUM_T;
    resp_time = resp_time + adj_time;
    SUM_T += resp_time;
    SUM_S_T += resp_time * resp_time;
    if(MIN_T > resp_time || CNT == 0){
        MIN_T = resp_time;
    }
    if(MAX_T < resp_time || CNT == 0){
        MAX_T = resp_time;
    }
    CNT += 1;
    printf("REQUEST %d ->\n", CNT);
    printf("AVG. %lf\n", SUM_T / CNT);
    printf("MAX. %lf\n", MAX_T);
    printf("MIN. %lf\n", MIN_T);
    printf("STD. DEV. %lf\n", sqrt(SUM_S_T/CNT - (SUM_T/CNT) * (SUM_T/CNT)));
    printf("------------------------------------\n");
}

void create_LL_random_req(){
    for(int i = 0; i < TOT_REQUESTS; i++){
        quad_ temp;
        temp.platter = rand() % 4;
        temp.cylinder = rand() % 25;
        temp.sector = rand() % 20;
        temp.num = rand() % 20;
        insert(temp);
    }
}

void RANDOM_S(){
    node *del;
    while(1){
        del = front;
        int pos = rand() % ll_size;
        while(pos){
            pos -= 1;
            del = del->next;
        }
        quad_ temp = del->data;
        int cyl_diff = abs(TR_POS - temp.platter);
        int curr_sec = ((int)(SEC_POS + ceil(RPMS * TT_T * cyl_diff * 20.0))) % 20;
        double tot_t = temp.num * SS_T + cyl_diff * TT_T;
        if(temp.sector < curr_sec){
            tot_t += (20.0 - curr_sec + temp.sector) * SS_T;
        }
        else{
            tot_t += (temp.sector - curr_sec) * SS_T;
        }
        SEC_POS = (temp.sector + temp.num) % 20;
        TR_POS = temp.cylinder;
        kill_node(del);
        calc_stats(tot_t);
        if(ll_size <= 0) break;
    }
}

void FIFO_S(){
    node *del;
    while(1){
        del = front;
        quad_ temp = del->data;
        int cyl_diff = abs(TR_POS - temp.platter);
        int curr_sec = ((int)(SEC_POS + ceil(RPMS * TT_T * cyl_diff * 20.0))) % 20;
        double tot_t = temp.num * SS_T + cyl_diff * TT_T;
        if(temp.sector < curr_sec){
            tot_t += (20.0 - curr_sec + temp.sector) * SS_T;
        }
        else{
            tot_t += (temp.sector - curr_sec) * SS_T;
        }
        SEC_POS = (temp.sector + temp.num) % 20;
        TR_POS = temp.cylinder;
        kill_node(del);
        calc_stats(tot_t);
        if(ll_size <= 0) break;
    }
}

void SSTF_S(){
    node *del, *temp;
    while(1){
        del = front;
        temp = front;
        int diff = 25;
        while(temp){
            quad_ val = temp->data;
            if((int)abs(val.cylinder - TR_POS) < diff){
                diff = (int)abs(val.cylinder - TR_POS);
                del = temp;
            }
            temp = temp->next;
        }
        quad_ temp = del->data;
        printf("%d trust %d", temp.cylinder, TR_POS);
        int cyl_diff = abs(TR_POS - temp.platter);
        int curr_sec = ((int)(SEC_POS + ceil(RPMS * TT_T * cyl_diff * 20.0))) % 20;
        double tot_t = temp.num * SS_T + cyl_diff * TT_T;
        if(temp.sector < curr_sec){
            tot_t += (20.0 - curr_sec + temp.sector) * SS_T;
        }
        else{
            tot_t += (temp.sector - curr_sec) * SS_T;
        }
        SEC_POS = (temp.sector + temp.num) % 20;
        TR_POS = temp.cylinder;
        kill_node(del);
        calc_stats(tot_t);
        if(ll_size <= 0) break;
    }
}
void SCAN_S(){
    node *del, *temp;
    int direction = rand() % 2; // 0 -> left, 1 -> right
    while(1){
        // go in one direction - pick one request if in that direction or go left all the way, start again
        temp = front, del = NULL;
        int diff_val = 25;
        while(temp){
            quad_ val = temp->data;
            if(direction == 0){
                if(val.cylinder <= TR_POS && TR_POS - val.cylinder < diff_val){
                    del = temp;
                    diff_val = TR_POS - val.cylinder;
                }
            }
            else{
                if(val.cylinder >= TR_POS && val.cylinder - TR_POS < diff_val){
                    del = temp;
                    diff_val = val.cylinder - TR_POS;
                }
            }
            temp = temp->next;
        }
        if(del == NULL){
            //reverse direction if nothing found in the previous direction moving forward
            direction = 1 - direction;
            continue;
        }
        quad_ temp = del->data;
        int cyl_diff = abs(TR_POS - temp.platter);
        int curr_sec = ((int)(SEC_POS + ceil(RPMS * TT_T * cyl_diff * 20.0))) % 20;
        double tot_t = temp.num * SS_T + cyl_diff * TT_T;
        if(temp.sector < curr_sec){
            tot_t += (20.0 - curr_sec + temp.sector) * SS_T;
        }
        else{
            tot_t += (temp.sector - curr_sec) * SS_T;
        }
        SEC_POS = (temp.sector + temp.num) % 20;
        TR_POS = temp.cylinder;
        kill_node(del);
        calc_stats(tot_t);
        if(ll_size <= 0) break;
    }
}

int PREV_T = -1.0;
void CSCAN_S(){
    node *del, *temp;
    int direction = rand() % 2; // 0 -> left, 1 -> right
    while(1){
        // go in one direction - pick one request if in that direction or go left all the way, start again
        temp = front, del = NULL;
        int diff_val = 25;
        while(temp){
            quad_ val = temp->data;
            if(direction == 0){
                if(val.cylinder <= TR_POS && TR_POS - val.cylinder < diff_val){
                    del = temp;
                    diff_val = TR_POS - val.cylinder;
                }
            }
            else{
                if(val.cylinder >= TR_POS && val.cylinder - TR_POS < diff_val){
                    del = temp;
                    diff_val = val.cylinder - TR_POS;
                }
            }
            temp = temp->next;
        }
        if(del == NULL){
            //reset position if nothing found in the given direction direction
            //when we reset, we add time for it and the new position is recognized
            if(direction == 0){
                double reset_time = (24 - TR_POS) * TT_T;
                SEC_POS = ((int)(SEC_POS + ceil(RPMS * reset_time * 20.0))) % 20;
                PREV_T = reset_time;
                TR_POS = 24;
            }
            else{
                double reset_time = (TR_POS - 0) * TT_T;
                SEC_POS = ((int)(SEC_POS + ceil(RPMS * reset_time * 20.0))) % 20;
                PREV_T = reset_time;
                TR_POS = 0;
            }
            continue;
        }

        quad_ temp = del->data;
        int cyl_diff = abs(TR_POS - temp.platter);
        int curr_sec = ((int)(SEC_POS + ceil(RPMS * TT_T * cyl_diff * 20.0))) % 20;
        double tot_t = temp.num * SS_T + cyl_diff * TT_T;
        if(temp.sector < curr_sec){
            tot_t += (20.0 - curr_sec + temp.sector) * SS_T;
        }
        else{
            tot_t += (temp.sector - curr_sec) * SS_T;
        }
        SEC_POS = (temp.sector + temp.num) % 20;
        TR_POS = temp.cylinder;
        kill_node(del);
        if(PREV_T != -1.0){
            tot_t = tot_t + PREV_T;
            PREV_T = -1.0;
        }
        calc_stats(tot_t);
        if(ll_size <= 0) break;
    }
}

int main(int argc, char **argv){
    if(argc != 6){
        printf("Insufficient number of CLI(s)\n");
        exit(-1);
    }
    RPM = atoi(argv[1]);
    AVG_SEEK_T = atoi(argv[2]);
    SEC_SZ = atoi(argv[3]);
    TOT_REQUESTS = atoi(argv[4]);
    SCHEDULING_TYPE = atoi(argv[5]);
    create_LL_random_req();

    TT_T = (AVG_SEEK_T * 3.0) / 25.0; // time to move between consecutive tracks
    SS_T = 3000.0 / RPM; // time to move between consecutive sector
    RPMS = RPM / 60000.0;

    switch(SCHEDULING_TYPE){
        case 1:
        RANDOM_S();
        break;
        case 2:
        FIFO_S();
        break;
        case 3:
        SSTF_S();
        break;
        case 4:
        SCAN_S();
        break;
        case 5:
        CSCAN_S();
        break;
        default:
        RANDOM_S();
    }
    printf("Simulation Complete\n");
}