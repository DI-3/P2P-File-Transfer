/**
 * @mithunna_assignment1
 * @author  Mithun Nagesh <mithunna@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define PORT "9034"
#include "../include/ProcessCommands.h"
#include "../include/global.h"
#define DATADELIM "##--##"
// port we're listening on
// get sockaddr, IPv4 or IPv6:

//extern char *ipaddress;
struct clntdata {

  int sockdscrptr;
  int iconnectionid ;
  char *cipaddr;
  char *cportNum;
  char *chname ;
  int connstat;
  int ndownloads;
  int nuploads;
  double uploadspeed;
  double downloadspeed;
  struct clntdata* next ;

};
char fdelimiter[4] ="#-#";
char rdelimiter[4] ="@@@";
const char regCmnd[4]="REG";
const char conCmnd[4]="CON";
static int iconnid = 0; 

fd_set master ;    
fd_set read_fds ;
int fdmax ;
int serverclientflag ;

int nbytes ;
char remoteIP[INET6_ADDRSTRLEN];
char cmd[100];   
int length = 0 ;
int listener ;  //listening socket
int newfd ;  //newly accepted socket descriptor
struct sockaddr_storage remoteaddr; //client address
socklen_t addrlen ;
char *portnum ;
int returnfd;
char ips[INET_ADDRSTRLEN]; 
char *sbcheckdata ="delimitedStr";
struct clntdata **headRef;
struct clntdata *head ;
struct clntdata *headClientNode;
struct clntdata *headConnectdNode = NULL;

void processCmd(char *rcvddata,int socket,int bytes_rcvd);
void clearClientList(struct clntdata **headRef,int iption);
void updtConnectionStatus(char *ipaddr,char *cport);
void updateConnctdPort(int socket,char *port,struct clntdata **headConnectdNode,char *rip);
void getConnectDetails(int fd,char *data,struct clntdata **headRefNode,char *ip);
void updateClientList(int sockfd,int clientserverflag);
void sendupdate(int currsockfd,int listener);
void sendReqDownload(int sockfd,char * filename);

// This method adds the registered clients to a list
/*  Method adds clients to the peer list/server list */
void addClientstoList(struct clntdata **headRef,char *ipaddrs,char* cportNo,char *chostname,int connid,int fd){

  struct clntdata *newclntdata;
  struct clntdata *current = *headRef;
  size_t num_bytes ;
  int ln;
  
  newclntdata = (struct clntdata *)malloc(sizeof(struct clntdata));
  num_bytes = strlen(ipaddrs) + 1;
  //printf("The size of string is %lu",num_bytes);
  newclntdata->cipaddr = (char *)malloc(num_bytes * sizeof(char));
  num_bytes = strlen(cportNo) + 1;
  newclntdata->cportNum = (char *)malloc(num_bytes * sizeof(char));
  num_bytes = strlen(chostname) + 1;
  newclntdata->chname = (char *)malloc(num_bytes * sizeof(char));

  newclntdata->iconnectionid = connid;
  newclntdata->sockdscrptr = fd;
  if(serverclientflag == 1){
    newclntdata->connstat = 1;  
  }else{
    newclntdata->connstat = 0;
  }

  strcpy(newclntdata->cipaddr,ipaddrs);
  strcpy(newclntdata->cportNum,cportNo);
  strcpy(newclntdata->chname,chostname);
  newclntdata->nuploads = 0 ;
  newclntdata->ndownloads = 0;
  newclntdata->uploadspeed = 0.00;
  newclntdata->downloadspeed=0.00;

  newclntdata->next = NULL ;
  
  if(current == NULL){

   *headRef = newclntdata ;

 } else{

   while(current->next != NULL){

    current = current->next ;
  }

  current->next = newclntdata ;

 }
 

}


void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/* Builds the list of updated clients,this is only valid in case of server where we need to send 
   the updated information to all the clients    */

