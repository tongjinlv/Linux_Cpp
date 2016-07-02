/* Linux 
* --Mao
*
* 播放音频文件要用ioctl函数设置使播放参数与音频文件相同
* 设置声卡采样频率 arg = 16; ioctl(handler,SOUND_PCM_WRITE_BITS,&arg);
*
*
*
*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/soundcard.h>
#include <sys/time.h>


#define Audio_Device "/dev/dsp"

	/*
	不同的声音有不同的播放参数，这些参数可以使用file命令获得
	*/

#define Sample_Size 16
//#define Sample_Rate 44100
#define Sample_Rate 9600

struct timeval timer;

int play_sound(char *filename)
{
	struct stat stat_buf;
	unsigned char *buf = NULL;
	int handler,fd;
	int result;
	int arg,status;
	//打开声音文件，将文件读入内存
	fd = open(filename,O_RDONLY);
	if(fd < 0)
		return -1;
	if(fstat(fd,&stat_buf))
	{
		close(fd);
		return -1;
	}
	if(!stat_buf.st_size)
	{
		close(fd);
		return -1;
	}

         printf("need buf size:%d\n",stat_buf.st_size);
      
	buf = malloc(stat_buf.st_size);
	if(!buf)
	{
		close(fd);
		return -1;
	}
	


	/*
	打开声卡设备，并设置声卡播放参数，这些参数必须与声音文件参数一致
	*/

	handler = open(Audio_Device,O_WRONLY);
	if(handler == -1)
	{
		perror("Cannot open the Audio_Device");
		return -1;
	}

	arg = Sample_Rate;
	status = ioctl(handler,SOUND_PCM_WRITE_RATE,&arg);  //9600
	if(status == -1)
	{
		perror("error from SOUND_PCM_WRITE_RATE ioctl");
		return -1;
	}

	arg = Sample_Size;
	status = ioctl(handler,SOUND_PCM_WRITE_BITS,&arg);  //16
	if(status == -1)
	{
		perror("error from SOUND_PCM_WRITE_BITS ioctl");
		return -1;
	}

       //while(1)  {
        if(read(fd,buf,stat_buf.st_size) < 0)
        //if(read(fd,buf,10000) != 10000)
	{
		free(buf);
		close(fd);
               // break;
		return -1;
	}  
       gettimeofday(&timer, NULL);
    printf("t1 tv_sec(%d), tv_usec(%d)\n",timer.tv_sec, timer.tv_usec);
	result = write(handler,buf,stat_buf.st_size);
	if(result == -1)
	{
		perror("Fail to play the sound");
		return -1;
	}
        
   gettimeofday(&timer, NULL);
    printf("t2 tv_sec(%d), tv_usec(%d)\n",timer.tv_sec, timer.tv_usec);
   
        //}

	free(buf);
	close(fd);
	close(handler);
	return result;
}

void main(void)
{
	play_sound("sound.wav");
}
