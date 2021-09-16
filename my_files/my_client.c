
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
    if (argc != 2)
    {
        if (argc == 1)
        {
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
            printf("\nsocket could not be created\n");
            exit(-1);
        }
        struct sockaddr_in s_adder;

        s_adder.sin_family = AF_INET;
        s_adder.sin_addr.s_addr = INADDR_ANY;
        int port = atoi(argv[1]);
        s_adder.sin_port = htons(port);

        bool connection_established = connect(sock, (struct sockaddr *)&s_adder, sizeof(struct sockaddr_in));
        if (connection_established < 0)
        {
            printf("Connect could not be made\n");
            exit(-1);
        }
        FILE * fp=fopen("input.txt","r");
        char * str[1000];
        char  str2[1000];
        int i1=0;
        while(fgets(str2, 200, fp)) {
            int str2len=strlen(str2);
            str[i1]=(char *)malloc(sizeof(char)*(str2len));

            for(int j=0;j<str2len;j++){
                    str[i1][j]=str2[j];

            }
            i1++;
        }


        fclose(fp);
        /*
        printf("%d\n",i1);
        for(int j=0;j<i1;j++){
            printf("%s",str[j]);
        }
        */
        int strsize = i1;
        char *read_recieve = (char *)malloc(sizeof(char) * 201);
        for (int i = 0; i < strsize; i++)
        {
            memset(read_recieve,'\0',sizeof(read_recieve));
            write(sock, str[i], strlen(str[i]));
            printf("Sent msg %s\n", str[i]);
            while (read_recieve[0] != '?')
            {
                int _recv_status = recv(sock, read_recieve, sizeof(read_recieve), 0);
                if (_recv_status <= 0)
                {
                    printf("\nmessage not received\n");
                    exit(-1);
                }
                else
                {
                    printf("\nreceived the message\n");
                }
            }
        }
        close(sock);
    }
}
