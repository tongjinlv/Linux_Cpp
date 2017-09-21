#ifndef _RECV_H
#define _RECV_H
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
 
#define BAUDRATE B115200 ///Baud rate : 115200
#define DEVICE "/dev/ttyS2"
#define SIZE 1024
 
#endif