int buildclist(struct clntdata *heads){

  struct clntdata *current ;
  char buffer[500]="\0";
  current = heads;
  //printf("Address head %p",&head);
  int count=0;
  char temp[33];
  int bufflen ;
  
  strcat(buffer,regCmnd);
  
  while(current != NULL){
    int i = current->iconnectionid;
    //temp = itoa(i,temp,10);
    sprintf(temp,"%d",i);
    strcat(buffer,rdelimiter);
    strcat(buffer,current->cipaddr);
    strcat(buffer,fdelimiter);
    strcat(buffer,current->cportNum);
    strcat(buffer,fdelimiter);
    strcat(buffer,current->chname);
    strcat(buffer,fdelimiter);
    strcat(buffer,temp);
    strcat(buffer,fdelimiter);
    i = current->sockdscrptr ;
    sprintf(temp,"%d",i);
    strcat(buffer,temp);
    
    count++;
    current = current->next ;
  }
  bufflen = strlen(buffer);
  //printf("\n Length of data %d",bufflen);
  sbcheckdata =(char *)malloc((bufflen + 1)*(sizeof (char))); 
  strcpy(sbcheckdata,buffer);
  return count ;
}


/* This method intialises the connections and sets the listening port */

void initialize(){

  int yes = 1;
  int i,j,rv;
  char *commands ;
  struct addrinfo hints,*ai,*p ;
  //char ips[INET_ADDRSTRLEN];  
  

  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  
  memset(&hints,0,sizeof hints);
  hints.ai_family =AF_INET ;
  hints.ai_socktype = SOCK_STREAM ;
  hints.ai_flags = AI_PASSIVE ;

  printf("[PA1]$:");
  fflush(stdout);
  
  gethIPaddress(ips,sizeof ips);
  
  if((rv = getaddrinfo(NULL,portnum,&hints,&ai)) !=0 ){
    //error
    fprintf(stderr,"selectserver %s\n",gai_strerror(rv));
    exit(1);
  }

  for(p = ai; p!=NULL;p= p->ai_next){

   listener = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
   if(listener < 0){
    continue ;
  }

  setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));

  if(bind(listener,p->ai_addr,p->ai_addrlen) < 0) {
    close(listener);;
    continue ;
  } 
  break ;
}
    //printf("Debug point2 %d",listener);
    //
if(p == NULL) {

 fprintf(stderr,"selectserver  failed to bind%s\n",gai_strerror(rv));
 exit(2);

} 

freeaddrinfo(ai);

    //listen

if(listen(listener,10) == -1){

 perror("listen");
 exit(3); 
}

 // add the listtener to master set 

FD_SET(listener,&master);
FD_SET(0,&master);
fdmax = listener ;


}

/* The main function */

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */


