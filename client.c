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

int validateBuff(char *buff){
    printf("Connected to server.\nPlease Enter user tio connect to:\n");
    scanf("%[^\n]",buff);
    char c;
    scanf("%c",&c);
    printf("%s\n",buff);
    if (buff[1] == '@')
       return OLDPEER;
    else 
       return NEWPEER;
}

//void TextUI(char *buff, char *DestUsername){
//    printf("Connected to Server 127.0.0.1.\nPlease Enter client name to connect to :-\n");
//    scanf("%s",DestUsername);
//    printf("Please Enter message:\n");
//    scanf("%[^\n]",buff);
//    destAddress = buff;
//    //char c;
//    //scanf("%c",&c);
//    printf("%s\n",buff);
//}

//char *CreateBuffer(char *SelfUserName, char *DestUsername, char *Message, char *bufferToBeSent) {
//      len_source = strlen(SelfUserName);
//      len_Dest = strlen(DestUsername);
//      len_msg = strlen(Message);
//      strcpy(bufferToBeSent, SelfUserName);
//      bufferToBeSent[len_source] = "%";
//      strcpy((bufferToBeSent+len_source+1), DestUsername);
//          
//} 
int client(int port)
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    Packet *packet;
    char *To;
    char *hello = "Hello from client";
    char c;
    char DestUsername[15] = {'\0'};
    char SelfUserName[15] = "one";
    char isClient[7] = {"client"};
    char message[976] = {'\0'};
    char msgDirection = '0';
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
    strcpy(buffer, "#@client#one#!#!#0#@#");
    send(sock , buffer, sizeof(buffer) , 0 );
    
    //printIntroMessage(buffer);
    //TextUI(buffer, DestUsername);
    while (1){
	//int message_to_whom = validateBuff(buffer);
        //printf("%s\n",buffer);
        printf("Enter destination username :\n");
        scanf("%[^\n]",DestUsername);
        scanf("%c",&c);
        printf("Enter your Mesage: \n");
        scanf("%[^\n]",message);
        scanf("%c",&c);
        strcpy(buffer,"#@#");
        //src address
        strcat(buffer,isClient);
        strcat(buffer,"#");
        strcat(buffer,"one#");
        //dest address
        strcat(buffer,DestUsername);
        strcat(buffer,"#");
        //client or server
        //message
        strcat(buffer,message);
        strcat(buffer,"#");
        //sending message
        strcat(buffer,"0#@#");
        
        send(sock , buffer, sizeof(buffer) , 0 );
        printf("message sent\n");
        if ((read (sock, buffer, sizeof(buffer))) > 0);
            printf("Received : %s",buffer);
    }
    return 0;
}

int main (){
    client(49152);
    return 0;
}
