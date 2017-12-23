#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>           
#include <fcntl.h>         
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>        
#include <linux/videodev2.h>
#include <pthread.h>
#define REQ_COUNT 6
#define uchar unsigned char
typedef struct Image 
{
	unsigned int width;
	unsigned int height;
	unsigned int channel;
	unsigned int size;
	unsigned char* data; 
}Image;
Image *Cameraimge;
Image *Yimge;
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
void NV21_T_RGB(unsigned int width , unsigned int height , unsigned char *yuyv , unsigned char *rgb)
{
    const int nv_start = width * height ;
    unsigned int i, j, index = 0, rgb_index = 0;
    unsigned char y, u, v;
    int r, g, b, nv_index = 0;
    for(i = 0; i <  height ; i++)
    {
		for(j = 0; j < width; j ++){
			nv_index = (rgb_index / 2 - width / 2 * ((i + 1) / 2)) * 2;
			//nv_index = i / 2  * width + j - j % 2;
			y = yuyv[rgb_index];
			u = yuyv[nv_start + nv_index ];
			v = yuyv[nv_start + nv_index + 1];
			r = y + (140 * (v-128))/100;  //r
			g = y - (34 * (u-128))/100 - (71 * (v-128))/100; //g
			b = y + (177 * (u-128))/100; //b
				
			if(r > 255)   r = 255;
			if(g > 255)   g = 255;
			if(b > 255)   b = 255;
       		if(r < 0)     r = 0;
			if(g < 0)     g = 0;
			if(b < 0)     b = 0;
			
			index = rgb_index % width + (height - i - 1) * width;
			rgb[index * 3+0] = b;
			rgb[index * 3+1] = g;
			rgb[index * 3+2] = r;
			rgb_index++;
		}
    }
}

void SaveImage2bmp(const char * path, Image* img)
{
	printf("SaveImge to %s \n" , path );
	FILE* fp = fopen(path, "wb");
	unsigned char bfType[2] = { 'B', 'M' };
	unsigned int size, headsize, i, appedsize;
	unsigned short shortdata, shortdata1;
	unsigned char* cp = bfType, *data;
	unsigned char append[4] = { 0, 0, 0, 0 };
	if (fp == NULL)
	{
		printf("Can not write to file:%s\n", path);
		return;
	}
	fwrite(bfType, 1, 2, fp);
	size = img->width * img->channel * img->height;
	if (img->channel == 1)
	{
		headsize = 1078;
		shortdata = 8;
	}
	else if (img->channel == 3)  //RGB24
	{
		headsize = 54;
		shortdata = 24;
	}
	else if (img->channel == 4)   //RGB32
	{
		headsize = 54;
		shortdata = 32;
	}
	else
	{
		fclose(fp);
		remove(path);
	}
	size += headsize;
	fwrite(&size, 4, 1, fp);
	fwrite(append, 1, 4, fp);
	fwrite(&headsize, 4, 1, fp);
	size = 40;
	fwrite(&size, 4, 1, fp);
	size = img->width;
	fwrite(&size, 4, 1, fp);
	size = img->height;
	fwrite(&size, 4, 1, fp);
	shortdata1 = 1;
	fwrite(&shortdata1, 2, 1, fp);
	fwrite(&shortdata, 2, 1, fp);
	size = 0;
	fwrite(&size, 4, 1, fp);
	fwrite(&size, 4, 1, fp);
	fwrite(&size, 4, 1, fp);
	fwrite(&size, 4, 1, fp);
	fwrite(&size, 4, 1, fp);
	fwrite(&size, 4, 1, fp);
	if (img->channel == 1) 
	{
		for (size = 0; size < 256; size++) 
		{
			*cp = size;
			fwrite(cp, 1, 1, fp);
			fwrite(cp, 1, 1, fp);
			fwrite(cp, 1, 1, fp);
			*cp = 0;
			fwrite(cp, 1, 1, fp);
		}
	}

	data = img->data;
	appedsize = (((img->width + 3) >> 2) << 2) - img->width;
	for (i = 0; i < img->height; i++) 
	{
		fwrite(data, 1, img->width, fp);
		data += img->width;
		fwrite(append, 1, appedsize, fp);
	}

	fflush( fp );
	fclose(fp);
}

