#include <stdbool.h>
#include <stdio.h>
#include<stdlib.h>
#include<string.h>
char  serialize[5001];
void _serialize(char * library_name,char * function_name,char *arguments[],int arguments_len){
    
    
    int serializelen=0;
    memset(serialize,'\0',sizeof(serialize));
    
    for(int i=0;library_name[i]!='\0';i++){
        serialize[serializelen++]=library_name[i];
    }
    serialize[serializelen++]='#';
    for(int i=0;function_name[i]!='\0';i++){
        serialize[serializelen++]=function_name[i];
    }
    serialize[serializelen++]='#';
    for(int i=0;i<arguments_len;++i){
        for(int j=0;arguments[i][j]!='\0';j++){
            serialize[serializelen++]=arguments[i][j];
        }
        serialize[serializelen++]='#';
    }
    for(int i=0;i<serializelen;i++){
        printf("%c",serialize[i]);
    }
    printf("\n");
}
int main()
{
    
    char * a="include";
    char * b="function";
    char **arguments=(char *[]){"heya______","there"};
    int arguments_len=2;
    _serialize(a,b,arguments,arguments_len);
    printf("%d",strlen(serialize));   
}
