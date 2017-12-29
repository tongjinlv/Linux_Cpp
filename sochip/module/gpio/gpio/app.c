#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
int main(int argc, const char *argv[])
{
    int fd;
    unsigned char ledbuf[2];
    fd = open("/dev/myled", O_RDWR);
    printf("iFd=%d\r\n",fd);
    ioctl(fd, 0x30);
    ledbuf[0]=0;
    ledbuf[1]=1;
    int ret=read(fd,ledbuf, 0);
    while(1)
    {
         ledbuf[0]=0;
         ledbuf[1]=1;
         write(fd,ledbuf,2);
         usleep(1);
         ledbuf[0]=0;
         ledbuf[1]=0;
         write(fd,ledbuf,2);
         usleep(1);
    }
    printf("ret=%d\r\n",ret);
    return 0;
}