struct buffer {
	void *                  start;Image *Cameraimge;
	Image *Yimge;
	size_t                  length;
};

static char *           dev_name        = "/dev/video0"; 
static int              fd              = -1;
struct buffer *         buffers         = NULL;
volatile int iFlagCamera = 0;


FILE *file_fd;
int i;
int r;

struct v4l2_capability cap;
struct v4l2_format fmt;
struct v4l2_buffer buf[REQ_COUNT];
struct v4l2_requestbuffers req;
struct v4l2_buffer tmp_buf;
enum v4l2_buf_type type;

struct timeval tv;
struct timeval tp1;	
struct timeval tp3;
struct timeval tp4;

fd_set fds;

#define V4L2_CAP_VIDEO_CAPTURE		0x00000001  /* Is a video capture device */
#define V4L2_CAP_VIDEO_OUTPUT		0x00000002  /* Is a video output device */
#define V4L2_CAP_VIDEO_OVERLAY		0x00000004  /* Can do video overlay */
#define V4L2_CAP_VBI_CAPTURE		0x00000010  /* Is a raw VBI capture device */
#define V4L2_CAP_VBI_OUTPUT		0x00000020  /* Is a raw VBI output device */
#define V4L2_CAP_SLICED_VBI_CAPTURE	0x00000040  /* Is a sliced VBI capture device */
#define V4L2_CAP_SLICED_VBI_OUTPUT	0x00000080  /* Is a sliced VBI output device */
#define V4L2_CAP_RDS_CAPTURE		0x00000100  /* RDS data capture */
#define V4L2_CAP_VIDEO_OUTPUT_OVERLAY	0x00000200  /* Can do video output overlay */
#define V4L2_CAP_HW_FREQ_SEEK		0x00000400  /* Can do hardware frequency seek  */
#define V4L2_CAP_RDS_OUTPUT		0x00000800  /* Is an RDS encoder */

/* Is a video capture device that supports multiplanar formats */
#define V4L2_CAP_VIDEO_CAPTURE_MPLANE	0x00001000
/* Is a video output device that supports multiplanar formats */
#define V4L2_CAP_VIDEO_OUTPUT_MPLANE	0x00002000

#define V4L2_CAP_TUNER			0x00010000  /* has a tuner */
#define V4L2_CAP_AUDIO			0x00020000  /* has audio support */
#define V4L2_CAP_RADIO			0x00040000  /* is a radio device */
#define V4L2_CAP_MODULATOR		0x00080000  /* has a modulator */

#define V4L2_CAP_READWRITE              0x01000000  /* read/write systemcalls */
#define V4L2_CAP_ASYNCIO                0x02000000  /* async I/O */
#define V4L2_CAP_STREAMING              0x04000000  /* streaming I/O ioctls */

#define V4L2_MBUS_FMT_YUYV8_2X8          0x2008


