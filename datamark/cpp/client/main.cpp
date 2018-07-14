#include <iostream>
#include <tcp_client.h>
#include <stdlib.h>
using namespace std;
 
int main(int argc,char* argv[])
{
        tcp_client tc(argv[1],argv[2]);
        tc.tcp_write(argv[3]);
        tc.tcp_close();
        return 0;
}

