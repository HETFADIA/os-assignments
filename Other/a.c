#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int main(){
    char *s = malloc(5);
    char *b;
    s[0]='A';
    s[1]='M'; s[2]='A'; s[3]='N'; s[4]='\0';
    printf("%s\n",s);
    b=malloc((int)strlen(s));
    strcpy(b,s);
    free(s);
    printf("%s\n",b);
    return 0;
}