int main(int argc,char** argv){

  int yes = 1;
  int i,j,rv;
  char *commands ;
  int length = 0 ;
  char array[100];
  char *cmdargs;
  char *res ;
  cmdargs = array;
  head = NULL ;
  headRef = &head; 
  int ln1;
  int obytes ;
  int retSel; 
  char ipremote[INET_ADDRSTRLEN];

  /* Bufferrs for read and write */

  char buf[350] ;
  char opbuff[350];
 
  
  

  if(argc != 3){

   printf("Invalid call ..exiting............\n");
   exit(0);
  }
  
  if(strcasecmp(argv[1],"s") != 0 &&  strcasecmp(argv[1],"c") != 0 ){

    printf("Invalid arguments ..exiting............\n");
    exit(0);

  }


  int iport ;
  iport = atoi(argv[2]);

  if(iport <= 1024 || iport > 45355){
 
     printf("Invalid port ..exiting............\n");
    exit(0);   

  }

 /* Setting the client or server flag */

  if(strcasecmp(argv[1],"S")== 0 ){
      serverclientflag = 1;
  }else{

    serverclientflag = 2 ;
  }

  portnum = argv[2]; // setting the portnum to global variable
  
  //Method to initialise the system

  initialize();
 
  
  for(;;){
 

  read_fds = master ;

  if(select(fdmax + 1,&read_fds,NULL,NULL,NULL) == -1){
    perror("select");
    exit(4);
  }

        //run through the existing connections looking for data
  
  for(i=0 ; i<= fdmax ; i++){

         if(FD_ISSET(i,&read_fds)){ // we got one conncetion

            if(i == 0){
             //read the command from the terminal
             //printf("[PA1]$");
             if((res=fgets(cmd,sizeof cmd,stdin) )!= NULL){
               length = strlen(cmd);
               cmd[length-1] = '\0';   
                //fflush(stdin);
                commands = cmd ;
                 returnfd = processCommand(commands,serverclientflag,portnum); //get the file descriptor for server connection 
                 fflush(stdout);

                 if(returnfd == -1){
                  
                   printf("Invalid arguments to command.");
                   fflush(stdout); 
                   printf("\n[PA1]$:");
                   fflush(stdout); 
                 }
               }
                 
             }else{
              
              if(i == listener){
                //handle new connection accept the connection and add it to the read set
                addrlen = sizeof remoteaddr ;
                newfd = accept(listener,(struct sockaddr *)&remoteaddr,&addrlen);
                
                if(newfd == -1){
                 perror("accept");
                 } else{

                    /* Getting the ip address of the remote connection */
                    /*And adding the  connection to list*/

                         struct sockaddr_in *s = (struct sockaddr_in *)&remoteaddr;
                         struct hostent *hnet; 
                         inet_ntop(AF_INET, &s->sin_addr, ipremote, sizeof ipremote);
                         int port = s->sin_port;
                         char tempport[33]; 
                         sprintf(tempport,"%d",port);
                         //hnet = gethostbyaddr(&ipremote,sizeof ipremote,AF_INET);
                         if(serverclientflag == 1){

                           getConnectDetails(newfd,tempport,&head,ipremote);
                         }else{

                           getConnectDetails(newfd,tempport,&headConnectdNode,ipremote);
                         }
                         
                         //adding the connection details in the memory  
                         //addClientstoList(&headConnectdNode,ipremote,portnum,hnet->h_name,iconnid,i); 
                         //portnum;
                    printf("----------A new connection requested from :%s",inet_ntop(remoteaddr.ss_family,get_in_addr((struct sockaddr*)&remoteaddr),remoteIP,INET6_ADDRSTRLEN));
                    printf ("\n---------------------Connected to :%s-----------\n",inet_ntop(remoteaddr.ss_family,get_in_addr((struct sockaddr*)&remoteaddr),remoteIP,INET6_ADDRSTRLEN));
                    
                    FD_SET(newfd,&master);
                    
                    //End Send the updated list of clients
                    if(newfd > fdmax){
                      fdmax = newfd ;
                   }
                    // process register command
                  }

                }else{

                     /*handle data from the client
                     clear the data from the buffer
                     */

                     if((nbytes = recv(i,buf,sizeof buf,0)) <= 0 ){
                       fflush(stdout);
                       if(nbytes == 0 ){
                         //connection closed
                          if(serverclientflag == 1){                       
                          //Reset the client list and push data to other clients
                           updateClientList(i,serverclientflag) ;   
                           buildclist(head);
                           sendupdate(i,listener);  
                          }else{
                         
                          updateClientList(i,serverclientflag);  
                          iconnid = iconnid - 1 ; 
                          }                              
                          printf("\n[PA1]$:");
                          fflush(stdout);
                        }else{
                         perror("recv");
                         printf("\n wrog connection");
                        } 
                      //close(i);
                      FD_CLR(i,&master);
                      printf("\n[PA1]$:"); 
                      }else{

                            //printf("\n Bytes received is %d",nbytes); 
                            
                             //we have data from the client
                            //buf[nbytes] ='\0';
                            //printf("\nBytes %d",nbytes) ;
                            //Get the data about the server
                            if(serverclientflag == 1){
                            
                                //Adding the server details first
                                //getClientDetails(i,buf,&head);
                                char ips[INET6_ADDRSTRLEN] ;
                                char *rip = ips ;
                                updateConnctdPort(i,buf,&head,rip);
                                //addClientstoList(&head,ipaddress,portnum,"dokken.cse.buffalo.edu",1);
                                ln1 =buildclist(head);
                                strcpy(opbuff,sbcheckdata);
                                obytes = send(i,opbuff,sizeof opbuff,0);
                                if(obytes < 0){
                                perror("senddata");
                            }
                               //obytes = send(i,"HEllo",6,0);
                              fflush(stdout);
                              //printf("\n Bytes transferred %d",obytes);
                           }else{

                            processCmd(buf,i,nbytes);
                            memset(buf,0,sizeof buf);
                            
                           }
                         
                           if(serverclientflag == 1 ){

                          //Send the updated data to all the clients
                          for(j=0;j<=fdmax;j++){
                             if(FD_ISSET(j,&master)){
                              //need not send the data to listener && this machine
                              if(j!=listener && j!=i){

                                if(send(j,opbuff,sizeof opbuff,0)== -1){
                                //perror("Sending the list data");
                              }
                              }
                            }
                          }
                        }
                        
                        //Send the updated data to all the clients
                       fflush(stdout);
                       printf("\n[PA1]$:");
                       fflush(stdout);
                       
                 }
               }

             }

           }
        } //end of the readset polling loop
    } //end of the running loop 
  
    return 0 ;
  }     

  /*Get the delimited data and save it in a strcture in the memory
    The data that reaches the client or server is processed in this 
    method */
  void processCmd(char *rcvddata,int socket,int bytes_rcvd){
    
    char *ipdata;
    char *arg;
    char *iprecdata;
    char *argz;
    char *field;
    int strdlen = strlen(rcvddata);
    int strreclen ;
    ipdata =(char *)malloc((strdlen + 1)* sizeof(char)); 
    strcpy(ipdata,rcvddata) ;
    int count = 0;
    int idatacnt = 0;
    char *dataarr[50];
    char *tokens[3];
    int leng;
    int ifunction ;
    int iIndex;
    int oIndex = 0 ;
    //char hport[];
    int iLoop;

    //printf("\n Entry point data:%s",ipdata);
    arg = strtok(ipdata,rdelimiter);

    while(arg){
     //printf("\n Entry point2:%s",arg);
     if(strcmp(arg,"REG") == 0){

      ifunction = REGISTER_CMD ;
      //clear the list and build new
      clearClientList(&headClientNode,0);
      
    }else if(strcmp(arg,"CON") == 0){

      ifunction =  CONNECT_CMD ;

    }else if(strcmp(arg,"UP") == 0){

      ifunction = UPLOAD_CMD ;
      oIndex = 0 ;

    }else if(strcmp(arg,"DOW") == 0){

      ifunction = DOWNLOAD_CMD ;

    }else if(strcmp(arg,"E1")== 0){

      printf("--------File error : File not found at remote peer");
 
    }

    if(ifunction == CONNECT_CMD && count == 1){

      
      char ips[INET6_ADDRSTRLEN] ;
      char *rip = ips ;
      updateConnctdPort(socket,arg,&headConnectdNode,rip);
      printf("\n-------Connected to Peer :%s",ips);

    }

    if(ifunction == UPLOAD_CMD){
     strreclen = strlen(arg); 
     dataarr[oIndex] = (char *)malloc((strreclen + 1)* sizeof(char));
     strcpy(dataarr[oIndex],arg);
     //printf("\n Entry point4:%s",dataarr[oIndex]);
     oIndex++ ;

    }

    if(ifunction == DOWNLOAD_CMD){
     strreclen = strlen(arg);
     dataarr[oIndex] = (char *)malloc((strreclen + 1)* sizeof(char));
     strcpy(dataarr[oIndex],arg);
     oIndex++ ;
    }

    if(ifunction == REGISTER_CMD ){
    strreclen = strlen(arg);
    //iprecdata = (char *)malloc((strreclen + 1)* sizeof(char));
    //strcpy(iprecdata,arg);
    dataarr[oIndex] = (char *)malloc((strreclen + 1)* sizeof(char));
    strcpy(dataarr[oIndex],arg);
    //printf("\n Entry point4:%s",dataarr[oIndex]);
      
      oIndex++ ;
    }

    count++ ;
    arg = strtok(NULL,rdelimiter) ;
  }
if(ifunction == REGISTER_CMD){
  for(iLoop = 0;iLoop < oIndex;iLoop++){

    //printf("Row data is:%s",dataarr[iLoop]);
    char *rec = strtok(dataarr[iLoop],fdelimiter);
      //rec = strtok(dataarr[iLoop],fdelimiter);
    iIndex = 0 ;
    while(rec){

      tokens[iIndex] = strdup(rec);
      if(iIndex == 4){
        //Call method to add to the clients to server list
        //printf("\n %s, %s, %s,%s",tokens[0],tokens[1],tokens[2],tokens[3]);
        addClientstoList(&headClientNode,tokens[0],tokens[1],tokens[2],atoi(tokens[3]),1);
          
      }
      iIndex++ ;
      rec = strtok(NULL,fdelimiter);

    }

  }
}
    
   if(ifunction == REGISTER_CMD){
     listactpeers(0,headClientNode,0);
   }

   if(ifunction == UPLOAD_CMD){

    //call multiple receive
    //fileReceive1(dataarr[1],dataarr[2],socket);
    if(oIndex ==3){

      dataarr[oIndex] = (char *)malloc((strreclen + 1)* sizeof(char));
      strcpy(dataarr[oIndex],"\0");
    }    
    fileReceive(dataarr[1],dataarr[2],dataarr[3],socket,bytes_rcvd);
    //printf("\n  THe sample data is%s :%s :%s",dataarr[1],dataarr[2],dataarr[3]);
   }
   
   
   if(ifunction == DOWNLOAD_CMD){
     
    int connectId = getConnectionId(socket);
    int success = 0;
    success = fileProcess(socket,dataarr[1],connectId);

    //error in reading file send it to client
    if(success == 0){

     send(socket,"E1@@@E",7,0);

    }

   } 

   if(ifunction == CONNECT_CMD){
     
          
   }
}


