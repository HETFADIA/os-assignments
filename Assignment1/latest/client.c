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

void msg(char *msg, bool ex){
    printf("%s\n", msg);
    if(ex) exit(-1);
}

void client(int PORT){

    int _socket = socket(AF_INET, SOCK_STREAM, 0);
    if(_socket < 0){
        msg("SOCKET CREATION UNSUCCESSFUL", true);
    }
    int IN_size = sizeof(IN);
    IN server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int _connect_status = connect(_socket, (SA *)&server_addr, IN_size);
    if(_connect_status < 0) {
        msg("Connect failed", true);
    }
    char **str = (char *[]){"/lib/x86_64-linux-gnu/libm.so.6#cos#2", "/lib/x86_64-linux-gnu/libm.so.6#sin#1", "/lib/x86_64-linux-gnu/libm.so.6#ceil#2.5", "/lib/x86_64-linux-gnu/libm.so.6#sqrt#10", "/lib/x86_64-linux-gnu/libm.so.6#log10#5", "/lib/x86_64-linux-gnu/libm.so.6#pow#2#5", "/lib/x86_64-linux-gnu/libm.so.6#hypot#3#4"};
    for(int i=0;i<7;i++){
      	char *buff = (char *)malloc(sizeof(char) * 201);
        write(_socket, str[i], strlen(str[i]));
        printf("Sent msg %s\n", str[i]);
        while(buff[0]!='#'){
            int _recv_status = recv(_socket, buff, sizeof(buff), 0);
            if(_recv_status<=0){
                printf("Not received");
                exit(-1);
            }
            else{
                printf("received\n");
            }
        }
    }
    close(_socket);
    
}

int main(int argc, char **argv){
    if(argc != 2){
        printf(".\a.out [PORT]n");
        exit(-1);
    }
    client(atoi(argv[1]));
}
