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
        int port = atoi(argv[1]);
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            printf("\nsocket could not be created\n");
            exit(-1);
        }
        struct sockaddr_in server_addr, client_addr;

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);

        int connected = connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
        if (connected < 0)
        {
            printf("Connect could not be made\n");
            exit(-1);
        }
        char **str = (char *[]){"/lib/x86_64-linux-gnu/libm.so.6#cos#2", "/lib/x86_64-linux-gnu/libm.so.6#sin#1", "/lib/x86_64-linux-gnu/libm.so.6#ceil#2.5", "/lib/x86_64-linux-gnu/libm.so.6#sqrt#10", "/lib/x86_64-linux-gnu/libm.so.6#log10#5", "/lib/x86_64-linux-gnu/libm.so.6#pow#2#5", "/lib/x86_64-linux-gnu/libm.so.6#hypot#3#4"};
        int strsize = 7;
        for (int i = 0; i < strsize; i++)
        {
            char *buff = (char *)malloc(sizeof(char) * 201);
            write(sock, str[i], strlen(str[i]));
            printf("Sent msg %s\n", str[i]);
            while (buff[0] != '#')
            {
                int _recv_status = recv(sock, buff, sizeof(buff), 0);
                if (_recv_status <= 0)
                {
                    printf("\nNot received\n");
                    exit(-1);
                }
                else
                {
                    printf("got the message\n");
                }
            }
        }
        close(sock);
    }
}
