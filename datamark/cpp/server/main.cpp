#include <iostream>
#include <tcp_server.h>
#include <stdlib.h>
int main(int argc,char* argv[])
{
        tcp_server ts(8081);
        ts.recv_msg();
        return 0;
}
