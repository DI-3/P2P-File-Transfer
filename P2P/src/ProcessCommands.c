#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <fcntl.h>
#include "global.h"
#include "../include/ProcessCommands.h"
#define MSG_MAX_LEN 13

extern fd_set master ;    
extern fd_set read_fds ;
extern int fdmax ;
extern char *portnum;
extern struct clntdata *head ;
extern struct clntdata *headClientNode;
extern struct clntdata *headConnectdNode;
//extern int iconnid;
extern char ipaddress[128];

//void printSArray(char **ptoArray);
int connect_to_server(char *ipaddr,char *portno);
void gethIPaddress(char* buffer, size_t buflen);

int connect_to_server(char *ipaddr,char *portno){

  struct  addrinfo hints,*serverinfo,*temp;
  int sockfd ;
  memset(&hints,0,sizeof hints);
  char ip[INET_ADDRSTRLEN];
  int addrstat;
  int nbytesrcvd;
  
  hints.ai_family = AF_INET ;
  hints.ai_socktype = SOCK_STREAM ;
  hints.ai_flags= AI_PASSIVE ;
  char sbuffer[50];
  
  
  if((addrstat = getaddrinfo(ipaddr,portno,&hints,&serverinfo)) != 0){

    fprintf(stderr,"getaddrinfo error : %s \n",gai_strerror(addrstat));
    exit(1);
  }


  for(temp = serverinfo; temp != NULL; temp = temp->ai_next){

   if((sockfd = socket(temp->ai_family,temp->ai_socktype,temp->ai_protocol)) == -1){
    perror("Client socket");
    continue;
  }
  if(connect(sockfd,temp->ai_addr,temp->ai_addrlen) == -1){
    close(sockfd);
    perror("Client connect error");
    continue ;
  }
  break ;
}

if(temp == NULL){
  fprintf(stderr,"client failed to connect");
  return 2;
}

fcntl(sockfd,F_SETFL,O_NONBLOCK);

FD_SET(sockfd,&master); 
if(sockfd > fdmax){
  fdmax = sockfd ;
} 
inet_ntop(temp->ai_family,get_in_addr((struct sockaddr*)temp->ai_addr),ip,sizeof ip);
printf("Client connecting to %s\n",ip);

     // 
freeaddrinfo(serverinfo);
     //call to pack the data
     //packdata(portno,text);

getServeWrapper(portno,sockfd,ip);

//sending the port number to the server.
if((nbytesrcvd = send(sockfd,portnum,sizeof portnum,0))== -1){

  perror("send");
  exit(1);

}

return sockfd;

}

int processCommand(char *cmd,int clientserverflg,char* portno)
{

 char *arg;
 int argno = 0 ;
 char *argz[30];
 char ips[INET_ADDRSTRLEN];
 int c = 0;
 int length = 0;
 int retfd;
 int argcount ;


 arg = strtok(cmd," ");  
 c = 0 ; 
 
 if(arg == '\0' || arg =="\n"){

   printf("[PA1]$:");
   return 0 ;

 }
 argcount = 0 ;

 while(arg){

   argz[c] = (char *)malloc(50*sizeof(char));
   strcpy(argz[c],arg);
      //printf("\nArgZ:%s",argz[c]);
   c += 1;
   argcount += 1 ; 
   arg = strtok(NULL," ") ;
 }

 if(strcasecmp(argz[0],"HELP")==0){

  if(argcount != 1){
    return -1 ;
  }
  
  printHelp();
  //fflush(stdout);
  printf("[PA1]$:");
        //fflush(stdin);
  *argz = NULL ;

}else if(strcasecmp(argz[0],"CREATOR")==0){

  if(argcount != 1){
    return -1 ;
  } 
  
  printCreator();
        //fflush(stdout);
  printf("[PA1]$:");
        //fflush(stdin);
  *argz = NULL ;
}else if(strcasecmp(argz[0],"MYIP")== 0){

  if(argcount != 1){
    return -1 ;
  }  
  printf("IP address:%s",ipaddress);
  fflush(stdout);
  printf("\n[PA1]$:");
        //fflush(stdin);
  *argz = NULL ;

}else if(strcasecmp(argz[0],"MYPORT")== 0){

  if(argcount != 1){
    return -1 ;
  }
  printf("Port number:%d",atoi(portno));
        //fflush(stdout);
  printf("\n[PA1]$:");
        //fflush(stdin);
  *argz = NULL ;
}else if(strcasecmp(argz[0],"REGISTER")== 0){

 if(argcount != 3){
  return -1 ;
}
if(clientserverflg == 1 ){
 printf("\n[PA1]$:Server does not support the command");
            //fflush(stdout);
 printf("\n[PA1]$:");
}else{
     //call validation for IPs
 retfd = connect_to_server(argz[1],argz[2]);
     //printf("\n Check cond");
}


}else if(strcasecmp(argz[0],"LIST")== 0){

 if(argcount != 1){
   return -1 ;
 }
 printf("\n[PA1]$:");
 if(clientserverflg == 1){
   retfd = listactpeers(0,head,1);
 }else{
   retfd = listactpeers(0,headConnectdNode,1);  
 }



}else if(strcasecmp(argz[0],"CONNECT")== 0){

 //call connection method for IPs
 //validateClient(argz[1],argz[2]);
 if(argcount !=3){
  return -1 ;
}

int no_connections = getConnectionNo();
if(no_connections > 3){

  printf("[PA1]$:Connection rejected!!\You can be connceted to no more than 3 peers!!\n");
  printf("[PA1]:");

}else{
 int valid = peerValidation(argz[1],argz[2]); 
 if(valid == 1){

      retfd = connect_to_client(argz[1],argz[2]);//new method we only need the connect here
    }

  }


 //printf("\n Check cond");

}else if(strcasecmp(argz[0],"TERMINATE")== 0){

   //to do the validation for valid connections
 if(argcount != 2){
  return -1 ;
}
int connct = atoi(argz[1]);
int check ;
   // if((check = validateConnectn(connct)) == 1) {
   //   int valid = closeconnectn(argz[1],clientserverflg);
   // }else{

   //  printf("\n A valid connection does not exist for the connection");

   // }
closeconnectn(argz[1],clientserverflg);

}else if(strcasecmp(argz[0],"EXIT") == 0){

//close all connections
  if(argcount > 1){
    return -1 ;
  }
  closeall(); 
  exit(0);

}else if(strcasecmp(argz[0],"UPLOAD") == 0){

  if(argcount != 3){
    return -1 ;
  }
// send the file to the client
  if(clientserverflg == 1){
    printf("\n[PA1]$:");
    printf("\n UPLOAD not supported by the server");
    fflush(stdout);
  }else{

   invokeFileEng(argz[1],argz[2]);
 }


}else if(strcasecmp(argz[0],"DOWNLOAD")==0){

  if(argcount > 7 || argcount < 3){
    return -1 ;
  }
  if(clientserverflg == 1){
    printf("\n[PA1]$:");
    printf("\n DOWNLOAD not supported by the server");
    fflush(stdout);
  }else{
    createdwnldrequest(&argz,c); 
  }
}else if(strcasecmp(argz[0],"STATISTICS")==0){
   
   if(argcount != 1){
   return -1 ;
  }
   statisticdets(headConnectdNode);
}
else{

  printf("Invalid command !! Type Command HELP to check the options ");
  fflush(stdout);
  printf("\n[PA1]$:");
}

      //printf("The String is %s",*argz);   



//getIPAddress();;
return 1 ;    

}
//this code is referred code from stack overflow -- http://stackoverflow.com/questions/212528/get-the-ip-address-of-the-machine

