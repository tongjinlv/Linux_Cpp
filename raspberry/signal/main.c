#include <signal.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/time.h> 
int sec; 

void sigroutine(int signo) { 
printf("Catch a signal -- SIGVTALRM "); 
exit(0);
} 

int main() { 
struct itimerval value,ovalue,value2; 
sec = 5; 

printf("process id is %d ",getpid()); 
signal(SIGALRM, sigroutine); 
signal(SIGINT, sigroutine); 
signal(SIGVTALRM, sigroutine); 

value.it_value.tv_sec = 1; 
value.it_value.tv_usec = 0; 
value.it_interval.tv_sec = 1; 
value.it_interval.tv_usec = 0; 
setitimer(ITIMER_REAL, &value, &ovalue); 

value2.it_value.tv_sec = 0; 
value2.it_value.tv_usec = 500000; 
value2.it_interval.tv_sec = 0; 
value2.it_interval.tv_usec = 500000; 
setitimer(ITIMER_VIRTUAL, &value2, &ovalue); 

for (;;) ; 
} 

