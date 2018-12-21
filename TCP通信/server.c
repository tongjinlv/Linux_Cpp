 #include<sys/types.h>
 #include<sys/socket.h>
 #include<netinet/in.h>
 #include<arpa/inet.h>
 #include<unistd.h>
 #include<stdio.h>
 #include<stdlib.h>
 #include<strings.h>
 #include<sys/wait.h>
 #include<string.h>
 #include "get_iplist.h"
/********************************************************************* 
 *filename: tcpserver.c 
 *purpose:tcp服务端程序 
 ********************************************************************/
 int main(void) 
 { 
     int sockfd,new_fd;
     struct sockaddr_in my_addr; /* 本机地址信息 */ 
     struct sockaddr_in their_addr; /* 客户地址信息 */ 
     unsigned int sin_size,lisnum,myport=18080; 
     if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) { 
         perror("Create socket fail!"); 
         exit(1); 
     } 
     printf("Socket use port=%d ok \n",myport);
     my_addr.sin_family=PF_INET; 
     my_addr.sin_port=htons(myport); 
     my_addr.sin_addr.s_addr = INADDR_ANY; 
     bzero(&(my_addr.sin_zero), 0); 
     if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) { 
         perror("bind"); 
         exit(1); 
     } 
     printf("bind ok \n");
     if (listen(sockfd, lisnum) == -1) { 
         perror("listen"); 
         exit(1); 
     }
     printf("listen ok \n");
     int step = 0;
     while(1) {
     char  szSnd[63] = {0};
     sprintf(szSnd,"i am server [%d]\n",step);
     step++;
         if (send(new_fd, szSnd, strlen(szSnd), 0) == -1) { 
             perror("send"); 
             close(new_fd); 
             break;
         }

  printf("send msg: %s \n",szSnd);
  sleep(1);
  }
  exit(0); 
 }
