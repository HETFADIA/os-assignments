#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
char *queue[100];
int start = 0;
int end = 0;
int qsize = 100;
int currsize=0;
bool enqueue(char *s)
{
    if(currsize>=qsize){
        return 0;
    }
    int slen = strlen(s);
    queue[end] = (char *)malloc(sizeof(char)*(slen+1));
    for(int i=0;i<=slen;i++){
        if (i < slen)
        {
            queue[end][i] = s[i];
        }
        else
        {
            queue[end][i] = '\0';
        }
    }
    end=(end+1)%qsize;
    currsize++;
    return 1;
}
char * deque(){
    if(currsize==0){
        return NULL;
    }
    currsize--;
    return queue[(start++)%qsize];
}
int main()
{
    enqueue("hey");
    enqueue("hi");
    char * s=deque();
    printf("%s\n",s);
    s=deque();
    printf("%s\n",s);
    for(int i=0;i<1001;i++){
        enqueue("abcdefghijklmn");
        char * s= deque();
        printf("%s\n",s);
    }
    for(int i=0;i<100;i++){
        enqueue("a");
    }
    for (int i = 0;i<100 ; i=(i+1))
    {
        printf("%d %s\n", i, queue[i]);
        
    }
}
