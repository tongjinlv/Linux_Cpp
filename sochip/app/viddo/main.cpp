#include <string.h>
#include <getopt.h>           
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>        
#include <linux/videodev2.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <utils/Thread.h>
#include "vencoder.h"
#include "memoryAdapter.h"
#define dev_name "/etc/video0"
#define #define REQ_COUNT 6
int YUVimageSave(char *path,unsigned char *data,unsigned int length)
{
    FILE* fp = fopen(path, "wb");    
    if (fp == NULL)
    {
       printf("Can not write to file:%s\n", path);
       return -1;
    }
    fwrite(data, 1, length, fp);
    fflush(fp);
    fclose(fp);
    return 0;
}
unsigned int YUVimageGet(unsigned char *data)
{
    int fd;
    struct v4l2_capability cap;
    struct v4l2_fmtdesc fmtd;
    struct v4l2_format fmt2;
    struct v4l2_requestbuffers req;
    if((fd=open(dev_name,O_RDWR | O_NONBLOCK,0))<0)
    {
        printf("Error: open %s\n",dev_name);
        return 0;
    }
    else
    {
        printf("Open camera video0 success!!!! fd=%d\n", fd);
    } 
    memset(&fmtd, 0, sizeof(fmtd)); 
    fmtd.index = 0; 
    fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 
    while ((ioctl(fd, VIDIOC_ENUM_FMT, &fmtd)) == 0) 
    { 
        fmtd.index++; 
	printf("format =%c%c%c%c,description=%s\n", 
                           fmtd.pixelformat & 0xFF,
                    (fmtd.pixelformat >> 8) & 0xFF,
                   (fmtd.pixelformat >> 16) & 0xFF,
                   (fmtd.pixelformat >> 24) & 0xFF, 
                                 fmtd.description); 
    }
    if(ioctl(fd, VIDIOC_QUERYCAP, &cap)<0)
    {
        printf("Error: query cap\n");
        return 0;
    }
    printf("capabilities= 0x%08x\n",cap.capabilities);
    fmt2.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;  
    ioctl(fd,VIDIOC_G_FMT, &fmt2);
    fmt2.fmt.pix.width = 640;
    fmt2.fmt.pix.height = 480;
    fmt2.fmt.pix.pixelformat = V4L2_PIX_FMT_NV21;
    if(ioctl(fd, VIDIOC_S_FMT, &fmt2) < 0)
    {
        printf("set format failed\n");
        return 0;
    }
    req.count               = REQ_COUNT ;
    req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory              = V4L2_MEMORY_MMAP;
    if(ioctl(fd, VIDIOC_REQBUFS, &req)<0)
    {
        printf("Error:VIDIOC_REQBUFS\n");
        return;
    }
    printf("req.count=%d\n",req.count);
    return 0;
}
int main(int argc, char **argv)
{
    pthread_t captureTid;
    const char* captureFile = "Rear_Center.wav";
    const char* recordFile = "Record.wav";
  //if(!strcmp("mic", argv[1]))
}
