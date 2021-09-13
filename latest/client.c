#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct sockaddr_in IN;
typedef struct sockaddr SA;

void msg(char *msg, bool cond)
{
    printf("%s\n", msg);
    if (cond)
        exit(-1);
}

void make_server(int PORT)
{

    int _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket < 0)
    {
        msg("SOCKET CREATION UNSUCCESSFUL", true);
    }
    int IN_size = sizeof(IN);
    IN server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int _connect_status = connect(_socket, (SA *)&server_addr, IN_size);
    if (_connect_status < 0)
    {
        msg("Connect failed", true);
    }
    printf("Gaylmao\n");
    for (int i = 0; i < 1; i++)
    {
        char buff[100];
        memset(buff, '\0', sizeof(buff));
        strncpy(buff, "/lib/libm.so.6#cos#2", sizeof(buff));
        write(_socket, buff, sizeof(buff));
        printf("Sent msg\n");
        // sleep(1);
        while (buff[0] != '#')
        {
            int _recv_status = recv(_socket, buff, sizeof(buff), 0);
            if (_recv_status <= 0)
            {
                printf("Not received");
                exit(-1);
            }
            else
            {
                printf("received\n");
            }
        }
    }
    close(_socket);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf(".\a.out [PORT] [THREAD_LIMIT_DISPATCHER not for client] [FILE_LIMIT not for client] [MEMORY_LIMIT not for client]\n");
        exit(-1);
    }
    make_server(atoi(argv[1]));
}