/* Method displays all the details of all the connected peers  */

int listactpeers(int option,struct clntdata *head,int iselect){

  struct clntdata *current ;
  int count ;
  current = head;
  
  if(current == NULL){
   
   printf("No Connection exixts...");
   printf("\n[PA1]$:");
   return 0 ;

  }
  if(iselect == 1){
    
    printf("\n-------------------------- CONNECTION LIST---------------------------\n");

  }else{

    printf("\n-------------------------- UPDATED PEER LIST---------------------------\n");
  }
  
  printf("%-5s%-35s%-20s%-8s\n","ID","HOSTNAME","IP ADDRESS","PORT No");

  while(current != NULL){
   if(option == 1){

     if(current->connstat == 1 || current->iconnectionid == 1){
        printf("%-5d%-35s%-20s%-8s%-5d\n",current->iconnectionid,current->chname,current->cipaddr,current->cportNum,current->sockdscrptr);
      }
    }else{
    printf("%-5d%-35s%-20s%-8d\n",current->iconnectionid,current->chname,current->cipaddr,atoi(current->cportNum));
    }
    current = current->next ;
    count = 1 ;
  }

  printf("-----------------------------------------------------------------------\n");
  printf("[PA1]$:");
  fflush(stdout);
  return count ;


}

/* The method clears the list when a peer exits                */


