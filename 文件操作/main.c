#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#define LENGTH 100
int GetTextLength(void *Tags)
{
	char *P;
	int  i=0;
	P=(char *)Tags;
	while(*P++)i++;
	return i;
}
char GetStringByFmt(char *String,char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	vsprintf(String,fmt,ap);
	va_end(ap);
	return GetTextLength(String);
}
int main()
{
	int fd,len,i,c=0;
	char str[LENGTH];
	char outbuf[1000];
	fd=open("hello.txt",O_CREAT|O_RDWR,S_IRUSR|S_IWUSR);
	if(fd)
	{
	for(i=0;i<LENGTH;i++)str[i]=i;
	write(fd,&str[0],LENGTH);
	close(fd);
	}
	fd=open("hello.txt",O_RDWR);
	if(fd)
	{
	len=read(fd,&str[0],LENGTH);
	for(i=0;i<LENGTH;i++)
	{
		c+=GetStringByFmt(&outbuf[c]," 0x%02x ",str[i]);
		if(i%10==9)c+=GetStringByFmt(&outbuf[c],"\n");
	}
		close(fd);
	}
	fd=open("out.txt",O_CREAT|O_RDWR,S_IRUSR|S_IWUSR);
        if(fd)
        {
        	write(fd,&outbuf[0],c);
		close(fd);
        }

	return 0;	
}
