#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#include "misc.h"
#undef LOG_TAG 
#define LOG_TAG "cdr_misc.cpp"
#include "debug.h"

void Line2 (HDC hdc, int x1, int y1, int x2, int y2)
{
	MoveTo(hdc, x1, y1);
	LineTo(hdc, x2, y2);
}

const char* removeSuffix(const char *file, char *buf)
{
	const char *ptr;
	ptr = strrchr(file, '.');
	if(ptr == NULL)
		return NULL;
	strncpy(buf, file, (ptr - file));
	buf[ ptr - file] = '\0';
	return buf;
}
const char* getBaseName(const char *file, char *buf)
{
	const char *ptr;	
	if(file == NULL || buf == NULL)
		return NULL;
	ptr = strrchr(file, '/');
	if(ptr == NULL) {
		strcpy(buf, file);
	} else {
		strcpy(buf, ptr + 1);
	}
	return buf;
}

int copyFile(const char* from, const char* to)
{
	db_msg("copy file\n");
#ifndef BUFFER_SIZE
#define BUFFER_SIZE 8192
#endif
	int from_fd,to_fd;
	int bytes_read,bytes_write;
	char buffer[BUFFER_SIZE + 1];
	char *ptr;
	int retval = 0;

	if(access(from, F_OK | R_OK) != 0) {
		db_error("%s\n", strerror(errno));
		return -1;
	}

	if((from_fd=open(from, O_RDONLY)) == -1 )
	{
		db_error("Open %s Error:%s\n", from, strerror(errno));
		return -1;
	}

	if((to_fd = open(to, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR)) == -1)
	{
		db_error("Open %s Error:%s\n", to, strerror(errno));
		close(from_fd);
		return -1;
	}

	while( (bytes_read = read(from_fd, buffer, BUFFER_SIZE)) )
	{
		if((bytes_read == -1) && ( errno != EINTR)) {
			db_msg("read error: %s\n", strerror(errno));
			retval = -1;
			break;
		}

		else if(bytes_read > 0)
		{
			ptr = buffer;
			while( (bytes_write = write(to_fd, ptr, bytes_read)) )
			{
				if((bytes_write == -1) && (errno != EINTR)) {
					db_msg("wirte error: %s\n", strerror(errno));
					retval = -1;
					break;
				}
				else if(bytes_write == bytes_read) {
					break;
				}
				else if( bytes_write > 0 )
				{
					ptr += bytes_write;
					bytes_read -= bytes_write;
				}
			}
			if(bytes_write == -1) {
				retval = -1;
				break;
			}
		}
	}
	db_msg("copy %s to %s\n", from, to);
	fsync(to_fd);
	close(from_fd);
	close(to_fd);
	return retval;
}

#define DISP_DEV	"/dev/disp"
#define DISP_CMD_LAYER_CK_OFF	0x52

void ck_off(int hlay)
{
	int ret;
	int fd;
	unsigned long args[4] = {0};
	args[0] = 0;
	args[1] = hlay;

	fd = open(DISP_DEV, O_RDWR);
	if(fd < 0) {
		db_error("open %s failed\n", DISP_DEV);
		return;
	}
	ret = ioctl(fd, DISP_CMD_LAYER_CK_OFF, args);
	if(ret < 0) {
		db_error("ck off failed\n");
	}
	close(fd);
}
#define DISP_CMD_LAYER_ALPHA_ON			0x4c
#define DISP_CMD_LAYER_ALPHA_OFF		0x4d
#define DISP_CMD_LAYER_SET_ALPHA_VALUE	0x4f
int set_ui_alpha(unsigned char alpha)
{
#ifdef PLATFORM_0
	int fd;	
	int hlay = 100;
	unsigned int args[4] = {0};
	if( (fd = open(DISP_DEV, O_RDWR)) == -1) {
		db_error("open %s failed: %s", DISP_DEV, strerror(errno));
		return -1;
	}
	args[0] = 0;
	args[1] = hlay;
	if(alpha < 255) {
		if(ioctl(fd, DISP_CMD_LAYER_ALPHA_ON, args) == -1) {
			db_error("set alpha failed: %s\n", strerror(errno));
			close(fd);
			return -1;
		}
		args[2] = alpha;
		if(ioctl(fd, DISP_CMD_LAYER_SET_ALPHA_VALUE, args) == -1) {
			db_error("set alpha failed: %s\n", strerror(errno));
			close(fd);
			return -1;
		}
	} else if(alpha == 255){
		args[2] = 255;
		if(ioctl(fd, DISP_CMD_LAYER_ALPHA_OFF, args) == -1) {
			db_error("set alpha failed: %s\n", strerror(errno));
			close(fd);
			return -1;
		}
	}
	close(fd);
#endif
	return 0;
}
#define USE_KEY_SOUND
#ifdef USE_KEY_SOUND
static char *keySoundDataBuffer = NULL;
static int keySoundReadNum = 0;
static int chunkFormat;
static float voiceVol = 0.6;
static bool isOpenVoice = true;

void setKeySoundVol(float val)
{
	voiceVol = val;
}
void switchKeyVoice(bool flag)
{
	isOpenVoice = flag;
}
void playKeySound()
{
	db_msg("playKeySound");
}
int initKeySound(const char *path)
{
	return 0;
}
#endif

void getScreenInfo(int *w, int *h)
{
	static int swidth = 0;
	static int sheight =0;
	if(swidth == 0 ){
		char *env = NULL;
		char mode[32]={0};
		db_msg(" ");
		if ((env=getenv("MG_DEFAULTMODE")) != NULL) {
			strncpy(mode, env, strlen(env)+1);
		}
		db_msg(" ");
		char *pW = NULL;
		char *pH = NULL;
		pW = strstr(mode, "x");
		*pW = '\0';
		swidth = atoi(mode);
		*pW = 'x';
		pH = strstr(mode, "-");
		*pW = '\0';
		sheight = atoi(pW+1);
		*pW = '-';
	}
	*w = swidth;
	*h = sheight;
}
