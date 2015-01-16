#ifndef GLOBAL_H_
#define GLOBAL_H_

#define HOSTNAME_LEN 128

#define MAX_PARALLEL_DOWNLOAD 3
#define REGISTER_CMD 3
#define CONNECT_CMD 4
#define UPLOAD_CMD 5
#define DOWNLOAD_CMD 6
#define HELP_CMD  7
#define MYIP_CMD  8
#define LIST_CMD  9


char ipaddress[128] ;
void gethIPaddress(char* buffer, size_t buflen);
void getClientWrapper(char *cport,int sockfd,char *ip);
//void getConnectDetails(int fd,char *data,struct clntdata **headRefNode,char *ip);
int peerValidation(char *ip,char *cport);
int fileProcess(int argc,char *argv,int connectionId);
int fileReceive(char *file_size,char *file_name,char*file_data,int sockfd,int bytesrcvd);
#endif
