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

#define maxconnect 5

typedef struct sockaddr_un un;


void msg(char **ms, int cond){
    printf("%s\n",ms);
    if(cond)
        exit(-1);
}


int main(int argc, int **argv) {

    int server_socket, client_socket, address_size;

    server_socket = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (server_socket<0){
        msg("Could not create socket", 1);
    }
    char *file = "Bruh_nigger";
    un server_addr;
    un client_addr;
    server_addr.sun_family = AF_LOCAL;
    strncpy(server_addr.sun_path,file,sizeof(server_addr.sun_path));
    server_addr.sun_path[sizeof(server_addr.sun_path)-1] = '\0';
    size_t size = SUN_LEN(&server_addr);
    int bind_socket = bind(server_socket, (struct sockaddr *) &server_addr, size);
    if(bind_socket<0)
        msg("Binding failed", 1);
    int listen_socket = listen(server_socket, maxconnect);
    if(listen_socket<0)
        msg("Could not listen", 1);
    
    while(1) {
        
    }

}
