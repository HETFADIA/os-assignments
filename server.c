#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#define PORT 8080

void* func(void *new_socket){
    int a = *((int*)new_socket);
    char buffer[1024] = {0};
    int valread;
    while((valread = recv( a , buffer, sizeof(buffer),0))>0){
            printf("%s\n",buffer);
            write(a,buffer, sizeof(buffer) );
        }
    return NULL;
}

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";
       
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
       
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
       
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 100) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    // if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
    //                    (socklen_t*)&addrlen))<0)
    // {
    //     perror("accept");
    //     exit(EXIT_FAILURE);
    // }
    while(1){
        printf("Waiting\n");
        int new_socket = accept(server_fd, (struct sockaddr *)&address, 
                        (socklen_t*)&addrlen);
        if(new_socket<0)
            exit(-1);
        pthread_t t;
        int *p_client = (int *)malloc(sizeof(int));
        *p_client = new_socket;
        pthread_create(&t, NULL, func, p_client);
        
    }
    // while((valread = recv( new_socket , buffer, 1024,0))>0){
    //     printf("%s\n",buffer);

    // }
    return 0;
}
