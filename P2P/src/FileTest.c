#define BUFFSIZE 64*1024 -2
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "global.h"
#include <sys/time.h>
#include "../include/ProcessCommands.h"
#include <math.h>

int fileProcess(int argc,char *argv,int connectionId){

char buffer[300];
char source[30] = "sample.txt";
char target[30];
char tempbuffer[350]="\0";
int s_handle ;
FILE *fp;
int file_size;
unsigned int numbytes ;
struct stat fdata ;
char cfile_size[32] = "\0";
struct timeval tx_start,tx_end ; 
double tf_rate = 0.00;


fp = fopen(argv,"r+");
if(fp ==NULL){

  printf("\n[PA1]$:File Error .Trying to request an invalid file.");
  return 0 ;

}else{

fseek(fp,0,SEEK_END);
file_size = ftell(fp);
fseek(fp,0,SEEK_SET);
fclose(fp);
}


int count = 0 ;
int sendbytes = 0;

if((s_handle = open(argv,O_RDONLY ,0)) == -1)
{

	printf("\n[PA1]$:Invalid file:Cannot open the file");
	return ;
}


sprintf(cfile_size,"%d",file_size);
strcat(tempbuffer,"UP");
strcat(tempbuffer,"@@@");
strcat(tempbuffer,cfile_size);
strcat(tempbuffer,"@@@");
strcat(tempbuffer,argv);
strcat(tempbuffer,"@@@");
int headerlen = strlen(tempbuffer);


int total_size = headerlen + file_size ;
memset(tempbuffer,0,sizeof tempbuffer);
memset(buffer,0,sizeof buffer);
sprintf(cfile_size,"%d",total_size);
strcat(tempbuffer,"UP");
strcat(tempbuffer,"@@@");
strcat(tempbuffer,cfile_size);
strcat(tempbuffer,"@@@");
strcat(tempbuffer,argv);
strcat(tempbuffer,"@@@");

// Referred : http://www.cs.loyola.edu/~jglenn/702/S2008/Projects/P3/time.html for timeofday function
/* Calculating the transmission rate  */
printf("\n -----------------------Uploading  file %s :",argv);

gettimeofday(&tx_start,NULL);

while(numbytes = read(s_handle,buffer,sizeof buffer)){
   
   //printf("The number of bytes read %d",numbytes);

   if(numbytes == -1){
    break;
   }
   if(count == 0){
   	 
   	 //strcat(tempbuffer,buffer);
     memcpy(tempbuffer + headerlen,buffer,numbytes);
    // tempbuffer[headerlen + strlen(buffer)-3]='\0';
     if((sendbytes = send(argc,tempbuffer,numbytes + headerlen,0)) == -1){
   	   perror("sendERRR");
       return 0 ;
     }
    }else{
     
     if((sendbytes = send(argc,buffer,numbytes,0)) == -1){
   	 perror("sendERRR");
     return 0 ;
     }
    }
     memset(buffer,'\0',sizeof buffer);
     count ++ ;
   }
	
 gettimeofday(&tx_end,NULL);
 
 double timedelay =  (tx_end.tv_sec * 1000000 + tx_end.tv_usec)-(tx_start.tv_sec * 1000000 + tx_start.tv_usec) ;
 printf("\n Finished uploading:time taken %lf",timedelay/1000000); 
 close(s_handle);
 
 tf_rate =  ((file_size * 8)/timedelay) * 1000000 ;
 printf("\n Transmission Rate %f",tf_rate); 
 int opt = 1 ;
 updateTransmissionDetails(connectionId,tf_rate,opt);
 displayUploadDetails(timedelay,basename(argv),connectionId,tf_rate,file_size);
 return 1 ;
} 

/* This method receives the files in chunks and creates the file in the remote end */

int fileReceive(char *file_size,char *file_name,char*file_data,int sockfd,int bytes_rcvd){


int fsize = atoi(file_size);
int datalen =0 ;

int t_handle ;
int bytes_to_read = datalen;
int bytes_written = 0;
char in_buffer[350];
int iter = 0 ;
int bytes_read_now = datalen ;
int metadatalen = 0;
double tf_rate = 0.00;
struct timeval rx_start,rx_end ;


metadatalen = strlen(file_size) + strlen(file_name) + 11 ;
memset(in_buffer,0,sizeof in_buffer);
memmove(in_buffer,file_data,sizeof in_buffer);



// referred  to get the file name from the path name
//http://stackoverflow.com/questions/3288006/are-there-any-c-apis-to-extract-the-base-file-name-from-its-full-path-in-linux

if((t_handle = open(basename(file_name),O_CREAT | O_WRONLY ,S_IWRITE)) == -1){


	printf("[PA1]$:Cannot create a file :Duplicate file or Invalid File\n");
	return ;
}
 
fchmod(t_handle,0666);  

printf("\n Uploading of the file has started") ;
gettimeofday(&rx_start,NULL);

if((bytes_written = write(t_handle,file_data,(bytes_rcvd - metadatalen))) == -1){
     printf("\n [PA1]$:Error writing to  the file");
     return ;
   
 } 
   //bytes_read_now = bytes_written ;

bytes_to_read = fsize - (bytes_rcvd + metadatalen) ; 

//printf("\n THe files size :%d , the dtata length :%d pending data:%d",fsize,datalen,bytes_to_read);

while(bytes_to_read > 0){

      
    if((bytes_read_now = recv(sockfd,in_buffer,sizeof in_buffer,0)) <=0){

       if(bytes_read_now == 0){
          
          printf("\n File Download complete");
          break ;

       }else{

        perror("Receive read for file");
        break ;

       }  
         

     }else{ 
            
           if((bytes_written = write(t_handle,in_buffer,bytes_read_now))== -1){
             printf("\n Error writing to  the file");
            } 
     }

   memset(in_buffer,'\0',sizeof in_buffer);
   
   bytes_to_read = bytes_to_read - bytes_read_now ;
   
   
   
   iter++;

 }
  

  close(t_handle);
  
  gettimeofday(&rx_end,NULL);
  
  double timedelay =  (rx_end.tv_sec * 1000000 + rx_end.tv_usec)-(rx_start.tv_sec * 1000000 + rx_start.tv_usec) ;
  printf("\n Finished downloading :time taken %lf sec",timedelay/1000000);  

  tf_rate =  (((fsize - metadatalen) * 8)/timedelay )* 1000000;

  int option = 2 ;
  updateTransmissionDetails(sockfd,tf_rate,option);
  displayDownloadDetails(timedelay,basename(file_name),sockfd,tf_rate,fsize - metadatalen);
  return ;

}







