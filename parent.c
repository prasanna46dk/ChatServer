#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#define PORT 8080
#include <arpa/inet.h>

int server(int port, char *addr){
    int server_fd, new_socket, valread;
    
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";
      
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    //use address family ipv4
    address.sin_family = AF_INET;
    //set address to given address 
    address.sin_addr.s_addr = inet_addr(addr);
    //use port no 
    address.sin_port = htons( port);
      
    // attach socket to port no 
    if (bind(server_fd, (struct sockaddr *)&address,sizeof(address))<0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
        if (listen(server_fd, 3) < 0){
            perror("listen");
            exit(EXIT_FAILURE);
        }
    while (1){
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0){
            perror("accept");
            exit(EXIT_FAILURE);
        }
        while (1){
            valread = read( new_socket , buffer, 1024);
            printf("%s\n",buffer );
            send(new_socket , hello , strlen(hello) , 0 );
            printf("Hello message sent\n");
        }
    }
    return 0;
}

int main(int argc, char *argv[]){
    int ret = server(49152, "127.0.0.1");
}

