//Example code: A simple server side code, which echos back the received message.
//Handle multiple socket connections with select and fd_set on Linux 
#include <stdio.h> 
#include <string.h>   //strlen 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>   //close 
#include <fcntl.h>
#include <arpa/inet.h>    //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 

#define MAXRECVSTRING 1024
#define TRUE   1 
#define FALSE  0 

int listen_broadcast_message(){
    int sock;                         /* Socket */
    struct sockaddr_in broadcastAddr; /* Broadcast Address */
    unsigned short broadcastPort = 49153;     /* Port */
    char recvString[MAXRECVSTRING+1]; /* Buffer for received string */
    int recvStringLen;                /* Length of received string */

    //if (argc != 2)    /* Test for correct number of arguments */
    //{
    //    fprintf(stderr,"Usage: %s <Broadcast Port>\n", argv[0]);
    //    exit(1);
    //}

    //broadcastPort = atoi();   /* First arg: broadcast port */

    /* Create a best-effort datagram socket using UDP */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    //if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        perror("socket() failed");

    /* Construct bind structure */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));   /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;                 /* Internet address family */
    broadcastAddr.sin_addr.s_addr = inet_addr("127.0.0.1");/*htonl(INADDR_LOOPBACK);  * Any incoming interface */
    broadcastAddr.sin_port = htons(broadcastPort);      /* Broadcast port */

    /* Bind to the broadcast port */
    if (bind(sock, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr)) < 0)
        perror("bind() failed");

    /* Receive a single datagram from the server */
    //if ((fcntl(sock, F_SETFL, /*flags | */O_NONBLOCK)) != 0)
    //    {printf ("fcntl failed.\n"); close(sock);return -1;}
    
    if ((recvStringLen = recvfrom(sock, recvString, MAXRECVSTRING, 0, NULL, 0)) < 0)
    //This means that no other servers are runnning on so return unsuccessful here  
    //if(read(sock, recvString, sizeof(recvString)) == 0)
       {printf("recvfrom() failed\n");close(sock);return -1;}
        

    recvString[recvStringLen] = '\0';
    printf("Received: %s\n", recvString);    /* Print the received string */
    
    close(sock);
    //exit(0);
}

int broadcast_message(){
//    void sendBroad(char *dstIP, char *localIP)
    int sock;                         /* Socket */
    struct sockaddr_in broadcastAddr; /* Broadcast address */
    int broadcastPermission;          /* Socket opt to set permission to broadcast */
    unsigned int localIPLen;          /* Length of string to broadcast */


    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    //if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        perror("socket() failed");

    /* Set socket to allow broadcast */
    broadcastPermission = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, 
          sizeof(broadcastPermission)) < 0)
        perror("setsockopt() failed");

    /* Construct local address structure */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));   /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;                 /* Internet address family */
    broadcastAddr.sin_addr.s_addr = inet_addr("127.255.255.255");   /* Broadcast IP address */
    broadcastAddr.sin_port = htons(49155);      /* Broadcast port */

    localIPLen = strlen("127.255.255.255");  /* Find length of localIP */
    //int j;
    //for (j=0; j<1; j++) //doesnt mean anything so far, not important
    //{
    /* Broadcast localIP in datagram to clients */
    if (sendto(sock, "127.255.255.255", localIPLen, 0, (struct sockaddr *) 
          &broadcastAddr, sizeof(broadcastAddr)) != localIPLen)
        perror("sendto() sent a different number of bytes than expected");
    //}
}

int server(char *Address, int Port)  
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
    char *message = "\0";  
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
    address.sin_addr.s_addr = inet_addr(Address);  
    address.sin_port = htons(Port);  
    //bind the socket to localhost port 8888 
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)  
    {  
        perror("bind failed");  
        exit(EXIT_FAILURE);  
    }  
    printf("Listener on port %d \n", Port);  
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
        if (FD_ISSET(master_socket, &readfds)) {  
            if ((new_socket = accept(master_socket,(struct sockaddr *)&address, (socklen_t*)&addrlen))<0)  
            {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }  
            //inform user of socket number - used in send and receive commands 
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
        }
        //send new connection greeting message
        message = "Hi This is from 127.0.0.1\n";
        if( send(new_socket, message, strlen(message), 0) != strlen(message) )  {  
            perror("send");  
        }  
        puts("Welcome message sent successfully");  
        //add new socket to array of sockets 
        for (i = 0; i < max_clients; i++)  {  
            //if position is empty 
            if( client_socket[i] == 0 )  {  
                client_socket[i] = new_socket;  
                printf("Adding to list of sockets as %d\n" , i);  
                    
                break;  
            } 
        } 
        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)  {  
            sd = client_socket[i];
            if (FD_ISSET( sd , &readfds))  {
                //Check if it was for closing , and also read the 
                //incoming message 
                if ((valread = read( sd , buffer, 1024)) == 0)  {
                    //Somebody disconnected , get his details and print 
                    getpeername(sd , (struct sockaddr*)&address ,(socklen_t*)&addrlen);  
                    printf("Host disconnected , ip %s , port %d \n" ,inet_ntoa(address.sin_addr), ntohs(address.sin_port)); 
                    //Close the socket and mark as 0 in list for reuse 
                    close( sd );  
                    client_socket[i] = 0;  
                }
                //Echo back the message that came in 
                else {
                    //set the string terminating NULL byte on the end 
                    //of the data read 
                    buffer[valread] = '\0';
                    send(sd , buffer , strlen(buffer), 0);  
                }
            }
        }
    }
    return 0; 
}

int main(int argc, char *argv[]){
    int i=0;
    int child[3] = {0};
    int port = 49152;
    int listen_return = listen_broadcast_message();
    if (listen_return < 0)
        broadcast_message();
    server("127.0.0.1",49152);
} 
