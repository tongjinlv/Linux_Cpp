#include "tcp_client.h"
#include <string.h>
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <cstdlib>
tcp_client::tcp_client(char* server_ip,char* server_port)
{
       if( (socket_fd = socket(AF_INET,SOCK_STREAM,0)) < 0 ) {
                printf("create socket error: %s(errno:%d)\n)",strerror(errno),errno);
                exit(0);
        }
 
        memset(&server_addr,0,sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(atoi(server_port));
 
        if( inet_pton(AF_INET,server_ip,&server_addr.sin_addr) <=0 ) {
                printf("inet_pton error for %s\n",server_ip);
                exit(0);
        }
 
        if( connect(socket_fd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0) {
                printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
                exit(0);
        }
        
}
void tcp_client::tcp_close(void)
{
    close(socket_fd);
    exit(0);
}
int tcp_client::tcp_write(char *text)
{
    char buf[1000];
    sprintf(buf,"POST /php/api.php?id=QuysUOh HTTP/1.1\nContent-Type: application/x-www-form-urlencoded\nHost: trtos.com\nContent-Length: %ld\nExpect: 100-continue\nConnection: Keep-Alive\n\n\n%s",strlen(text),text);
    if( send( socket_fd,buf,strlen(buf),0 ) < 0 ) {
            printf("send message error\n");
            exit(0);
    }
    char buffer[1000];
    memset(buffer,0,1000);
    if( ( read(socket_fd,buffer,1000)) < 0 ) {
            throw("Read() error!");
    } else {
            if(strstr(buffer,"HTTP/1.1 100 Continue"))printf("OK\n");
            else printf("ERROR");
    }
}

