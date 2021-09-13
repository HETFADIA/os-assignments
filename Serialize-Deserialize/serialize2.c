#include <stdbool.h>
#include <stdio.h>
#include<stdlib.h>
#include<string.h>
void _serialize(char * library_name,char * function_name,char *arguments[],int arguments_len){
    int library_name_size=sizeof(library_name)/sizeof(library_name[0]);
    int function_name_size=sizeof(function_name)/sizeof(function_name[0]);
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
    serialize[serializelen++]='#';
    for(int i=0;i<arguments_len;++i){
        int temp=strlen(arguments[i]);
        for(int j=0;j<temp;j++){
            serialize[serializelen++]=arguments[i][j];
        }
        serialize[serializelen++]='#';
    }
    for(int i=0;i<serializelen;i++){
        printf("%c",serialize[i]);
    }
}
int main()
{
    
    char * a="include";
    char * b="function";
    char **arguments=(char *[]){"heya______","there"};

    int arguments_len=2;
    
    
    _serialize(a,b,arguments,arguments_len);
}