void clearClientList(struct clntdata **headRefNode,int option){

struct clntdata *current = *headRefNode ;
struct clntdata *next ;

while(current != NULL){

  next = current->next ;
  if(option == 1){
    
    close(current->sockdscrptr);
    FD_CLR(current->sockdscrptr,&master);
  }
  free(current);
  current = next ;
}

 *headRefNode = NULL ;
}

void getClientWrapper(char *cport,int sockfd,char *ip){

   getConnectDetails(sockfd,cport,&headConnectdNode,ip);
   
   //updtConnectionStatus(ip,cport);  
}

void getServeWrapper(char *cport,int sockfd,char *ip){
   
   getConnectDetails(sockfd,cport,&headConnectdNode,ip);
}


void updtConnectionStatus(char *ipaddr,char *cport){

   struct clntdata *current = headClientNode ;
   while(current!=NULL){
   
        if(((strcmp(current->chname,ipaddr) == 0) || (strcmp(current->cipaddr,ipaddr) == 0)) && (strcmp(cport,current->cportNum)==0)){

           //printf("\nData is IPADDR:%s PORT:%s SOCKDESCRPR:%d",current->cipaddr,current->cportNum,current->sockdscrptr);
           current->connstat = 1;
           break; 
        }
        current = current->next ;
   }

}

