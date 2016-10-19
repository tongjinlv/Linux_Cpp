    #include <stdio.h>   
    #include <sys/ioctl.h>   
    #include <signal.h>   
    #include <stdlib.h>   
    #include <time.h>   
      
    #define LED_ON      _IOW('h',0x01,unsigned long)        //LED开的命令   
    #define LED_OFF     _IOW('h',0x02,unsigned long)        //LED关的命令   
      
    int fd;  
      
    void close_file(int sig_no)  
    {  
        if(SIGINT == sig_no)  
        {  
            close(fd);  
            printf("Close the file suc!\n");  
            exit(0);  
        }  
    }  
    int main(void)  
    {  
      
        fd = open("/dev/led",0);  
        if(fd < 0)  
        {  
            perror("Open file Error:");  
            return -1;  
        }  
      
        signal(SIGINT,close_file);  
        while(1)  
        {  
        ioctl(fd,LED_ON);       //led on   
        usleep(500000);     //delay 500 ms   
        ioctl(fd,LED_OFF);      //led off   
        usleep(500000);  
        }  
        return 0;  
    }  
