#ifndef _STORAGE_MANAGER_H
#define _STORAGE_MANAGER_H

#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <fcntl.h>
#include <mutex>
#include <string>
#include <vector>


#include <minigui/common.h>
#include <minigui/gdi.h>

#include "Process.h"
#include "misc.h"
#include "cdr_res.h"

#include "Rtc.h"
#include "EventManager.h"
#include "windows.h"
#include "cdr_widgets.h"
#include "keyEvent.h"
#include "cdr_message.h"
#include "ResourceManager.h"

using namespace zeta;

long long totalSize(const char *path);
long long availSize(const char *path);

#define MOUNT_POINT STORAGE_POINT
#define MOUNT_PATH STORAGE_PATH
#define SZ_1M ((unsigned long long)(1<<20))
#define SZ_1G ((unsigned long long)(1<<30))
#define MAX_CARD_SIZE_MB ((unsigned long long)1<<17)	//1024*128 MB
//#define RESERVED_SIZE (SZ_1M * 300)
//#define RESERVED_SIZE ((unsigned long long)((SZ_1M*500)/SZ_1M))
#define RESERVED_SIZE ((unsigned long long)((SZ_1M*103)/SZ_1M))
#define LOOP_COVERAGE_SIZE (RESERVED_SIZE + ((SZ_1M*1100)/SZ_1M))
#define LOOP_COVERAGE_MY   ((unsigned long long)((SZ_1M*110)/SZ_1M))


#define FILE_SZ_1M ((uint32_t)(1*1024*1024))

#define ALIGN( num, to ) (((num) + (to-1)) & (~(to-1)))
#define ALIGN32M(num) (ALIGN(num, (1<<25)))

#define MINUTE(ms) (ms/1000)
#define CSI_PREALLOC_SIZE(ms) (ALIGN32M((MINUTE(ms)*FRONT_CAM_BITRATE/8)))
#define UVC_PREALLOC_SIZE(ms) (ALIGN32M((MINUTE(ms)*BACK_CAM_BITRATE/8)))
#define AWMD_PREALLOC_SIZE(ms) CSI_PREALLOC_SIZE(ms)


#define CDR_TABLE "CdrFile"

//#define status_t int

typedef unsigned short WORD;
typedef unsigned char BYTE;

#define WININFOHEADERSIZE  40

#define SIZEOF_BMFH     14
#define SIZEOF_BMIH     40


#define BI_RGB          0
#define BI_RLE8         1
#define BI_RLE4         2
#define BI_BITFIELDS    3

#define SIZEOF_RGBQUAD      4

typedef struct tagRGBQUAD
{
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} RGBQUAD;


typedef struct BITMAPFILEHEADER
{
   unsigned short bfType;
   unsigned long  bfSize;
   unsigned short bfReserved1;
   unsigned short bfReserved2;
   unsigned long  bfOffBits;
} BITMAPFILEHEADER;

typedef struct WINBMPINFOHEADER  /* size: 40 */
{
   unsigned long  biSize;
   unsigned long  biWidth;
   unsigned long  biHeight;
   unsigned short biPlanes;
   unsigned short biBitCount;
   unsigned long  biCompression;
   unsigned long  biSizeImage;
   unsigned long  biXPelsPerMeter;
   unsigned long  biYPelsPerMeter;
   unsigned long  biClrUsed;
   unsigned long  biClrImportant;
} WINBMPINFOHEADER;

typedef struct abgrColor
{
    /**
     * The red component of a RGBA quarter.
     */
    unsigned char r;
    /**
     * The green component of a RGBA quarter.
     */
    unsigned char g;
    /**
     * The blue component of a RGBA quarter.
     */
    unsigned char b;
    /**
     * The alpha component of a RGBA quarter.
     */
    unsigned char a;
} abgrColor;


#define FILE_EXSIST(PATH)	(!access(PATH, F_OK))

#define FLAG_NONE ""
#define FLAG_SNAP "_snap"
#define FLAG_SOS "_SOS"

#define FILE_FRONT "A"
#define FILE_BACK "B"

#ifdef TS_MUXER
#define EXT_VIDEO_MP4 ".mov"
#else
#define EXT_VIDEO_MP4 ".mp4"
#endif
#define EXT_PIC_JPG ".jpg"
#define EXT_PIC_BMP ".bmp"

#define GENE_IMPACT_TIME_FILE  "/data/impcttime.db"
#define IMPACT_TIME_TEXT_LENGTH   20
#define IMPACT_TIME_TEXT  128