int  peerValidation(char *ip,char *cport){

  //Loop through the present connections and update the 
  //connection status
  int errorflag = 0;
  int success = 0;

   
  if((strcmp(ip,ipaddress) == 0) && (strcmp(cport,portnum) == 0)){

   printf("\n[PA1]$: Self connection not allowed !!");
   printf("\n[PA1]$:");
   fflush(stdout);  
   return 0 ; 
  }

  errorflag = validateClient(ip,cport,headClientNode);
  //errorflag = 1 ;
  if(errorflag == 0)
  {
    printf("\n Invalid IP/HOST NAME OR AN IP/HOSTNAME NOT LISTED BY THE SERVER");     
    printf("\n[PA1]$:");
    fflush(stdout);    
  }else{
    errorflag = validateClient(ip,cport,headConnectdNode);
    if(errorflag == 1){
    
     printf("\n Duplicate Connection already exists");
     printf("\n[PA1]$:");
    fflush(stdout);  
    } else{
      success = 1 ;
      //updtConnectionStatus(ip,cport);  
    }
  } 

   return success ;
} 



int validateClient(char *ipaddr,char *cport,struct clntdata *head){
   
  struct clntdata *current = head ;
  int errorflag = 0 ;
  while(current!=NULL){
    
    if(((strcmp(current->chname,ipaddr) == 0) || (strcmp(current->cipaddr,ipaddr) == 0)) && (strcmp(cport,current->cportNum) == 0)){
      
     errorflag = 1 ; 
     
     break ;
    }    
    current = current->next ;
  }
  return errorflag ;
}


void getConnectDetails(int fd,char *data,struct clntdata **headRefNode,char *ip){


  struct hostent *he ;
 int port;
 char portNo[10] ;
 struct in_addr ipv4addr ;
 //headRef = &head ;
 int ln;
 
 strcpy(portNo,data) ;
 
 inet_pton(AF_INET,ip,&ipv4addr);
 he=gethostbyaddr(&ipv4addr,sizeof ipv4addr,AF_INET);  
 
  //Build the list of the clients
 iconnid+=1;
 addClientstoList(headRefNode,ip,data,he->h_name,iconnid,fd);
 // ln =length1();
}


void closeconnectn(char* connectionId,int clientserv){

 int connidc ;
 struct clntdata * prev,*current ;
 connidc = atoi(connectionId);
  
  if(clientserv == 1){

   current = head ;
  }
  else{

  current = headConnectdNode;
  }  

  //Loop through the connections and remove the node and 
  //close the connection
   prev = NULL ;
 
  while(current != NULL){
   
   
   if(current->iconnectionid == connidc){

       if(prev == NULL){
       
        if(clientserv == 1){
           head = current->next ;
        }else{

           headConnectdNode = current->next ;
        }
         

       }else{

        prev->next = current->next;
        
       }
       int isockDescrptr = current->sockdscrptr ;
       free(current);
       iconnid = iconnid - 1;
       close(isockDescrptr);
       FD_CLR(isockDescrptr,&master);
       printf("\n Terminated the connection :%d",connidc);
       printf("\n[PA1]$:");
       return ; 
   }
   
   prev = current ;
   current = current->next ;
      
  }

}

