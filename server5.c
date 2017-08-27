//Example code: A simple server side code, which echos back the received message.
//Handle multiple socket connections with select and fd_set on Linux 
#include <stdio.h> 
#include <string.h>   //strlen 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>   //close 
#include <arpa/inet.h>    //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
    
#define TRUE   1 
#define FALSE  0 
#define PORT 49152
    
int client(int port)
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[1024] = {0};
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
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    //while (1){
        //send(sock , hello , strlen(hello) , 0 );
        //printf("Hello message sent\n");
        valread = read( sock , buffer, 1024);
        printf("%s\n",buffer );
    //}
    return 0;
}
int server(int port)  
{  
    int opt = TRUE;  
    int master_socket , addrlen , new_socket , client_socket[30]  = {0}, 
          max_clients = 30 , activity, i , valread , sd;  
    int connected_to_other_server = 0; 
    int max_sd;  
    struct sockaddr_in address;  
    char buffer[1025];  //data buffer of 1K 
    //set of socket descriptors 
    fd_set readfds;  
    //a message 
    char *message = "ECHO Daemon v1.0 \r\n";  
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)  
    {  
        perror("socket failed");  
        exit(EXIT_FAILURE);  
    }  
    //set master socket to allow multiple connections , 
    //this is just a good habit, it will work without this 
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )  
    {  
        perror("setsockopt");  
        exit(EXIT_FAILURE);  
    }  
    //type of socket created 
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons(port);  
    //bind the socket to localhost port 8888 
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)  
    {  
        perror("bind failed");  
        exit(EXIT_FAILURE);  
    }  
    printf("Listener on port %d \n", port);  
    //try to specify maximum of 3 pending connections for the master socket 
    if (listen(master_socket, 3) < 0)  
    {  
        perror("listen");  
        exit(EXIT_FAILURE);  
    }  
    //accept the incoming connection 
    addrlen = sizeof(address);  
    puts("Waiting for connections ...");  
    while(TRUE)  
    {  
        //connect to other server so that we can setup circular server ServerA -> ServerB -> ServerC 
        //                                                               ^_____________________|
        while (connected_to_other_server == 0)
        {
            int client_connection, other_server;
            if (port == 49154)
            {
                client_connection = client(49152);
                other_server = 49152;
            }
            else 
            {
                client_connection = client(port+1);
                other_server = port+1;
            }
            if (client_connection == 0)
            {
                connected_to_other_server = 1;
                printf("Connected to other server with port %d\n",other_server);
                break;
            }
        } 
        FD_ZERO(&readfds);  
        //add master socket to set 
        FD_SET(master_socket, &readfds);  
        max_sd = master_socket;  
        //add child sockets to set 
        for ( i = 0 ; i < max_clients ; i++)  
        {  
            //socket descriptor 
            sd = client_socket[i];  
            //if valid socket descriptor then add to read list 
            if(sd > 0)  
                FD_SET( sd, &readfds);  
            //highest file descriptor number, need it for the select function 
            if(sd > max_sd)  
                max_sd = sd;  
        }  
        //wait for an activity on one of the sockets , timeout is NULL , 
        //so wait indefinitely 
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);  
        if ((activity < 0) && (errno!=EINTR))  
        {  
            printf("select error");  
        }  
        //If something happened on the master socket , 
        //then its an incoming connection 
        if (FD_ISSET(master_socket, &readfds))  
        {  
            if ((new_socket = accept(master_socket,(struct sockaddr *)&address, (socklen_t*)&addrlen))<0)  
            {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }  
            //inform user of socket number - used in send and receive commands 
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
        }  
            //send new connection greeting message
            //if( send(new_socket, message, strlen(message), 0) != strlen(message) )  
            //{  
            //    perror("send");  
            //}  
            //    
            //puts("Welcome message sent successfully");  
            //    
            ////add new socket to array of sockets 
            //for (i = 0; i < max_clients; i++)  
            //{  
            //    //if position is empty 
            //    if( client_socket[i] == 0 )  
            //    {  
            //        client_socket[i] = new_socket;  
            //        printf("Adding to list of sockets as %d\n" , i);  
            //            
            //        break;  
            //    }  
            //}  
            
        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)  
        {  
            sd = client_socket[i];  
            if (FD_ISSET( sd , &readfds))  
            {  
                //Check if it was for closing , and also read the 
                //incoming message 
                if ((valread = read( sd , buffer, 1024)) == 0)  
                {  
                    //Somebody disconnected , get his details and print 
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);  
                    printf("Host disconnected , ip %s , port %d \n" , 
                          inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
                        
                    //Close the socket and mark as 0 in list for reuse 
                    close( sd );  
                    client_socket[i] = 0;  
                }  
                    
                //Echo back the message that came in 
                else
                {  
                    //set the string terminating NULL byte on the end 
                    //of the data read 
                    buffer[valread] = '\0';  
                    send(sd , buffer , strlen(buffer) , 0 );  
                }  
            }  
        }  
    }  
        
    return 0;  
} 


int main(int argc, char *argv[]){
    int i=0;
    int child[3] = {0};
    int port = 49154;
    server(port);
    //for (i = 0; i < 3 ; i++){
    //    if ((child [i] = fork()) == 0){
    //        //child code create server here 
    //        server(port+i); 
    //        sleep(1);
    //        if (i == 2)
    //            client(port);
    //        else 
    //           client(port+i+1);
    //    }
    //} 
     
} 