enum {
	RET_IO_OK = 0,
	RET_IO_NO_RECORD = -150,
	RET_IO_DB_READ = -151,
	RET_IO_DEL_FILE = -152,
	RET_IO_OPEN_FILE = -153,
	RET_NOT_MOUNT = -154,
	RET_DISK_FULL = -155,
	RET_NEED_LOOP_COVERAGE = -156,
};

typedef struct elem
{
	char *file;
	long unsigned int time;
	char *type;
	char *info;
}Elem;

typedef struct {
	std::string file;
	int time;
	std::string type;
	std::string info;
}dbRecord_t;

class CommonListener
{
public:
	CommonListener(){};
	virtual ~CommonListener(){}
	virtual void finish(int what, int extra) = 0;
};

class StorageManager
{
private:
	void init();
	StorageManager();
	~StorageManager();
public:
	static StorageManager* getInstance();
	void exit();
	void notify(int what, int status, char *msg);
	bool isInsert();
	bool isMount();
	int doMount(const char *fsPath, const char *mountPoint,
	             bool ro, bool remount, bool executable,
	             int ownerUid, int ownerGid, int permMask, bool createLost);
	int format(CommonListener *listener);
	int reNameFile(const char *oldpath, const char *newpath);
	void dbInit();
	void dbExit();
	void dbReset();
	int dbCount(char *item);
	void dbGetFiles(char *item, std::vector<std::string>&file);
	void dbGetFile(char *item, std::string &file );
	int dbGetSortByTimeRecord(dbRecord_t &record, int index);
	void dbAddFile(Elem *elem);
	int dbDelFile(char const *file);
	int generateFile(time_t &now, std::string &file, int cam_type, fileType_t type, bool cyclic_flag=true, bool next_fd=false);
	int savePicture(void* data, int size, std::string &file);
	int checkDirs();
	int dbUpdate();
	int getSoS_fileCapitly();
	int dbUpdate(char *type);
	int deleteFile(const char *file);
	bool dbIsFileExist(std::string file);
	int shareVol();
	int unShareVol();
	bool isDiskFull();
	unsigned long fileSize(std::string path);
	void dbUpdateFile(const char* oldname, const char *newname);

	void dbUpdateFileType(const char* name, const char *type) ;
	void setFileInfo(const char* oldName, bool isLock1 ,std::string& nfile);

	std::string lockFile(const char *path);
	int getCapacity(unsigned long long *as, unsigned long long *ts);

	void stopUpdate();
	void dblock(bool flag);
	#ifdef CAMDROID
	class FormatThread : public Thread
	{
	public:
		virtual bool threadLoop() {
			mSM->format();
			mSM->dbReset();
			mListener->finish(0, 0);
			return false;
		}
		status_t start() {
			return run("FormatThread", PRIORITY_NORMAL);
		}
		FormatThread(StorageManager *sm, CommonListener *listener) {mSM = sm; mListener = listener;};
	private:
		StorageManager *mSM;
		CommonListener *mListener;
	};
	#endif
	void setEventListener(EventListener *pListener);
	int  readCpuInfo(char ret[]);
	int geneImpactTimeFile();
	int readImpactTimeFile(char ret[]);

	#ifdef CAMDROID
	class StorageSyncThread : public Thread
	{
	public:
		virtual bool threadLoop() {
			storageSyncLoop();
			return false;
		}
		status_t start() {
			return run("StorageSyncThread", PRIORITY_NORMAL);
		}
		void stopThread()
		{
			mLoop = false;
		}
		void storageSyncLoop();
		StorageSyncThread(StorageManager *sm, bool loop=false);
	private:
		StorageManager *mSM;
		bool mLoop;
	};
	#endif
	void startSyncThread(bool loop=false);
	void stopSyncThread();
	void storageSync();
	int  deleteRecentlyFile();

private:
	int isInsert(const char *checkPath);
	int isMount(const char *checkPath);
	int deleteFiles(const char *type, const char *info);
	int deleteFiles();
	int format();
	std::mutex mDBLock;
	#ifdef CAMDROID
	sp <FormatThread> mFT;
	sp <StorageSyncThread> mSST;
	#endif
	bool mInsert;
	bool mMounted;
    bool mAbnormalRemoved;
	EventListener *mListener;
	bool mDBInited;
	std::mutex mDBInitL;
	bool mStopDBUpdate;
};
#endif
