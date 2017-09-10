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
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    //if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        perror("socket() failed");

    /* Construct bind structure */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));   /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;                 /* Internet address family */
    broadcastAddr.sin_addr.s_addr = inet_addr("127.0.0.1");//htonl(INADDR_ANY);  /* Any incoming interface */
    broadcastAddr.sin_port = htons(broadcastPort);      /* Broadcast port */

    /* Bind to the broadcast port */
    if (bind(sock, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr)) < 0)
        perror("bind() failed");

    /* Receive a single datagram from the server */
    if ((fcntl(sock, F_SETFL, /*flags | */O_NONBLOCK)) != 0)
        {printf ("fcntl failed.\n"); close(sock);return -1;}
    
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
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    //if ((sock = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
        perror("socket() failed");

    /* Set socket to allow broadcast */
    broadcastPermission = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, 
          sizeof(broadcastPermission)) < 0)
        perror("setsockopt() failed");

    /* Construct local address structure */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));   /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;                 /* Internet address family */
    broadcastAddr.sin_addr.s_addr = /*htonl(INADDR_BROADCAST);*/inet_addr("127.0.0.1");   /* Broadcast IP address */
    broadcastAddr.sin_port = htons(49153);      /* Broadcast port */

    localIPLen = strlen("127.0.0.1");  /* Find length of localIP */
    for (; ;) //doesnt mean anything so far, not important
    {
    /* Broadcast localIP in datagram to clients */
    if (sendto(sock, "127.0.0.1", localIPLen, 0, (struct sockaddr *) 
          &broadcastAddr, sizeof(broadcastAddr)) != localIPLen)
        perror("sendto() sent a different number of bytes than expected");
    //else 
    //    printf("Message Broadcasted.\n");
    }
}
typedef struct SocketTable {
     int socket;
     char clientName[10];
}SockTab;

char * getPeerName(char packet[], char buff[]) {
    int i, j=0;
    for (i = 0; i < strlen(packet); i++) {
        if (j >= 2) {     
           if (packet [i] == '#') {
               buff[j - 2] = '\0';
               break;
           } else {
               buff[j - 2] = packet[i];
               j++;
           }
        }
        if (packet[i] == '#' && j < 2)
           j++;
        
    }
    return buff;
}

int getDestSocket(char packet[], char buff[], SockTab Table[]) {
    int i, j=0;
    for (i = 0; i < strlen(packet); i++) {
        if (j >= 4) {     
           if (packet [i] == '#') {
               buff[j-4] = '\0';
               break;
           } else {
               buff[j - 4] = packet[i];
               j++;
           }
        }
        if (packet[i] == '#' && j < 4)
           j++;
    }
    for (i = 0; i < 30 ; i++) {
        if (!strcmp (buff, Table[i].clientName)) {
            return Table[i].socket;
        }
    }
    return -1;
}

int server(char *Address, int Port)  
{  
    int opt = TRUE;  
    int listen_socket , addrlen , new_socket , client_socket[30]  = {0}, max_clients = 30 , activity, i , valread , sd, j;  
    int connected_to_other_server = 0; 
    SockTab Table[100];
    for (i = 0; i < 100; i++)
        Table[i].socket = 0;
    i = 0;
    int max_sd;  
    struct sockaddr_in address;  
    char buffer[1025];  //data buffer of 1K 
    char PeerName[10] = {'\0'};
    char DestName[100] = {'\0'};
    fd_set readfds, serverfds, masterfds;  
    char *message = "\0"; 
    int k; 
    if( (listen_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) {
        perror("socket failed");  
        exit(EXIT_FAILURE);  
    }
    if( setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {
        perror("setsockopt");  
        exit(EXIT_FAILURE);  
    }
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = inet_addr(Address);  
    address.sin_port = htons(Port);  
    if (bind(listen_socket, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", Port);
    
    if (listen(listen_socket, 15) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    addrlen = sizeof(address);
    puts("Waiting for connections ...");
    FD_ZERO(&readfds);
    FD_ZERO(&masterfds);
    FD_SET(listen_socket, &masterfds);
    FD_SET(listen_socket, &readfds);
    max_sd = listen_socket; 
    while(TRUE) {
        readfds = masterfds;
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
        if ((activity < 0) && (errno!=EINTR)) {
            printf("select error");
        }
        printf("activity :%d\n",activity);
        if (FD_ISSET(listen_socket, &readfds)) {
            if ((new_socket = accept(listen_socket,(struct sockaddr *)&address, (socklen_t*)&addrlen))<0)  {  
                perror("accept");
                exit(EXIT_FAILURE);
            }

            valread = read( new_socket , buffer, 1024);
            FD_SET(new_socket, &masterfds);
            readfds = masterfds;
            if (new_socket > max_sd) 
                max_sd = new_socket;
           
            for (i = 0; i < 100; i++) {
                if (Table[i].socket == 0) {
                    k = i;
                    break;
                }
            }
            
            strcpy(Table[k].clientName,getPeerName(buffer, Table[k].clientName));
            Table[k].socket = new_socket;
            
            printf("Socket , Client Name :%d, %s\n",Table[k].socket,Table[k].clientName);
            printf("New connection, socket fd is %d, ip is : %s, port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
            
            for (i = 0; i < max_clients; i++) {
                //if position is empty 
                if( client_socket[i] == 0 ) {
                    client_socket[i] = new_socket;  
                    printf("Adding to list of sockets as %d\n" , i);  
                    break;  
                }
            }
        } else {
        for (i = 0; i < max_clients; i++) {  
            sd = client_socket[i];
            if (FD_ISSET( sd , &readfds)) {
                if ((valread = read( sd , buffer, 1024)) == 0) {
                    getpeername(sd , (struct sockaddr*)&address ,(socklen_t*)&addrlen);
                    FD_CLR(sd, &masterfds);
                    FD_CLR(sd, &readfds);
                    
                    for (k = 0; k< 100; k++) {
                        if (sd == Table[k].socket) {
                            Table[k].socket = 0;
                            Table[k].clientName[0] = '\0';
                            break;
                        }
                    }
                    printf("Host disconnected , ip %s , port %d \n" ,inet_ntoa(address.sin_addr), ntohs(address.sin_port)); 
                    client_socket[i] = 0; 
                    //Close the socket 
                    if (max_sd == sd) {
                        max_sd = 0;
                        for (k = 0; k < max_clients; k++) {
                            if (client_socket[k] > max_sd)
                                max_sd = client_socket[k];
                        }
                    }
                    close(sd);
                    // make client socket entry empty 
                     
                } else {
                    buffer[valread] = '\0';
                    printf("%s\n",buffer);
                    j = getDestSocket(buffer,DestName, Table);
		    if (j < 0) {
		       printf("Error : No destination found");
                       // send this to another server cause dest client is not connected to this server.
		    }
                    write(j , buffer,strlen(buffer));  
                }
            }
        }
    }
    }
    return 0; 
}

int main(int argc, char *argv[]) {
    int i=0;
    int child[3] = {0}, pid;
    int port = 49152;
    //int listen_return = listen_broadcast_message();
    //if (listen_return < 0)
    //    pid = fork();
    //    if (pid == 0)
    //        broadcast_message();
    server("127.0.0.1",49152);
} 
