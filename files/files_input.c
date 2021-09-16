#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
int main()
{
    FILE * fp=fopen("input.txt","r");
    char * arr[1000];
    char  arr2[1000];
    int i=0;
    while(fgets(arr2, 200, fp)) {
        int arr2len=strlen(arr2);
        arr[i]=(char *)malloc(sizeof(char)*(arr2len+1));

        for(int j=0;j<arr2len;j++){
                arr[i][j]=arr2[j];

        }
        i++;
    }


    fclose(fp);
    printf("%d\n",i);
    for(int j=0;j<i;j++){
        printf("%s",arr[j]);
    }

}
