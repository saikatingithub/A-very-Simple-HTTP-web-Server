#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<ctype.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/socket.h> // contains definations of structures needed for unix socket
#include<netinet/in.h> // contains definations of structrures needed for internet style sockets
#include<arpa/inet.h> // contains funtions modules needed to perform netwrok to system conversions

#define PORT 8080

int handel(int ,struct sockaddr_in *) ;
void send_msg(int ,char *);
int get_file_size(int);

void show(char *msg){
 int i =0 ;
 char ch;
 printf("\n\n***\n\n");
 ch = *(msg +i);
 while(ch !='\r'){
   printf("%c",ch);
   i++;
   ch =*(msg+i);
 }
}

void dis(char *m){
 int i =0;
 char ch =*(m+i);

 printf("\n\n URL\n");
 while(ch != '\0'){
  printf("%c",ch);
   i++;
   ch =*(m+i);
 }
}

int main(void){
 printf("\n\nSTARTING UP THE SERVER ............\n\n");
 int sockfd ,new_sockfd;
 struct sockaddr_in host_addr ,client_addr ;
 socklen_t address_size;
 int recv_len =1;
 int yes =1;
 int status;
 char buffer[1024];

 //creating a new socket: int socket(int domain,int type ,int protocol )
 if( (sockfd =socket(PF_INET,SOCK_STREAM,0)) ==-1)
   printf("\n \tcannot creat socket \n\t");
 printf("\n\nSERVER CREATING SOCKET...........DONE\n\n");
 //setting up a socket option
 if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) ==-1)
   printf("Failed to set up socket option");
 else{
//if i had successsfully created a socket then bind this socket to a address for connection;
   host_addr.sin_family =AF_INET;
   host_addr.sin_port =htons(PORT);
   host_addr.sin_addr.s_addr =0;
   memset(&(host_addr.sin_zero),'\0',8); //setting rest of the portion to \0
//binding the socket to host_addr so that the server can listen to the incomming connection;
  if((bind(sockfd,(struct sockaddr *)&host_addr,sizeof(struct sockaddr))) ==-1)
    printf("\n binding failed \n");
  printf("\n:SERVER READY TO ACCEPT CONNECTION:\n");
//setting process to listen to the socket
  if((listen(sockfd,5)) ==-1)
    printf("\nfailed to listening the socket\n");  
//now keep the server running for ever to recive and serve clients
  while(1){
    address_size =sizeof(struct sockaddr_in);
    new_sockfd =accept(sockfd,(struct sockaddr *)&client_addr,&address_size); //returns a new socket for the incomming connection
    if(new_sockfd ==-1)
      printf("\nERROR IN ACCEPTING CONNECTION FORM THE CLIENT\n");
    else{
      printf("\n CONNECTION RECIVED:%s ,%d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
      status = handel(new_sockfd ,&client_addr);
    // printf("in main stats =%d\n",status);
    /*
      if(status ==1)
        printf("\n client request successfully handeled \n");
      else
       printf("\nclient request could not be handeled\n");
      close(new_sockfd);
   */ 
   }
 }

 }
 return 0;
}

//defination of function handel()
int handel(int sfd ,struct sockaddr_in *c_addr){
 char request[500];
 char *buffer;
 char url[200];
 char ch;
 
 unsigned char *ptr ,*loc;
 int fd,length ,status,url_index;
  
 url_index =0; 

 status =recv(sfd,&request,sizeof(request),0);
 if(status ==-1)
  printf("Error while reciving client data\n");
 else{ //i.e we had successfully recived a client msg
  show(request);
  ptr =strstr(request,"HTTP");
  if(ptr == NULL)
    printf("\n Not a valid HTTP protocol \n");
  else{ //successfully recived client msg and we have a valid HTTP protocol
    ptr =NULL;
    if(strncmp(request,"GET ",4) ==0)
      ptr =request + 4; // ptr now pointing to the resource url
    if(strncmp(request,"HEAD ",5) ==0)
      ptr =request + 5; //ptr now pointing to the resource url
    if(ptr ==NULL)
      printf("\nInvalid Request Method \n");
    else{ //we have a valid request method and now locating the resource ,ptr pointing to the resource url
   //coping the url, from the recived client msg to url memory   
      loc =ptr;
      ch =*(loc); 
      while(!isspace(ch)){
       printf("\nch =%c",ch);
       if(url_index == 0){
        url[url_index] ='.';
        url_index ++;
        url[url_index] =ch;
        url_index ++;
       }
       else{
       url[url_index] =ch ;
       url_index ++;
       }
       loc++;
       ch =*(loc);
      }
      url[url_index] ='\0';
      dis(url);
   //url holds the address of the requested header;
   //opening the resource file 
      fd = open(url,O_RDONLY);
        if(fd ==-1){ //the resource file not found
          printf("\n file not found\n");
          send_string(sfd,"HTTP/1.0 404 NOT FOUND\r\n"); //responce status line
          send_string(sfd,"Server: My_Web_Server\r\n\r\n") ;//responce header line
          send_string(sfd,"<html><head><title>404 not found</title></head>");
          send_string(sfd,"<body><h1>Resource Not found</h1></body></html>\r\n");
        }
        else{ //requested file is found and now sending the contents;
          printf("\nfile found and now sending the file\n");
          send_string(sfd,"HTTP/1.0 200 OK\r\n"); //responce status line
          send_string(sfd,"Server: My_Web_Server\r\n\r\n") ;//responce header line
          
          if(ptr == (request+4)){ //if the request is a http get request
           length =get_file_size(fd);
           if(length == -1)
             printf("fatal in geting file size\n");
            else{
              buffer =(char *)malloc(length);
              if(buffer ==NULL)
               printf("Error while allocating memory\n");
              else{
                read(fd,buffer,length);
                if((send(sfd,buffer,length,0)) ==-1)
                 printf("Error while sending file\n"); //sending responce body;
                free(buffer);
              }
             close(fd);
             printf("\nData sent :\n");
            }
             
           } 
        }   
    } 
  }
  
 }
 shutdown(sfd,2);
 printf("\n Client request handeld successfully..waiting for further Request \n");
}

void send_string(int sfd,char *msg){
 int len =strlen(msg);
 int sts;
  sts =send(sfd,msg,len,0);
  if(sts ==-1)
   printf("\nUnable to send msg\n");
}

int get_file_size(int fd){
  struct stat stat_struct;
  if(fstat(fd,&stat_struct) ==-1)
   return -1;
  else 
    return (int)stat_struct.st_size;
}
