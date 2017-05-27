#include <wiringSerial.h>
#include <stdio.h>
#include<unistd.h>
#include <signal.h> 
void app_close(int tags)
{
	
        printf("clonse");
}
int main(void)
{
    int fd;
    char UartBuff[100];
    signal(SIGINT, app_close); 
    if((fd = serialOpen("/dev/ttyUSB0",115200))<0)	
    {
        printf("serial err\n");
    }
	printf("dddddd");
    while(1)
    {
	 UartBuff[0]=serialGetchar(fd);
	serialPutchar(fd,'a');
        if(UartBuff[0]=='a')
        {
	        serialPutchar(fd,UartBuff[0]);
		printf("revice=%c",UartBuff[0]);
        }
        sleep(0.5);
    }
	return 0;
}
