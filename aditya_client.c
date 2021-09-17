#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct sockaddr_in sckadd_in;
typedef struct sockaddr sckadd;

void make_server(int port){

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0){
        printf("Socket could not be created\n");
        exit(-1);
    }
    int in_size = sizeof(sckadd_in);
    sckadd_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int connect_status = connect(server_socket, (sckadd *)&server_addr, in_size);
    if(connect_status < 0) {
        printf("Connect failed\n");
        exit(-1);
    }
    printf("New client-------------\n");
    char **str = (char *[]){"/lib/x86_64-linux-gnu/libm.so.6$cos$2", "/lib/x86_64-linux-gnu/libm.so.6$floor$2.8", "/lib/x86_64-linux-gnu/libm.so.6$log$50", "/lib/x86_64-linux-gnu/libm.so.6$sqrt$50", "/lib/x86_64-linux-gnu/libm.so.6$pow$3$4", "/lib/x86_64-linux-gnu/libm.so.6$remainder$13$3"};
    for(int k=0;k<12;k++){
        int i = k%6;
        char req[50];
        memset(req, '\0', sizeof(req));
        strncpy(req,str[i], sizeof(req));
        write(server_socket, req, sizeof(req));
        printf("Sent msg\n");
        while(!(req[0]=='@' && req[1]=='!')){
            int recv_status = recv(server_socket, req, sizeof(req), 0);
            if(recv_status<=0){
                printf("Could not receive message, exiting...");
                exit(-1);
            }
            else{
                printf("Server acknowledged request\n");
            }
        }
    }
    close(server_socket);
    
}

int main(int argc, char **argv){
    if(argc < 2){
        printf("Invalid arguments!\n");
        printf("./client <Port>");
        exit(-1);
    }
    make_server(atoi(argv[1]));
}
