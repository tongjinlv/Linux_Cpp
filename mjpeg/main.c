#include <stdio.h>
 #include <sys/socket.h>
 #include <sys/types.h>
 #include <string.h>
 #include <netinet/in.h>
 #include <stdlib.h>
 #include <errno.h>
 #include <unistd.h>
 #include <arpa/inet.h>
 #include <sys/time.h>
 #include <pthread.h>
#define MAXLINE 1024
 

#define IO_BUFFER 256
#define BOUNDARY "boundarydonotcross"
#define STD_HEADER "Connection: close\r\n" \
    "Server: MJPG-Streamer/0.2\r\n" \
    "Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\n" \
    "Pragma: no-cache\r\n" \
    "Expires: Mon, 3 Jan 2000 12:34:56 GMT\r\n"

typedef struct {
    int level;              /* how full is the buffer */
    char buffer[IO_BUFFER]; /* the data */
} iobuffer;

char *getfile(char *name,int *l)
{
	FILE*fp;
	char *p; 
	int flen;
	fp=fopen(name,"rb");
	fseek(fp,0L,SEEK_END);   
	flen=ftell(fp);
    *l=flen;	
	p=(char *)malloc(flen+1);  
	if(p==NULL)  
	{  
	fclose(fp);  
	return 0;  
	}  
	fseek(fp,0L,SEEK_SET); 
	fread(p,flen,1,fp);  
	return p;
}
 void *client_thread(void *arg)
 {
	 int fd=*(int *)arg;
	 int n;
	  struct timeval timestamp;
	 char *file;
	 char *file1;
	int length,length1;
	 char buff[MAXLINE];
	 n = recv(fd,buff,MAXLINE,0);
	 buff[n] = '\0';
     printf("recv:%s",buff);
	 file=getfile("shot.jpg",&length);
	 file1=getfile("1.jpg",&length1);
     printf("length=%d\n",length);
	 char buffer[10240] = {0};
	 sprintf(buffer, "HTTP/1.0 200 OK\r\n" \
            STD_HEADER \
            "Content-Type: multipart/x-mixed-replace;boundary=" BOUNDARY "\r\n" \
            "\r\n" \
            "--" BOUNDARY "\r\n");

    if(write(fd, buffer, strlen(buffer)) < 0) {
		printf("error");
    }
int z=0;
   while(1)
   {
	gettimeofday(&timestamp,NULL);
	sprintf(buffer, "Content-Type: image/jpeg\r\n" \
                "Content-Length: %d\r\n" \
                "X-Timestamp: %d.%06d\r\n" \
                "\r\n", length, (int)timestamp.tv_sec, (int)timestamp.tv_usec);
    write(fd, buffer, strlen(buffer));
	write(fd, file, length);
    sprintf(buffer, "\r\n--" BOUNDARY "\r\n");
    write(fd, buffer, strlen(buffer));
	sleep(1);
	printf("writeok\r\n");
	fflush(stdout);
	
	gettimeofday(&timestamp,NULL);
	sprintf(buffer, "Content-Type: image/jpeg\r\n" \
                "Content-Length: %d\r\n" \
                "X-Timestamp: %d.%06d\r\n" \
                "\r\n", length1, (int)timestamp.tv_sec, (int)timestamp.tv_usec);
    write(fd, buffer, strlen(buffer));
	write(fd, file1, length1);
    sprintf(buffer, "\r\n--" BOUNDARY "\r\n");
    write(fd, buffer, strlen(buffer));
	sleep(1);
	printf("writeok%d---%d\r\n",fd,z++);
	fflush(stdout);
   }
	 close(fd);
 }
 int main(int argc,char **argv)
 {
	
    char *file;
	int on=1;
	struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_storage);
 int listenfd,connfd;
 struct sockaddr_in sockaddr;
 char buff[MAXLINE];
 int n;
 setbuf(stdout,NULL);
 memset(&sockaddr,0,sizeof(sockaddr));
 
 sockaddr.sin_family = AF_INET;
 sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
 sockaddr.sin_port = htons(15000);
 
 listenfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
 if(listenfd<0)return 0;
   if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
            perror("setsockopt(SO_REUSEADDR) failed");
        }

 bind(listenfd,(struct sockaddr *) &sockaddr,sizeof(sockaddr));

 listen(listenfd,1024);
 printf("Wait for the client information\n");
 int z=10;
 while(1)
 {
	 pthread_t client;
	 printf("zzzz");
     if((connfd = accept(listenfd,(struct sockaddr *)&client_addr, &addr_len))==-1)
     {
         printf("accpet socket error: %s errno :%d\n",strerror(errno),errno);
         continue;
     }
	 printf("create \r\n");
	 fflush(stdout);
	 pthread_create(&client, NULL, &client_thread,&connfd);
	 pthread_detach(client);
	 printf("over");
	 fflush(stdout);
 }
 printf("ok");
    close(listenfd);
	return 0; 
 }