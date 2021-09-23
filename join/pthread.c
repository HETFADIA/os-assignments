#include <stdio.h>
#include <pthread.h>
#include<stdlib.h>
void * fun(void * a){
    int *t=(int * )a;
    (*t)++;
    return NULL;
}
int main(void) {
    pthread_t t1,t2;
    int a=0;
    int * temp=&a;
    pthread_create(&t1,NULL,fun,temp);
    pthread_create(&t2,NULL,fun,temp);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("%d\n",a);
  
}
