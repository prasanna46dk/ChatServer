#include <stdio.h> 
#include <string.h>       
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>       
#include <arpa/inet.h>    
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h>     //FD_SET, FD_ISSET, FD_ZERO macros 
    
#define OLDPEER 1
#define NEWPEER 0
#define STDIN 0

int client(int port, char *name)
{
    struct sockaddr_in address;
    int sock = 0, valread, length, activity, max_sd;
    struct sockaddr_in serv_addr;
    fd_set masterfd, readfds;
    char c, DestUsername[15] = {'\0'}, SelfUserName[15], isClient[7] = {"client"}, message[976] = {'\0'}, msgDirection = '0', buffer[1024] = {'\0'}, readbuffer[1024] = {'\0'};
    strcpy(SelfUserName,name);
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
      
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
	close(sock);
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
	close(sock);
        return -1;
    }
    strcpy(buffer, "#@client#");
    length = strlen(buffer);
    strcpy(&buffer[length],name);
    length = strlen(buffer);
    strcpy(&buffer[length],"#!#!#0#@#");
    printf("buffer: %s\n",buffer);
    send(sock , buffer, sizeof(buffer) , 0 );
    max_sd = sock;
    FD_ZERO(&readfds);
    FD_ZERO(&masterfd);
    FD_SET(sock,&masterfd);
    FD_SET(STDIN,&masterfd);
    while (1){
        readfds = masterfd;
        printf("Enter destination username :\n");
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
        if ((activity < 0) && (errno!=EINTR)) 
            printf("select error");
        if (FD_ISSET(sock, &readfds)) {
            valread = read(sock, buffer, 1024);
            if ((valread == 0)) {
                printf("\nConnection closed from server\n");
                close(sock);
                break;
            } else {
                printf("\nReceived : %s\n",buffer);
                printf("Enter destination username :\n");
            }
        } 
        scanf("%[^\n]",DestUsername);
        scanf("%c",&c);
        printf("Enter your Mesage: \n");
        scanf("%[^\n]",message);
        scanf("%c",&c);
        strcpy(buffer,"#@#");
        //client or server 
        strcat(buffer,isClient);
        strcat(buffer,"#");
        //name of src client
        strcat(buffer,name);
        strcat(buffer,"#");
        //name of dest client
        strcat(buffer,DestUsername);
        strcat(buffer,"#");
        //message to send
        strcat(buffer,message);
        strcat(buffer,"#");
        //direction of message
        strcat(buffer,"0#@#");
         
        write(sock , buffer, sizeof(buffer));
        printf("message sent\n");
    }
    return 0;
}

int main (int argc, char *argv[]){
    client(atoi(argv[1]),argv[2]);
    return 0;
}
