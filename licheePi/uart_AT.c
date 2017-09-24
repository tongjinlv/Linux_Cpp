#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
 
#define BAUDRATE B115200 
#define DEVICE "/dev/ttyS2"
#define SIZE 1024
 
int nFd = 0;
struct termios stNew;
struct termios stOld;
int SerialInit(void);

int main(int argc, char **argv)
{
    int nRet = 0,i;
    char buf[SIZE];
	if(argc<2)perror("Null Commend");
    if( SerialInit() == -1 )
    {
        perror("SerialInit Error!\n");
        return -1;
    }
    bzero(buf, SIZE);
	
	for(i=0;i<strlen(argv[1]);i++)
	{
		buf[i]=argv[1][i];
	}
	buf[i++]='\r';
	buf[i++]='\n';
	buf[i++]=0;
	printf(buf);
	write(nFd,buf,i-1);
	bzero(buf, SIZE);
	nRet = read(nFd, buf, SIZE);
	if(0 < nRet)
	{
		buf[nRet] = 0;
		printf("Recv Data: %s\n", buf);
	}
	if(-1 == nRet)
	{
		perror("Read Data Error!\n");
	}
    close(nFd);
    return 0;
}

int SerialInit()
{
    nFd = open(DEVICE, O_RDWR|O_NOCTTY|O_NDELAY);
    if(-1 == nFd)
    {
        perror("Open Serial Port Error!\n");
        return -1;
    }
    if( (fcntl(nFd, F_SETFL, 0)) < 0 )
    {
        perror("Fcntl F_SETFL Error!\n");
        return -1;
    }
    if(tcgetattr(nFd, &stOld) != 0)
    {
        perror("tcgetattr error!\n");
        return -1;
    }
 
    stNew = stOld;
    cfmakeraw(&stNew);
 
    //set speed
    cfsetispeed(&stNew, BAUDRATE);//115200
    cfsetospeed(&stNew, BAUDRATE);
 
    //set databits
    stNew.c_cflag |= (CLOCAL|CREAD);
    stNew.c_cflag &= ~CSIZE;
    stNew.c_cflag |= CS8;
 
    //set parity
    stNew.c_cflag &= ~PARENB;
    stNew.c_iflag &= ~INPCK;
 
    //set stopbits
    stNew.c_cflag &= ~CSTOPB;
    stNew.c_cc[VTIME]=0;    //指定所要读取字符的最小数量
    stNew.c_cc[VMIN]=1; //指定读取第一个字符的等待时间，时间的单位为n*100ms
                //如果设置VTIME=0，则无字符输入时read（）操作无限期的阻塞
    tcflush(nFd,TCIFLUSH);  //清空终端未完成的输入/输出请求及数据。
    if( tcsetattr(nFd,TCSANOW,&stNew) != 0 )
    {
        perror("tcsetattr Error!\n");
        return -1;
    }
 
    return nFd;
}
 