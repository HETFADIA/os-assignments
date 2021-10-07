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

struct bakwaas{
    int a;
    char b;
};

typedef struct sockaddr_un un;
const char *file = "bruhbruh";

void msg(char *ms, int cond){
    printf("%s\n",ms);
    if(cond)
        exit(-1);
}

int make_socket(bool client){
    int server_socket, client_socket, address_size;

    server_socket = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (server_socket<0){
        msg("Could not create socket", 1);
    }
    if(!client && access(file, F_OK) != -1){
        msg("access", 0);
        if(unlink(file) < 0) msg("retard delete that file", true);
    }
    
    un server_addr;
    un client_addr;
    server_addr.sun_family = AF_LOCAL;
    strncpy(server_addr.sun_path,file,sizeof(server_addr.sun_path));
    server_addr.sun_path[sizeof(server_addr.sun_path)-1] = '\0';
    size_t size = SUN_LEN(&server_addr);

    if(client){
        int connect_client = connect(server_socket, (struct sockaddr *) &server_addr, size);
        if(connect_client < 0)
            msg("connection failed", 1);
    }
    else{
        int bind_socket = bind(server_socket, (struct sockaddr *) &server_addr, size);
        if(bind_socket<0)
            msg("Binding failed", 1);
    }
    return server_socket;
}

int main(int argc, char **argv) {

    int socket_info;
    if(argc >= 2){
        struct bakwaas ab;
        ab.a = atoi(argv[2]);
        ab.b = argv[3][0];
        socket_info = make_socket(1);
        if(write(socket_info, &ab, sizeof(ab)) > 0){
            char buff[5001];
            memset(buff, '\0', sizeof(buff));
            if(read(socket_info, buff, sizeof(buff)) > 0){
                msg(buff, 0);
            }
        }
        close(socket_info);
        return 0;
    }
    else{
        socket_info = make_socket(0);
    }

    int server_socket = socket_info;
    
    int listen_socket = listen(server_socket, maxconnect);
    if(listen_socket<0)
        msg("Could not listen", 1);
    
    while(1) {
        un abc;
        int len = sizeof(abc);
        printf("Wait....\n");
        int client_accept = accept(server_socket, (struct sockaddr *) &abc, &len);

        if(client_accept < 0){
            msg("client missed", 0);
        }
        // char buff[5001];
        // memset(buff, '\0', sizeof(buff));
        struct bakwaas xy;
        if(read(client_accept, &xy, sizeof(xy)) > 0){
            printf("%d\n", xy.a);
            char buff[5001];
            memset(buff, '\0', sizeof buff);
            for(int i = 0; i < xy.a; i++){
                buff[i] = xy.b;
            }
            write(client_accept, buff, sizeof(buff));
        }
        close(client_accept);
    }
}
