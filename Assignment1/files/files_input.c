#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
int main()
{
    FILE * fp=fopen("input.txt","r");
    char * str[1000];
    char  str2[1000];
    int i=0;
    while(fgets(str2, 200, fp)) {
        int str2len=strlen(str2);
        str[i]=(char *)malloc(sizeof(char)*(str2len+1));

        for(int j=0;j<str2len;j++){
                str[i][j]=str2[j];

        }
        i++;
    }


    fclose(fp);
    printf("%d\n",i);
    for(int j=0;j<i;j++){
        printf("%s",str[j]);
    }

}