void getCameraCapabilities(int iCameraCapabilities)
{
	if(iCameraCapabilities&V4L2_CAP_VIDEO_CAPTURE)
	{
		printf("!!!!**** camera ****!!!! Is a video capture device\n");
	}

	if(iCameraCapabilities&V4L2_CAP_VIDEO_OUTPUT)
	{
		printf("**** camera **** Is a video output device\n");
	}

	if(iCameraCapabilities&V4L2_CAP_VIDEO_OVERLAY)
	{
		printf("**** camera **** Can do video overlay\n");
	}

	if(iCameraCapabilities&V4L2_CAP_VBI_CAPTURE)
	{
		printf("**** camera **** Is a raw VBI capture device\n");
	}

	if(iCameraCapabilities&V4L2_CAP_VBI_OUTPUT)
	{
		printf("**** camera **** Is a raw VBI output device\n");
	}

	if(iCameraCapabilities&V4L2_CAP_SLICED_VBI_CAPTURE)
	{
		printf("**** camera **** Is a sliced VBI capture device\n");
	}

	if(iCameraCapabilities&V4L2_CAP_SLICED_VBI_OUTPUT)
	{
		printf("**** camera **** Is a sliced VBI output device\n");
	}

	if(iCameraCapabilities&V4L2_CAP_RDS_CAPTURE)
	{
		printf("**** camera **** RDS data capture\n");
	}

	if(iCameraCapabilities&V4L2_CAP_VIDEO_OUTPUT_OVERLAY)
	{
		printf("**** camera **** Can do video output overlay\n");
	}

	if(iCameraCapabilities&V4L2_CAP_HW_FREQ_SEEK)
	{
		printf("**** camera **** Can do hardware frequency seek\n");
	}

	if(iCameraCapabilities&V4L2_CAP_RDS_OUTPUT)
	{
		printf("**** camera **** Is an RDS encoder\n");
	}

	if(iCameraCapabilities&V4L2_CAP_VIDEO_CAPTURE_MPLANE)
	{
		printf("**** camera **** Is a video capture device that supports multiplanar formats\n");
	}

	if(iCameraCapabilities&V4L2_CAP_VIDEO_OUTPUT_MPLANE)
	{
		printf("**** camera **** Is a video output device that supports multiplanar formats\n");
	}

	if(iCameraCapabilities&V4L2_CAP_TUNER)
	{
		printf("**** camera **** has a tuner\n");
	}

	if(iCameraCapabilities&V4L2_CAP_AUDIO)
	{
		printf("**** camera **** has audio support\n");
	}

	if(iCameraCapabilities&V4L2_CAP_RADIO)
	{
		printf("**** camera **** is a radio device\n");
	}

	if(iCameraCapabilities&V4L2_CAP_MODULATOR)
	{
		printf("**** camera **** has a modulator\n");
	}

	if(iCameraCapabilities&V4L2_CAP_READWRITE)
	{
		printf("**** camera **** read/write systemcalls\n");
	}

	if(iCameraCapabilities&V4L2_CAP_ASYNCIO)
	{
		printf("**** camera **** async I/O\n");
	}

	if(iCameraCapabilities&V4L2_CAP_STREAMING)
	{
		printf("!!!!**** camera ****!!!! streaming I/O ioctls\n");
	}
}
/*  Four-character-code (FOURCC) */
#define v4l2_fourcc(a, b, c, d)\
((__u32)(a) | ((__u32)(b) << 8) | ((__u32)(c) << 16) | ((__u32)(d) << 24))
#define V4L2_PIX_FMT_JZ420B  v4l2_fourcc('J', 'Z', '1', '2') /* 12  YUV 4:2:0 B   */

// #include "imgprocess.h"

#include <sys/time.h>

