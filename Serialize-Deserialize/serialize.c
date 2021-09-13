#include <stdbool.h>
#include <stdio.h>
#include<stdlib.h>
#include<string.h>
void _serialize(char * library_name,char * function_name,int library_name_size,int function_name_size){

    char  serialize[5001];
    int serializelen=0;
    memset(serialize,'\0',sizeof(serialize));
    for(int i=0;i<library_name_size;i++){
        serialize[serializelen++]=library_name[i];
    }
    serialize[serializelen++]='#';
    for(int i=0;i<function_name_size;i++){
        serialize[serializelen++]=function_name[i];
    }
    printf("%s",serialize);
}
int main()
{
    
    char * a="include";
    char * b="function";
    _serialize(a,b,strlen(a),strlen(b));
}
