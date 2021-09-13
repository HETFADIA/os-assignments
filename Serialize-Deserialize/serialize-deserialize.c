#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char serialize[5001];
char deserialize[501][501];
int serializelen = 0;
int deserializelen = 0;
void _serialize(char *library_name, char *function_name, char *arguments[], int arguments_len)
{

    serializelen = 0;
    memset(serialize, '\0', sizeof(serialize));

    for (int i = 0; library_name[i] != '\0'; i++)
    {
        serialize[serializelen++] = library_name[i];
    }
    serialize[serializelen++] = '#';
    for (int i = 0; function_name[i] != '\0'; i++)
    {
        serialize[serializelen++] = function_name[i];
    }
    serialize[serializelen++] = '#';
    for (int i = 0; i < arguments_len; ++i)
    {
        for (int j = 0; arguments[i][j] != '\0'; j++)
        {
            serialize[serializelen++] = arguments[i][j];
        }
        if (i != arguments_len-1)
        {

            serialize[serializelen++] = '#';
        }
    }
    for (int i = 0; i < serializelen; i++)
    {
        printf("%c", serialize[i]);
    }
    printf("\n");
}
void _deserialize()
{
    deserializelen = 0;
    int counter = 0;
    for (int i = 0; serialize[i] != '\0'; i++)
    {
        if (serialize[i] == '#')
        {
            deserializelen++;
            counter=0;
            continue;
        }
        deserialize[deserializelen][counter++]=serialize[i];
    }
    deserializelen++;
}
int main()
{
    char *a = "include";
    char *b = "function";
    char **arguments = (char *[]){"heya______", "there"};
    int arguments_len = 2;
    _serialize(a, b, arguments, arguments_len);
    printf("%d\n", strlen(serialize));
    _deserialize();
    for(int i=0;i<deserializelen;i++){
        printf("%s\n",deserialize[i]);
    }
}
