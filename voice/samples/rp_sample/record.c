#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/ioctl.h>
#include<stdlib.h>
#include<stdio.h>
#include<linux/soundcard.h>
#include<termios.h>

#define LENGTH 20
#define RATE 44100
#define SIZE 16
#define CHANNELS 2
#define RSIZE 8

struct fhead
{
	unsigned char a[4];
	long int b;
	unsigned char c[4];
    	unsigned char d[4];
	long int e;
	short int f;
	short int g;
	long int h;
	long int i;
	short int j;
	short int k;
	unsigned char p[4];
	long int q;
}wavehead;
int main(void)
{
	int fd_dev_r;
	int fd_dev_w;
	int fd_f;
	int arg,status;
	char choice;
	int i;
	unsigned char buf[RSIZE];
	wavehead.a[0]='R';
	wavehead.a[1]='I';
	wavehead.a[2]='F';
	wavehead.a[3]='F';
	wavehead.b=LENGTH*RATE*CHANNELS*SIZE/8-8;
	wavehead.c[0]='W';
	wavehead.c[1]='A';
	wavehead.c[2]='V';
	wavehead.c[3]='E';
	wavehead.d[0]='f';
	wavehead.d[1]='m';
	wavehead.d[2]='t';
	wavehead.d[3]=' ';
	wavehead.e=16;
	wavehead.f=1;
	wavehead.g=CHANNELS;
	wavehead.h=RATE;
	wavehead.i=RATE*CHANNELS*SIZE/8;
	wavehead.j=CHANNELS*SIZE/8;
	wavehead.k=SIZE;
	wavehead.p[0]='d';
	wavehead.p[1]='a';
	wavehead.p[2]='t';
	wavehead.p[3]='a';
	wavehead.q=LENGTH*RATE*CHANNELS*SIZE/8;
	fd_dev_r=open("/dev/dsp",O_RDONLY,0777);
	if(fd_dev_r<0)
	{
		perror("You Con't open /dev/dsp device");
		return 1;
	}
	if(ioctl(fd_dev_r,SNDCTL_DSP_SYNC,NULL)==-1)
		perror("SNDCTL_DSP_SYNC");
	arg=SIZE;
	status=ioctl(fd_dev_r,SNDCTL_DSP_SETFMT,&arg);
	if(status==-1)
	{
		perror("You Con't set SNDCTL_DSP_SETFMT");
 		return 1;
	}
	arg=CHANNELS;
	status=ioctl(fd_dev_r,SNDCTL_DSP_CHANNELS,&arg);
	if(status==-1)
	{
		perror("You Con't set SNDCTL_DSP_CHANNELS");
 		return 1;
	}
	arg=RATE;
	status=ioctl(fd_dev_r,SNDCTL_DSP_SPEED,&arg);
	if(status==-1)
	{
		perror("You Con't set SNDCTL_DSP_SPEED");
 		return 1;
	}
	fd_dev_w = open("/dev/dsp",O_WRONLY,0777);
	if(fd_dev_w<0)
	{
		perror("Cannot open /dev/dsp device");
		return 1;
	}
	arg=SIZE;
	status=ioctl(fd_dev_w,SNDCTL_DSP_SETFMT,&arg);
	if(status==-1)
	{
		perror("You Con't set SNDCTL_DSP_SETFMT");
 		return 1;
	}
	arg=CHANNELS;
	status=ioctl(fd_dev_w,SNDCTL_DSP_CHANNELS,&arg);
	if(status==-1)
	{
		perror("You Con't set SNDCTL_DSP_CHANNELS");
 		return 1;
	}
	arg=RATE;
	status=ioctl(fd_dev_w,SNDCTL_DSP_SPEED,&arg);
	if(status==-1)
	{
		perror("You Con't set SNDCTL_DSP_SPEED");
 		return 1;
	}

	//system("rm  -f *.wav);
	if((fd_f = open("/mnt/hgfs/linuxshare/ly.wav",O_CREAT|O_RDWR,0777))==-1)
	{
		perror("cannot creat the sound file");
	}
	if((status = write(fd_f, &wavehead, sizeof(wavehead)))==-1)
	{
		perror("write to sound'head wrong!!");
	}
	for(i=0;i<(LENGTH*RATE*SIZE*CHANNELS/8)/RSIZE;i++)
	{
		status = read(fd_dev_r, buf, sizeof(buf));
		if (status != sizeof(buf))
		{
			perror("read wrong number of bytes");
		}
		if(write(fd_f, buf, status)==-1)
		{
			perror("write to sound wrong!!");
		}
	}
	close(fd_dev_r);
	close(fd_f);
	printf("play your sound now??\n y OR n\n");
	if(getchar()=='y')
	{
		printf("Play...:\n");
		if(( fd_f = open("/mnt/hgfs/linuxshare/ly.wav", O_RDONLY,0777))==-1)
		{
			perror("cannot creat the sound file");
		}
		lseek(fd_f,44,SEEK_SET);
		for(i=0;i<(LENGTH*RATE*SIZE*CHANNELS/8)/RSIZE;i++)
		{
		status = read(fd_f, buf, sizeof(buf));//读语音文件
		if (status != sizeof(buf))
			perror("write wrong number of bytes");
		status = write(fd_dev_w, buf, sizeof(buf));//送声卡播放
		if (status != sizeof(buf))
			perror("wrote2 wrong number of bytes");
		}
		close(fd_f);
		close(fd_dev_w);
		return 0;
		}
		else
		{
			printf("bye!\n");
			return 0;
		}

}