int validateConnectn(int iconnectionid,int clientservflg){
   
  struct clntdata *current ;
  int errorflag = 0 ;

  if(clientservflg == 1)
  current= head;
  else
  current = headConnectdNode;

  while(current!=NULL){
    
    if((current->iconnectionid) == iconnectionid){
      
     errorflag = 1 ; 
     
     break ;
    }    
    current = current->next ;
  }
  return errorflag ;
}

// This method closes all the active connections and informs the server 
void closeall(){

  // send the message to the server to intimate connection close

  clearClientList(&headClientNode,1);
  
  //inform the client
}

void updateClientList(int sockfd,int clientserverflag){

 

 struct clntdata * prev,*current ;
  
  if(clientserverflag == 1){
     current = head ;
  }else{
     current = headConnectdNode ;
  }
  
  
  //Loop through the connections and remove the node and 
  //close the connection
  prev = NULL ;
  
  while(current != NULL){
   
   
   if(current->sockdscrptr == sockfd){
       printf("\n-----The peer %s:%s has terminated the connection..",current->cipaddr,current->chname); 
       if(prev == NULL){
           if(clientserverflag == 1){
            head = current->next ;
           }else{
            headClientNode = current->next ;
           }
           

       }else{

        prev->next = current->next;
        
       }

       free(current);
       return ; 
   }
   
   prev = current ;
   current = current->next ;
      
  }
  
   
  
}

void sendupdate(int currsockfd,int listener){

int j= 0 ;
char opbuff[200];
//printf("DAta is dta %s",sbcheckdata);
strcpy(opbuff,sbcheckdata);
//strcpy(opbuff,sbcheckdata);
for(j=1;j<=fdmax;j++){
    if(FD_ISSET(j,&master)){
      //need not send the data to listener && this machine
        if(j!=currsockfd && j!= listener){

          if(send(j,opbuff,sizeof opbuff,0)== -1)
               perror("Sending the list data");
            }
        }
    }

}
void updateConnctdPort(int socket,char *port,struct clntdata **headConnectdNode,char *rip){

struct clntdata *current = *headConnectdNode ;
  
  while(current!=NULL){
    if(current->sockdscrptr == socket){
    
     strcpy(current->cportNum,port);
     //rip =(char *)malloc(sizeof(char)*(strlen(current->cipaddr))); 
     strcpy(rip,current->cipaddr); 
     break ;
    }    
    current = current->next ;
  }
 
}

int getSockDscpConn(int connectionId){

   struct clntdata *current = headConnectdNode ;
   int sockfd = -1 ;
   while(current != NULL){
     
     if(current->iconnectionid == connectionId){
        
         return current->sockdscrptr ;

     }
    current = current->next ;
   }

 } 

int invokeFileEng(char *connectionId,char *filename){
  int success ;
  int sockfd ;
  int conid = atoi(connectionId);
  sockfd = getSockDscpConn(conid);
  
  success =fileProcess(sockfd,filename,conid);


  //get the socket id and send to the file processing program 
 
}


void createdwnldrequest(char **argz,int c){

int iIndex ;
int connId ;
int valid ;
int sockfd ;
int errorflag = 0 ;
 for(iIndex = 0;iIndex < c ; iIndex++){
  
    valid = iIndex % 2 ; 
    if(valid !=0){
     
      sockfd = validateConnectionIds(atoi(argz[iIndex]));
      if(sockfd == 0){
                
       errorflag = 1; 
       printf("\n Enter a valid connection id in the command");
       break ;   
      } 
    }
  }

 if(errorflag != 1 ){

  //process the connection ids and send the requests
  for(iIndex =0 ;iIndex < c;iIndex++){
      valid = iIndex % 2 ;
      if(valid != 0){
        sockfd = validateConnectionIds(atoi(argz[iIndex]));

        sendReqDownload(sockfd,argz[iIndex+1]);
      }
      
  } 
}


}

