#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
 
class tcp_client
{
private:
        int socket_fd;
        char message[4096];
        struct sockaddr_in server_addr;
 
public:
        tcp_client(char* server_ip,char* server_port);
        int tcp_write(char *text);
        void tcp_close(void);
};

