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
int main(int argc, char **argv)
{
        pthread_t captureTid;
        const char* captureFile = "Rear_Center.wav";
        const char* recordFile = "Record.wav";
        //if(!strcmp("mic", argv[1]))
}
