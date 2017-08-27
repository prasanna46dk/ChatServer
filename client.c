#include <stdio.h> 
#include <string.h>       //strlen 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>       //close 
#include <arpa/inet.h>    //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h>     //FD_SET, FD_ISSET, FD_ZERO macros 
    
#define OLDPEER 1
#define NEWPEER 0

typedef struct Packet{
    unsigned int PacketNumber;
    char *From;
    char *To;
    unsigned int LengthofMessage;
    char *Message;      
}Packet;

Packet *ParsetoPacket(char *buff, int packetno, char *from){
    Packet *packet = (Packet*)malloc(sizeof (Packet));
    char *token = strtok(buff,"#");
    int i = 0, length = 0;
    while (token){
        if ( i == 0){
            packet -> To = token; 
        } else if (i == 1){
            packet -> Message = token;
            packet -> LengthofMessage = strlen(token);
            break;
        }
        i++;
    }
    packet -> PacketNumber = packetno;
    packet -> From = from;
    //strcpy(packet -> From, from); 
    return packet;
}

int validateBuff(char buff){
    if (buff == '@')
       return OLDPEER;
    else 
       return NEWPEER;
}

char *printIntroMessage(char *buff){
    char *destAddress;
    char *hello;
    hello = "Connected to Server 127.0.0.1.\nPlease Enter address(a) and port number(b) in a format a:b :- \n";
    printf("%s",hello);
    //char buff[100];
    scanf("%[^\n]",buff);
    destAddress = buff;
    char c;
    scanf("%c",&c);
    printf("%s\n",buff);
}

int client(int port)
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    Packet *packet;
    char *To;
    char *hello = "Hello from client";
    char buffer[1024] = {'\0'};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
      
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
	close(sock);
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
	close(sock);
        return -1;
    }
    
    printIntroMessage(buffer);
    while (1){
            scanf("%[^\n]",buffer);
            char c;
            scanf("%c",&c);
            printf("%s\n",buffer);
	int message_to_whom = validateBuff(buffer[1]);
	if (message_to_whom == OLDPEER) {
	    packet -> Message = buffer;
            packet -> LengthofMessage = strlen(buffer);
        }
	else 
	    packet = ParsetoPacket(buffer,0,"127.0.0.1:49152");
        send(sock , packet, sizeof(packet) , 0 );
        printf("Hello message sent\n");
        hello = "Please enter your message:\n";
        //valread = read( sock , buffer, 1024);
        //printf("%s\n",buffer );
    }
    return 0;
}

int main (){
    client(49152);
    return 0;
}