void sendReqDownload(int sockfd,char * filename){

   int sendbytes = 0;
   char tempbuffer[150];
   int cmdlen =0;

   memset(tempbuffer,0,sizeof tempbuffer);
   strcat(tempbuffer,"DOW");
   strcat(tempbuffer,"@@@");
   strcat(tempbuffer,filename);
   
   //printf("\nTHe file name is  %s",tempbuffer);
   cmdlen = strlen(tempbuffer);
   if((sendbytes = send(sockfd,tempbuffer,(cmdlen + 1) * sizeof(char),0)) == -1){
       perror("sendERRR");
       exit(1);
   }

}

int validateConnectionIds(int connId){

struct clntdata *current;
current = headConnectdNode ;
int sockfd= 0 ;

   while(current!= NULL){

      if(current->iconnectionid == connId){

        sockfd = current->sockdscrptr ;
      }
     current = current->next ; 
   }

  return sockfd ;
}



int getConnectionId(int socketId){

struct clntdata *current;
current = headConnectdNode ;
int iconnect= 0 ;

   while(current!= NULL){

      if(current->sockdscrptr == socketId){

        iconnect = current->iconnectionid ;
      }
     current = current->next ; 
   }

  return iconnect ;
}


int getConnectionNo(){

  return iconnid ;

}


int updateTransmissionDetails(int connectionId,double tf_rate,int option){


struct clntdata *current;
current = headConnectdNode ;
int sockfd= 0 ;
   
   while(current!= NULL){

    if(option == 1){
      if(current->iconnectionid == connectionId){

        current->uploadspeed += tf_rate ; 
        current->nuploads += 1;
       } 
     }else{
        
        if(current->sockdscrptr == connectionId){
        current->downloadspeed += tf_rate ; 
        current->ndownloads += 1; 
       } 
     } 
     current = current->next ; 
   }

  return sockfd ; 

}


int statisticdets(struct clntdata *head){

  struct clntdata *current ;
  int count ;
  current = headConnectdNode;
  int flag = 0;
  double tup_rate = 0.00 ;
  double tdw_rate = 0.00 ;
  if(current == NULL){
   
   printf("No Connections exixts");
   printf("\n[PA1]$:");
   return 0 ;

  }
  
    printf("\n---------------------------------------------------------------STATISTICS----------------------------------------------------------------------\n");
    
    printf("\n\n"); 
  
  printf("%-35s%-20s%-10s%-20s%-20s%-20s%-20s\n","Hostname","IP Address","PORT NUM","Total Uploads","AvgUp Speed(Bps)","Total Downloads","AvgDown Speed(Bps)");

  while(current != NULL){
   
    if(current->iconnectionid != 1 ){
    
    tup_rate = current->uploadspeed; 
    tdw_rate = current->downloadspeed;
    
    if(current->nuploads != 0){
      tup_rate = tup_rate / current->nuploads ;
    }
    if(current->ndownloads != 0){
      tdw_rate = tdw_rate / current->ndownloads ;
    }
    printf("%-35s%-20s%-10s%-20d%-20f%-20d%-20f\n",current->chname,current->cipaddr,current->cportNum,current->nuploads,tup_rate,current->ndownloads,tdw_rate);
    
    flag = 1;
    }
    current = current->next ;
  }
  if(flag == 0){
    printf("\n -----------------------------------------------------------NO DATA TO DISPLAY---------------------------------");
    fflush(stdout);;
  }
  printf("\n"); 
  printf("------------------------------------------------------------------------------------------------------------------------------------------------\n");
  
  printf("\n[PA1]$:");
  fflush(stdout);
  
  return 1;

}