void gethIPaddress(char* buffer, size_t buflen) 
{

  struct sockaddr_in myipaddr;
  struct sockaddr_in addr;
  // creating a udp socket
  int sock = socket(AF_INET, SOCK_DGRAM, 0);

  const char* dnsIP = "8.8.8.8";
  uint16_t dnsport = 53;
  

  memset(&addr, 0, sizeof(addr));
  
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(dnsIP);
  addr.sin_port = htons(dnsport);

  int conn = connect(sock,(const struct sockaddr *)&addr, sizeof(addr));

  
  socklen_t namelen = sizeof(myipaddr);
  conn = getsockname(sock, (struct sockaddr*) &myipaddr, &namelen);
  const char* p = inet_ntop(AF_INET, &myipaddr.sin_addr, buffer, buflen);
  
  //setting the ip address in a gloabal variable
  strcpy(ipaddress,p);
  strcpy(buffer,ipaddress);

  close(sock);
} 

int connect_to_client(char *ipaddr,char *cportnum){

  struct addrinfo hints,*res,*temp;
  int sockfd,addrstat;
  char clipaddress[INET_ADDRSTRLEN];
  int numbytes;
  char* buffer;
  size_t bufflen;
  memset(&hints,0,sizeof hints);
  hints.ai_family = AF_INET ;
  hints.ai_socktype = SOCK_STREAM ;
  char connctdata[30];
  //printf("IP address and port %s ,%s",ipaddr,cportnum) ;
  if((addrstat = getaddrinfo(ipaddr,cportnum,&hints,&res)) != 0){

    fprintf(stderr,"getaddrinfo error : %s \n",gai_strerror(addrstat));
    exit(1);

  }

  // loop through all the and connect

  for(temp = res ;temp!= NULL;temp = temp->ai_next){
    if((sockfd = socket(temp->ai_family,temp->ai_socktype,temp->ai_protocol)) == -1){

      perror("Client socket connection  not accepted");
      continue;
    }

    if(connect(sockfd,temp->ai_addr,temp->ai_addrlen) == -1){
      close(sockfd);
      perror("client:connect");

      continue;
    }

    break ;
  }
  
  if(temp == NULL){

    fprintf(stderr,"client:failed to connect\n");
    printf("\n[PA1]$:Could not connect to the client");
    fflush(stdout);
    return 2;
  }

  //add the connection to the list
  
  FD_SET(sockfd,&master); 
  
  if(sockfd > fdmax){
   fdmax = sockfd ;
 }
  //get_in_addr((struct sockaddr *)temp->ai_addr)   temp->ai_addr
 inet_ntop(temp->ai_family,get_in_addr((struct sockaddr *)temp->ai_addr),clipaddress,sizeof clipaddress);
 printf("\n---------Connecting to Peer:%s",clipaddress);


//  inet_ntop(temp->ai_family,get_in_addr((struct sockaddr*)temp->ai_addr),ip,sizeof ip);
  //    printf("Client connecting to %s\n",ip);

 freeaddrinfo(res);

 strcat(connctdata,"CON");
 strcat(connctdata,"@@@");
 strcat(connctdata,portnum);

 if((numbytes = send(sockfd,connctdata,sizeof connctdata,0)) == -1) {

   perror("send");
   exit(1);
 }
  //printf("No of bytes sent %d",sockfd) ;
 printf("\n---------Connected to Peer:%s",clipaddress);
 printf("\n[PA1]$:");
 getClientWrapper(cportnum,sockfd,clipaddress);

 return sockfd ;
}



