#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
int system(const char *string);

void tts_play(const char *path)
{
	system("./openwav.sh");
	printf("路径=%s",path);
	printf("this play music");
}
