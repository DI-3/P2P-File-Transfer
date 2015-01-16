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


extern struct clntdata *headConnectdNode;
char ips[INET_ADDRSTRLEN]; 
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
extern char ips[INET_ADDRSTRLEN]; 

/* This function displays the details of the upload and download */
void displayDownloadDetails(double timedelay,char* filename,int sockfd,double tf_rate,int file_size){

struct clntdata *current;
current = headConnectdNode ;

double  tx_time = 0.00;
   while(current!= NULL){

      if(current->sockdscrptr == sockfd){
   
        printf("\n File %s has been downloaded by %s ",filename,current->chname);
        printf("\n Rx: %s -> %s ,File Size :%d Bytes, Time taken :%lf, Rx Rate : %lf bits/second",ips,current->cipaddr,file_size,(timedelay/1000000),tf_rate); 
        
      }
     current = current->next ; 
   }

}

/* This function displays the details of the upload and download */
void displayUploadDetails(double timedelay,char* filename,int connectionId,double tf_rate,int file_size){

struct clntdata *current;
current = headConnectdNode ;
int sockfd= 0 ;

   while(current!= NULL){

      if(current->iconnectionid == connectionId){
   
        printf("\n File %s has been downloaded by %s ",filename,current->chname);
        printf("\n Tx: %s -> %s ,File Size :%d Bytes, Time taken :%lf, Tx Rate : %lf bits/second",ips,current->cipaddr,file_size,(timedelay/1000000),tf_rate); 
        printf("\n[PA1]$:");
        //fflush(stdout); 
      }
     current = current->next ; 
   }
 

}

/* Help command call this method                                                   */
void printHelp(){


printf("\n------------------------------------------COMMANDS  INFORMATION----------------------------------------------");
printf("\n");
printf("\n       1. CREATOR   :Prints the creator information");
printf("\n       2  MYIP      :Prints the IP Address of the system");
printf("\n       3. MYPORT    :Prints thelistening port ");
printf("\n       4. REGISTER  :Command for registering the clients to server");
printf("\n                        REGISTER <IPADDRESS> <PORT#>");
printf("\n       5. CONNECT   :Command to connect to a peer");
printf("\n                        CONNECT <IPADDRESS> <PORT#>");
printf("\n       6. LIST      :Command to list the active connections");
printf("\n       7. TERMINATE :Command to terminate a connection");
printf("\n                        TERMINATE <CONNECTION ID>");
printf("\n       8. EXIT      :Command to exit ");
printf("\n       9. UPLOAD    :Command to upload a file");
printf("\n                        UPLOAD <CONNECTION ID> <FILENAME/FILEPATH>");
printf("\n      10.DOWNLOAD  :Command to download a file");
printf("\n                        DOWNLOAD <CONNECTION ID1> <FILENAME/FILEPATH> <CONNECTION ID2> <FILENAME/FILEPATH> ");
printf("\n                        You can download from no more than 3 peers!");
printf("\n      11.STATISTICS :Command to show the statistics");
printf("\n"); 
printf("\n-------------------------------------------------------------------------------------------------------------\n");   


}

/* Creator command calls this method                                                 */
void printCreator(){


printf("\n----------------------------------CREATOR---------------------------------------------------\n");
printf("\n     mithunna@buffalo.edu");
printf("\n     I have read and understood the course academic integrity policy located at ");
printf("\n     http://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f14/index.html#integrity\n");
printf("\n------------------------------------------------------------------------------------------------\n ");

}
/* This method builds the string format of the statistial details for the server      */
void buildCStatistics(){

struct clntdata *current;
current = headConnectdNode ;
double  tx_time = 0.00;
char *recdelim = "@";
char *fielddelim = "#";
char buffer[300];
int i,j;
char *data ;
 
 j=  sprintf(buffer,"%s","STAT");
 j+= sprintf(buffer+j,"%s",recdelim);


   while(current!= NULL){
      
      //if(current->nuploads != 0 || current->ndownloads != 0){

        j+= sprintf(buffer+j,"%s",current->chname);
        j+= sprintf(buffer+j,"%s",current->cportNum);
        j+= sprintf(buffer+j,"%s",recdelim);
        j+= sprintf(buffer+j,"%d",current->nuploads);
        j+= sprintf(buffer+j,"%s",recdelim);
        j+= sprintf(buffer+j,"%f",current->uploadspeed);
        j+= sprintf(buffer+j,"%s",recdelim);
        j+= sprintf(buffer+j,"%d",current->ndownloads);
        j+= sprintf(buffer+j,"%s",recdelim);
        j+= sprintf(buffer+j,"%f",current->downloadspeed);
        j+= sprintf(buffer+j,"%s",fielddelim);
     // }
     current = current->next ; 
   }
     printf("\n The statistic data for server is %s",buffer);
     fflush(stdout);
     printf("\n[PA1]$:");
}
  
/* This method deflates the string format of the statistial details for the server      */
 
 void deflateStatistics(char *data,int fd){
  
   //get the ip or hostname associated with socket fd
   //get hostnamed
 	  char *recdelim ="@";
 	  char *fieldelim ="#";
    char *datarr[50];
    int oIndex ,iIndex;
    char *recarg;
    char *arg;
 	  char *data1 ="local@rec1#rec2#rec3#rec4@dec1#dec2#dec3#@";
    char *token[5];
    char  buffer[1000] ; 
    arg = strtok(data,recdelim);
    oIndex = 0;
    iIndex = 0;
    while(arg){

    int strreclen = strlen(arg);
    datarr[oIndex] = (char *)malloc((strreclen + 1)* sizeof(char));
    strcpy(datarr[oIndex],arg);    
    oIndex++ ;
    arg =strtok(NULL,recdelim);
   } 
    
    int iLoop = 0;
    
    for(iLoop = 0 ;iLoop < oIndex; iLoop++){

     recarg = strtok(datarr[iLoop],fieldelim) ;
     iIndex = 0 ;
     while(recarg){
    

      token[iIndex] = strdup(recarg);
       if(iIndex == 5){

        //createStringForDisplay(fd,token[1],token[2],token[3],token[4],token[5]);
        
       }
      
      iIndex++ ;
   
     recarg = strtok(NULL,fieldelim);

     }


    }
    
    
 } 