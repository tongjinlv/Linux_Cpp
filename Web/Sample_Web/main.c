#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXLINE 80

void deal_config(int *port_number,char *path);

int main(void)
{
 struct sockaddr_in servaddr,cliaddr;
 socklen_t cliaddr_len;
 int listenfd,connfd;
 char buf[MAXLINE];
 char str[INET_ADDRSTRLEN];
 int i,n;            //n descript the side number
 int pd_index,ret;
 
 int td;

 int port_number;
 char *path=NULL;

 listenfd=socket(AF_INET,SOCK_STREAM,0);

 int opt=1;
 setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

 
 bzero(&servaddr,sizeof(servaddr));
 servaddr.sin_family=AF_INET;

 deal_config(&port_number,path); 
 servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
 servaddr.sin_port=htons(port_number);

 bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
 
 listen(listenfd,20);

 printf("Accepting connections ...\n");
 while(1){ 
  printf("while");
  cliaddr_len=sizeof(cliaddr);
  connfd=accept(listenfd,(struct sockaddr *)&cliaddr,&cliaddr_len);
  while(1){
   n=read(connfd,buf,MAXLINE);
   if(n==0){
    printf("the other side has been closed.\n");
    break; 
   }
   //printf("received  File from %s at PORT %d\n",
     //           inet_ntop(AF_INET,&cliaddr.sin_addr,str,sizeof(str)),
       //                         ntohs(cliaddr.sin_port));
   i=0;
   char title[15];
   while(buf[i]!='\r'){
    title[i]=buf[i];
    i++;
   }
   title[i]='\0';
   if(0==(strcmp(title,"GET / HTTP/1.1"))){
    bzero(buf,MAXLINE); 
    strcpy(buf,"HTTP/1.1 200 0K\r\n");
    write(connfd,buf,strlen(buf));
    bzero(buf,MAXLINE);
    strcpy(buf,"Conent_Type: peizhi/html\r\n");
    write(connfd,buf,strlen(buf));
    bzero(buf,MAXLINE);
    buf[0]='\r';
    buf[1]='\n';
    write(connfd,buf,2);
    pd_index=open("./peizhi/index.html",O_RDONLY);
    if(pd_index<0){
     perror("open file failed.\n");
     exit(1);
    }
    bzero(buf,MAXLINE);
    while((ret=read(pd_index,buf,MAXLINE))>0){
     write(connfd,buf,ret);
     printf("send index data...\n");
     bzero(buf,MAXLINE);
    }
  // <SPAN style="WHITE-SPACE: pre"> </SPAN>close(pd_index);
   }
  } 
 } 
}
void deal_config(int *port_number,char *path)
{
 FILE *fd;
  
 fd=fopen("./peizhi/config.txt","r");
 if(fd<0){
  perror("open config failed.\n");
  exit(1);
 }
 fscanf(fd,"PORT=%d\nDirectory=%s",port_number,path);

}

