#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    //two arguments should be there
    //how to run
    /*
    gcc myclient.c
    ./a.out 7000

    */
    if (argc != 2)
    {

        if (argc == 1)
        {
            // only one arguments
            printf("Too  few arguments");
        }
        else
        {
            printf("Too  many arguments");
        }
        printf("\n");
        printf("\nWrite ./a.out then [port] of the server\n");
    }

    else
    {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            //socket could not be created so we return;
            printf("\nsocket could not be created\n");
            exit(-1);
        }
        struct sockaddr_in s_adder;

        s_adder.sin_family = AF_INET;
        s_adder.sin_addr.s_addr = INADDR_ANY;
        int port = atoi(argv[1]);
        s_adder.sin_port = htons(port);

        int connection_established = connect(sock, (struct sockaddr *)&s_adder, sizeof(struct sockaddr_in));
        if (connection_established < 0)
        {
            printf("Connection could not be made\n");
            exit(-1);
        }
        //taking the input from the input.txt as read file
        //the input consists of the requests that will be given to the server
        FILE *fp = fopen("input.txt", "r");
        //str[i] contains the ith request
        char *str[1000];
        char str2[1000];
        int i1 = 0;
        while (fgets(str2, 200, fp))
        {
            int str2len = strlen(str2);
            str[i1] = (char *)malloc(sizeof(char) * (str2len + 1));
            //copying the i1th line in str[i1]
            for (int j = 0; j <= str2len; j++)
            {
                if (j < str2len)
                {

                    str[i1][j] = str2[j];
                }
                else
                {
                    //the last char shoudl be \0 for string
                    str[i1][j] = '\0';
                }
            }

            i1++;
        }
        //closing the file
        fclose(fp);
        /*
        */
        printf("%d\n", i1);
        for (int j = 0; j < i1; j++)
        {
            printf("%s", str[j]);
        }
        int strsize = i1;
        char *read_recieve = (char *)malloc(sizeof(char) * 201);
        for (int j = 0; j < 1; j++)
            for (int i = 0; i < strsize; i++)
            {
                memset(read_recieve, '\0', sizeof(read_recieve));
                int strilen = strlen(str[i]);
                write(sock, str[i], strilen);
                //printf("Sent msg %s\n", str[i]);
                while (read_recieve[0] != '?')
                {

                    if (recv(sock, read_recieve, sizeof(read_recieve), 0) <= 0)
                    {
                        //we recieved the messaage from teh server
                        printf("\nmessage not received from the server\n");
                        exit(-1);
                    }
                    else
                    {
                        //receive the mesasge from the server
                        printf("\nreceived message from the server\n");
                    }
                }
            }
            //close the socket
        close(sock);
    }
}
/*
    gcc myclient.c
    ./a.out 7000

    */