// int main ()
void thread2(void)
{

	int iCounterCamera = 0;
	int iCounter100frame = 0;

	if((fd=open(dev_name,O_RDWR | O_NONBLOCK,0))<0)
	{
		printf("Error   66666: open %s\n",dev_name);
		return;
	}
	else
	{
		printf("****  Open camera video0 success!!!! fd=%d\n", fd);
	}
	struct v4l2_fmtdesc fmtd; 
	int ret = 0;
	memset(&fmtd, 0, sizeof(fmtd)); 
	fmtd.index = 0; 
	fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; 

	while ((ret = ioctl(fd, VIDIOC_ENUM_FMT, &fmtd)) == 0) 
	{ 
		fmtd.index++; 
		printf("\n...... pixelformat = ''%c%c%c%c'', description = ''%s'' \n", fmtd.pixelformat & 0xFF, (fmtd.pixelformat >> 8) & 0xFF, (fmtd.pixelformat >> 16) & 0xFF, (fmtd.pixelformat >> 24) & 0xFF, fmtd.description); 
	}
	if(ioctl(fd, VIDIOC_QUERYCAP, &cap)<0)
	{
		printf("Error: query cap\n");
		return;
	}
	printf("\n capabilities= 0x%08x\n",cap.capabilities);
	int iCapCapabilities=cap.capabilities;
	int index = 0;
	if(ioctl(fd, VIDIOC_S_INPUT, &index)<0)
	{
		printf("Error:VIDIOC_S_INPUT \n");
		return;
	}
	struct v4l2_format fmt2;
	fmt2.type=V4L2_BUF_TYPE_VIDEO_CAPTURE; 
	ret = ioctl(fd,VIDIOC_G_FMT, &fmt2);
	printf("VIDIOC_G_FMT ret=%d \n", ret);
        printf("VIDIOC_G_FMT fmt.fmt.pix.width=%d \n", fmt2.fmt.pix.width);
        printf("VIDIOC_G_FMT fmt.fmt.pix.height=%d \n", fmt2.fmt.pix.height);
	fmt2.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt2.fmt.pix.pixelformat =V4L2_PIX_FMT_YUV422P;
	if (ioctl(fd, VIDIOC_S_FMT, &fmt2) < 0)
	{
		printf("set format failed\n");
		return ;
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

	buffers = calloc (req.count, sizeof (*buffers));
	printf(" chenmuyuan767676767sizeof (*buffers) =%08x\n", sizeof (*buffers));

	for(i=0;i<req.count;i++)
	{
		buf[i].type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf[i].memory      = V4L2_MEMORY_MMAP;
		buf[i].index       = i;
		printf("**chenmuyuan 1**  buf.length=%d \n",buf[i].length);

		if(ioctl(fd, VIDIOC_QUERYBUF, buf+i)<0)
		{
			printf("Error:VIDIOC_QUERYBUF\n");
			return;
		}


		printf("**cmy 2** i=%d buf.length=%d, \n",buf[i].length);

		buffers[i].length =buf[i].length;//14400
		buffers[i].start =
			mmap (NULL /* start anywhere */,   
				  buf[i].length,
				  PROT_READ | PROT_WRITE /* required */,
				  MAP_SHARED /* recommended */,
				  fd, buf[i].m.offset);
		printf("**** i=%d buf.length=%d, buffers[i].start=%08x buf.m.offset=%08x\n", i, buf[i].length, *( (unsigned int *) (buffers[i].start) ), buf[i].m.offset);


		buf[i].type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf[i].memory      = V4L2_MEMORY_MMAP;
		buf[i].index       = i;

		if(ioctl(fd, VIDIOC_QBUF, buf+i)<0)
		{
			printf("Error: VIDIOC_QBUF\n");
		}
	}

	type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(fd, VIDIOC_STREAMON, &type)<0)
	{
		printf("Error: VIDIOC_STREAMON\n");
		return;
	}
	
	gettimeofday(&tv, NULL);
	int iSecond = tv.tv_sec;
	int iuSecond = tv.tv_usec;
	int iCostTime = 0;
	
	int iSecond3 = tv.tv_sec;
	int iuSecond3 = tv.tv_usec;
	int iCostTime3 = 0;
	
	int iSecond4 = tv.tv_sec;
	int iuSecond4 = tv.tv_usec;
	int iCostTime4 = 0;
	
	int iCameraCounter = 0;
	
	int iFrameCounter = 0;
	int iFrameCounter3 = 0;
	int iFrameCounter4 = 0;
        iFlagCamera=3;
	while(1)
	{
		if(iFlagCamera>1000)
		{
			break;
		}

		iFrameCounter++;
		
		if(iFrameCounter%100==0)
		{
		 	gettimeofday(&tp1, NULL);
		 	
		 	iCostTime = (tp1.tv_sec - iSecond)*1000000 + tp1.tv_usec - iuSecond;
		 	printf("@@@@ 100 frame=%u us, fps=%f\n",iCostTime, 1000000*100.0/iCostTime) ;
		 	
		 	iSecond = tp1.tv_sec;
		 	iuSecond = tp1.tv_usec;
		}

		FD_ZERO (&fds);
		FD_SET (fd, &fds);
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		r = select (fd + 1, &fds, NULL, NULL, &tv);
		if(r<0)
                {
			printf("**** Error:select error no %d\n", errno);
			return;
		}

		tmp_buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		tmp_buf.memory      = V4L2_MEMORY_MMAP;
		tmp_buf.index = 0;
		if(ioctl(fd, VIDIOC_DQBUF, &tmp_buf)<0)
		{
			printf("**** Error:VIDIOC_DQBUF\n");
			continue;
		}
		iFrameCounter3++;
		if(iFrameCounter3%100==0)
		{
		 	gettimeofday(&tp3, NULL);
		 	iCostTime3 = (tp3.tv_sec - iSecond3)*1000000 + tp3.tv_usec - iuSecond3;
			printf("####100 frame=%u us, fps=%f\n", iCostTime3, 1000000*100.0/iCostTime3) ;
		 	iSecond3 = tp3.tv_sec;
		 	iuSecond3 = tp3.tv_usec;
		}
		if(iFlagCamera)
		{
			iCameraCounter++;
			iFlagCamera--;
			char GRAYname[100];
                        char YUVname[100];
			sprintf(YUVname, "/opt/mytest/out/gray%04d.yuv", iCameraCounter);
                        sprintf(GRAYname, "/opt/mytest/out/gray%04d.bmp", iCameraCounter);
                        YUVimageSave(YUVname,buffers[tmp_buf.index].start,buffers[tmp_buf.index].length);
			Cameraimge->data =  buffers[tmp_buf.index].start;
                        NV21_T_RGB(Yimge->width,Yimge->height,Cameraimge->data,Yimge->data);
                        SaveImage2bmp(GRAYname,Yimge);
			printf ("**** write success w=%d,h=%d j=%d!!!!\n", Yimge->width,Yimge->height,iCameraCounter);
		}
		tmp_buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		tmp_buf.memory      = V4L2_MEMORY_MMAP;
		if(ioctl(fd, VIDIOC_QBUF, &tmp_buf)<0)
		{
			printf("#### Error:VIDIOC_QBUF\n");
			continue;
		}
		iFrameCounter4++;
		if(iFrameCounter4%100==0)
		{
			gettimeofday(&tp4, NULL);
			iCostTime4 = (tp4.tv_sec - iSecond4)*1000000 + tp4.tv_usec - iuSecond4;
			printf("@@@@ 100 frame=%u us, fps=%f\n",iCostTime, 1000000*100.0/iCostTime) ;
			printf("####100 frame=%u us, fps=%f\n", iCostTime3, 1000000*100.0/iCostTime3) ;
			printf("^^^^ 100 frame=%u us, fps=%f\n", iCostTime4, 1000000*100.0/iCostTime4) ;
			printf("^^^^ 100 frame=%u us, fps=%f iCounter100frame=%d\n", iCostTime4, 1000000*100.0/iCostTime4, iCounter100frame) ;
			iSecond4 = tp4.tv_sec;
			iuSecond4 = tp4.tv_usec;
			iCounter100frame++;
			printf("\n\n") ;
		}
	}

	if(ioctl(fd, VIDIOC_STREAMOFF, &type)<0)
	{
		printf("Error:VIDIOC_STREAMOFF\n");
		return;
	}

	printf("\n capabilities= 0x%08x, iCapCapabilities=0x%08x \n",cap.capabilities, iCapCapabilities);
	getCameraCapabilities(iCapCapabilities);
	for(i=0;i<req.count;i++)
	{
		munmap(buffers[i].start, buf[i].length);
	}

	close(fd);
	return;
} 

int main(int argc, char **argv)
{
	pthread_t id1;
	pthread_t id2;
	int ret2;
	Cameraimge = (Image*)malloc(sizeof(Image));
	Cameraimge->width = 1920;
	Cameraimge->height = 1080;
	Cameraimge->channel = 1;
	Cameraimge->data = NULL;
	Yimge = (Image*)malloc(sizeof(Image));;
	Yimge->width = 1920;
	Yimge->height = 1080;
	Yimge->channel = 1;
	Yimge->data = malloc( Yimge->width *Yimge->height * 3);;
	ret2=pthread_create(&id2,NULL,(void *) thread2,NULL);
	if(ret2!=0)
	{
		printf ("Create pthread2 error!\n");
		return 0;
	}
	pthread_join(id2,NULL);
	return 0;
}

