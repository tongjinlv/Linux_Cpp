

#undef LOG_TAG
#define LOG_TAG "StorageManager"
#include "debug.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include "StorageManager.h"
#define SYNC_THREAD

static StorageManager* gInstance;
static std::mutex gInstanceMutex;

void StorageManager::init()
{
	if(isMount(MOUNT_POINT)) {
		mMounted = true;
		checkDirs();
	}
	else
		mMounted = false;

	db_msg("%s is %s\n", MOUNT_PATH, (mMounted == true) ? "mounted" : "not mount");
}

void StorageManager::exit()
{

}

int StorageManager::shareVol()
{

	return -1;
}
int StorageManager::unShareVol()
{
	return -1;
}
StorageManager::StorageManager()
	: mListener(NULL)
	#ifdef CAMDROID
	, mSST(NULL)
	#endif
	, mDBInited(false)
	, mStopDBUpdate(false)
	, mAbnormalRemoved(false)
{
	init();
}

StorageManager::~StorageManager()
{
	exit();
}
StorageManager* StorageManager::getInstance(void)
{	
#if 0
	mutex::Autolock _l(gInstanceMutex);
	if(gInstance != NULL) {
		return gInstance;
	}
	gInstance = new StorageManager();
	return gInstance;
#endif
	std::unique_lock<std::mutex> lck(gInstanceMutex);
	if(gInstance != NULL) {
		return gInstance;
	}
	gInstance = new StorageManager();
	return gInstance;
}

void StorageManager::notify(int what, int status, char *msg)
{
	db_msg("by hero notify");
}
int StorageManager::isInsert(const char *checkPath)
{
	FILE *fp;
	db_msg("------------checkPath = %s \n", checkPath);
	if (!(fp = fopen(checkPath, "r"))) {
		db_msg(" isInsert fopen fail, checkPath = %s\n", checkPath);
		return 0;
	}
	if(fp){
		fclose(fp);
	}
	return 1;
}

int StorageManager::isMount(const char *checkPath)
{
	const char *path = "/proc/mounts";
	FILE *fp;
	char line[255];
	const char *delim = " \t";
	db_msg(" isMount checkPath=%s \n",checkPath);
	if (!(fp = fopen(path, "r"))) {
		db_msg(" isMount fopen fail,path=%s\n",path);
		return 0;
	}
	memset(line,'\0',sizeof(line));
	while(fgets(line, sizeof(line), fp))
	{
		char *save_ptr = NULL;
		char *p        = NULL;
		//db_msg(" isMount line=%s \n",line);
		if (line[0] != '/' || strncmp("/dev/block/vold",line,strlen("/dev/block/vold")) != 0)
		{
			memset(line,'\0',sizeof(line));
			continue;
		}       
		if ((p = strtok_r(line, delim, &save_ptr)) != NULL) {		    			
			if ((p = strtok_r(NULL, delim, &save_ptr)) != NULL){
				db_msg(" isMount p=%s \n",p);
				if(strncmp(checkPath,p,strlen(checkPath) ) == 0){
					fclose(fp);
					db_msg(" isMount return 1\n");
					return 1;				
				}				
			}					
		}		
	}//end while(fgets(line, sizeof(line), fp))	
	if(fp){
		fclose(fp);
	}
	return 0;
}

bool StorageManager::isInsert(void)
{
	db_msg("------------mInsert = %d \n", mInsert);
	return mInsert;
}
bool StorageManager::isMount(void)
{
	return mMounted;
}

std::string StorageManager::lockFile(const char *path)
{
#if 0
	string file(path);
	// /mnt/extsd/video/time_A.mp4 -> /mnt/extsd/video/time_A_SOS.mp4
	file = file.c_str();
	file += FLAG_SOS;
	file += EXT_VIDEO_MP4;
	db_msg("lockFile:%s --> %s", path, file.string());
	file = string(CON_2STRING(MOUNT_PATH,LOCK_DIR))+file.getPathLeaf();
	reNameFile(path, file.c_str());

	return file;
#endif
	std::string file = path;

	return file;

}

int StorageManager::reNameFile(const char *oldpath, const char *newpath)
{
	int ret = 0;

	ret = rename(oldpath,newpath);
	if(ret != 0){
		db_error("reNameFile fail oldpath=%s, newpath = %s,ret=%d\n",oldpath,newpath,ret);
	}

	return ret;
}

int needDeleteFiles()
{
	unsigned long long as = availSize(MOUNT_PATH);
	unsigned long long ts = totalSize(MOUNT_PATH);
	db_msg("[Size]:%lluMB/%lluMB, reserved:%lluMB", as, ts, RESERVED_SIZE);
#if 1
	//			103M
	if (as <= RESERVED_SIZE) {
		db_msg("!Disk FULL");
		return RET_DISK_FULL;
	}
	//			1203M
	if (as <= LOOP_COVERAGE_SIZE) {
		db_msg("!Loop Coverage");
		return RET_NEED_LOOP_COVERAGE;
	}
	
#else
	if(ts <= LOOP_COVERAGE_SIZE) {
		db_msg("----------TF card too small-----");
		return RET_DISK_FULL;
	}else if (as <= LOOP_COVERAGE_SIZE) {
		db_msg("------sd card will full---");
		
	}
#endif
	return 0;
}

int StorageManager::getCapacity(unsigned long long *as, unsigned long long *ts)
{
	*as = availSize(MOUNT_PATH);
	*ts = totalSize(MOUNT_PATH);
	return 0;
}

int StorageManager::deleteFile(const char *file)
{
#if 0
	int ret = 0;
	string path(file);
	if (FILE_EXSIST(file)) {
		if (remove(file) != 0) {
			ret = -1;
		}
	} else {
		db_msg("file not existed:%s", file);
	}
	/* delete thumbnail */
	if (path.getPathExtension() == EXT_VIDEO_MP4 ||
		path.getPathExtension() == EXT_PIC_JPG) {
		string stime;
		stime = path.getBasePath().getPathLeaf();
		path = (path.getPathDir())+"/"+THUMBNAIL_DIR+stime+EXT_PIC_BMP;// /mnt/extst/video/time_A.mp4 ->/mnt/extsd/video->/mnt/extsd/video/.thumb/time_A.bmp
		db_msg("delete pic :%s", path.string());
		return deleteFile(path.string());
	}
	return ret;
#endif
	
	return 0;
}

int StorageManager::deleteFiles(const char *type, const char *info)
{
	std::unique_lock<std::mutex> lock(mDBLock);
	return RET_IO_OK;
}

int StorageManager::deleteFiles()
{
	int ret = 0;

	ret = deleteFiles(TYPE_VIDEO, INFO_NORMAL);
	if (ret != RET_IO_OK) {
		ret = deleteFiles(TYPE_VIDEO, INFO_LOCK);
	}
	return ret;
}

int StorageManager::generateFile(time_t &now, std::string &file, int cam_type, fileType_t type, bool cyclic_flag, bool next_fd)
{
	
	db_msg("by hero nothing generateFile");
	return RET_IO_OK;
}

int  StorageManager::readCpuInfo(char ret[]){
       char line[128];
       FILE *fp = fopen("/proc/cpuinfo", "r");
       if(fp==NULL){
               db_msg("open file failed.");
               return -1;
       }
       memset(ret,0,sizeof(ret));
       while(fgets(line, sizeof(line), fp)) {
               if (strncmp(line, "Serial", strlen("Serial")) == 0) {
                       char *str = strstr(line, ":");
                       str += 2;
               //      int num = atoi(str);
               //      printf("serial is %d\n", num);
	               if(strlen(str)>4){
	                       str +=(strlen(str)-5);
						   char* str1 = strstr(str, "\n");
						   if (str1 != NULL) {
						       *str1 = '\0';
						   }
	                       strcpy(ret,str);
	                       db_msg("str=%s,ret=%s",str,ret);
	               	}
               }
       }
       fclose(fp);
       return 0;

}


int StorageManager::geneImpactTimeFile(){
	int fd;
	int ret;
	struct tm *tm=NULL;
	char buf[IMPACT_TIME_TEXT_LENGTH]= {0};
	memset(buf,0,sizeof(buf));
	if(isMount()== false)
		return 0;
	db_msg("%s %d", __FUNCTION__, __LINE__);

	if (!access(GENE_IMPACT_TIME_FILE, F_OK)){
		db_msg("file in -----------");
	}
	fd = open(GENE_IMPACT_TIME_FILE, O_RDWR|O_APPEND | O_CREAT, 0666);
	if (fd < 0) {
		db_msg("failed to open file '%s'!!\n", buf);
		return 0;
	}
	getDateTime(&tm);

	sprintf(buf, "%04d/%02d/%02d %02d:%02d",tm->tm_year + 1900, tm->tm_mon + 1,
		tm->tm_mday, tm->tm_hour,tm->tm_min);
	db_msg("%s %d", __FUNCTION__, __LINE__);
	buf[16]='\n';
	ret = write(fd,buf,sizeof(buf));
	close(fd);
	if(ret<0){
		db_msg("write file error!, %s,%d",__FUNCTION__, __LINE__);
		return 0;
	}
	
	return 1;


}

int StorageManager::readImpactTimeFile(	char ret[]){
	int fd,size,count=0;
	char buf[IMPACT_TIME_TEXT_LENGTH];
	db_msg("****--------------------%s,%d***",__FUNCTION__,__LINE__);
	memset(buf,0,sizeof(buf));
	memset(ret,0,sizeof(ret));
    if (access(GENE_IMPACT_TIME_FILE, F_OK)) {
		db_msg("impct_time_file not found");
		return 0;
	}
	fd = open(GENE_IMPACT_TIME_FILE, O_RDONLY , 0666);
	if (fd < 0) {
		db_msg("open file false'%s'!!\n", buf);
		return 0;
	}
	do{
		if(count<10)
			count++;
		else
			break;
		size=read(fd,buf,sizeof(buf));
		db_msg("******buff=%s----size=%d",buf,size);
		if(size==0)
			break;
		strcat(ret,buf);
		memset(buf,0,sizeof(buf));
	}while(size>1);
	close(fd);
	if(size<0){
		db_msg("read file failed!");
		db_msg("%s,%d",__FUNCTION__,__LINE__);
		return 0;
	}
	return 1;
}

int StorageManager::doMount(const char *fsPath, const char *mountPoint,
		bool ro, bool remount, bool executable,
		int ownerUid, int ownerGid, int permMask, bool createLost) {
	db_msg("doMount");
	return 0;
}

int StorageManager::format(CommonListener *listener)
{

#ifdef ASYNC_FORMAT
	mFT = new FormatThread(this, listener);
	status_t err = mFT->start();
	if (err != OK) {
		mFT.clear();
	}
#else
	format();
	//dbReset();
//	listener->finish(0, 0);	
#endif
	return 0;
}

int StorageManager::format()
{
	int ret = -1;
	
	checkDirs();
	return ret;
}

long long totalSize(const char *path)
{

	return 0;
}

long long availSize(const char *path)
{

	return 0;
}

int getFiles(char *dir, char *filetype, std::vector<std::string> &files)
{
#if 0
	string file(dir);
	DIR *p_dir;
	struct stat stat;
	struct dirent *p_dirent;
	p_dir = opendir(dir);
	if (p_dir == NULL) {
		db_error("fail to opendir %s", dir);
		return -1;
	}
	while( (p_dirent = readdir(p_dir)) ) {
		if (strcmp(p_dirent->d_name, ".") == 0 ||
				strcmp(p_dirent->d_name, "..") == 0) {
			continue;
		}
		file = dir;
		file += p_dirent->d_name;
		if(lstat(file.string(), &stat) == -1) {
			db_error("fail to get file stat %s", file.string());
			continue;
		}
		if (!S_ISREG(stat.st_mode)) {
			db_error("%s is not a regular file", file.string());
			continue;
		}
		string ext = file.getPathExtension();
		if(ext.isEmpty()) {
			continue;
		}
		if(strcasecmp(ext.string(), filetype) == 0) {
			files.add(file);
		} else {
			db_debug("file is not type:%s", filetype);
		}
	}
	closedir(p_dir);
#endif
	return 0;
}

void StorageManager::dbReset()
{
	mDBInitL.lock();

	mDBInitL.unlock();
}

void StorageManager::dbInit()
{
	if (mDBInited) {
		return ;
	}
	dbUpdate();
	mDBInited = true;
}

time_t parase_time(char *ptr1)
{
	char *ptr;
	char chr;
	int value;
	struct tm my_tm;
	time_t my_time_t;
	/*
	   ptr = strchr(filename, '.');
	   if(ptr == NULL) {
	   printf("not found the delim\n");
	   return 0;
	   }
	   db_msg("filename is %s\n", filename);
	   ptr1 = strndup(filename, ptr - filename - 1);
	   */
	memset(&my_tm, 0, sizeof(my_tm));
	if (strlen(ptr1) >= strlen("20141201_121212")) {
	/************ year ************/
	ptr = ptr1;
	chr = ptr[4];
	ptr[4] = '\0';
	value = atoi(ptr);
	my_tm.tm_year = value - 1900;
	ptr[4] = chr;

	/************ month ************/
	ptr = ptr + 4;
	chr = ptr[2]; 
	ptr[2] = '\0';
	value = atoi(ptr);
	my_tm.tm_mon = value - 1;
	ptr[2] = chr;

	/************ day ************/
	ptr = ptr + 2;
	chr = ptr[2]; 
	ptr[2] = '\0';
	value = atoi(ptr);
	my_tm.tm_mday = value;

	/************ hour ************/
	ptr += 3;
	chr = ptr[2]; 
	ptr[2] = '\0';
	value = atoi(ptr);
	my_tm.tm_hour = value;
	ptr[2] = chr;

	/************ minite ************/
	ptr += 2;
	chr = ptr[2]; 
	ptr[2] = '\0';
	value = atoi(ptr);
	my_tm.tm_min = value;
	ptr[2] = chr;

	/************ second ************/
	ptr += 2;
	chr = ptr[2]; 
	ptr[2] = '\0';
	value = atoi(ptr);
	my_tm.tm_sec = value;
	} else {
		my_tm.tm_year = 2014;
		my_tm.tm_mon  = 12;
		my_tm.tm_mday = 1;
		my_tm.tm_hour = 1;
		my_tm.tm_min  = 1;
		my_tm.tm_sec  = 1;
	}
//	db_msg("year: %d, month: %d, day: %d, hour: %d, minute: %d, second: %d\n", 
//			my_tm.tm_year, 
//			my_tm.tm_mon, 
//			my_tm.tm_mday, 
//			my_tm.tm_hour, 
//			my_tm.tm_min, 
//			my_tm.tm_sec);
	my_time_t = mktime(&my_tm);
//	db_msg("time_t is %lu, time str is %s\n", my_time_t, ctime(&my_time_t));
	return my_time_t;
}

unsigned long StorageManager::fileSize(std::string path)
{
#if 0
	struct stat	buf; 
	stat(path.string(),&buf);
	if (buf.st_size == 0) {
		db_msg("%s size is 0", path.string());
	}
	return buf.st_size;
#endif
	return 0;
}

int StorageManager::dbUpdate(char *type)
{
#if 0
	vector<string> files_db, files;
	string file, file_db, tmp, stime;
	//	vector<string>::iterator result;
	Elem elem;
	time_t time;
	int size, size_db, i, ret=0;
	const char *ext = NULL;
	if (0 == strcmp(type, TYPE_VIDEO)) {
		ext = EXT_VIDEO_MP4;
		ret = getFiles((char*)CON_2STRING(MOUNT_PATH, VIDEO_DIR), (char*)ext, files);
		ret = getFiles((char*)CON_2STRING(MOUNT_PATH, LOCK_DIR), (char*)ext, files);
	} else if (0 == strcmp(type, TYPE_PIC)) {
		ext = EXT_PIC_JPG;
		ret = getFiles((char*)CON_2STRING(MOUNT_PATH, PIC_DIR), (char*)ext, files);
	}
	/*
	dbGetFiles(type, files_db);
	size_db = files_db.size();
	for(i=0; i<size_db; i++) {
		file_db = files_db.itemAt(i);
		if (!FILE_EXSIST(file_db.string())) {	//File is in the database but does not exist
			db_msg("has not found %s, delete from database", file_db.string());
			dbDelFile(file_db.string());	//delete file record in the database			
			ret = deleteFile(file_db.string());
			if (ret < 0) {
				db_debug("fail to delete file %s", (const char *)file_db.string());
				return RET_IO_DEL_FILE;
			}
		} else {
			db_msg("has found %s in the tfcard", file_db.string());
		}
	}
	*/
	size = files.size();
	for(i=0; i<size; i++)
	{
		if (mStopDBUpdate) {
			return 0;
		}
		//isValidVideoFile(files.itemAt(i));
		file = files.itemAt(i);
		if (!dbIsFileExist(file)) {	//file exsit but there is no record in the database
			if (fileSize(file) == 0) {	//file size is 0
				deleteFile(file.string());
				continue;
			}
			time = parase_time((char*)file.getBasePath().getPathLeaf().string());
			elem.file = (char *)file.string();
			elem.time = (long unsigned int)time;
			elem.type = type;
			elem.info = (char*)INFO_NORMAL;
			if (0 == strcmp(type, TYPE_VIDEO)) {
				if(strstr(file.string(), FLAG_SOS) != NULL) {
					db_msg("file type is %s\n", FLAG_SOS);
					elem.info = (char*)INFO_LOCK;
				}
			}
			dbAddFile(&elem);	//new file insert to the database
		}
//		db_msg("file %s exists in the database", files.itemAt(i).string());
	}
#endif
	return 0;
}

int StorageManager::dbUpdate()
{
//	db_msg("------dbUpate");
	int ret = 0;
	if(isMount()== false) {
		db_error("not mount\n");
		return -1;
	}
	mStopDBUpdate = false;
	db_msg(" ");
	ret = dbUpdate((char*)TYPE_VIDEO);
	if (ret >= 0) {
		ret = dbUpdate((char*)TYPE_PIC);
	}
	return ret;
}


int StorageManager::getSoS_fileCapitly()
{
	std::vector<std::string> files;
	std::string file;
	int size, i, ret=0;
	unsigned long int file_size=0;
	unsigned long int tmp,file_size_total=0;
	const char *ext = NULL;
	ext = EXT_VIDEO_MP4;
	ret = getFiles((char*)CON_2STRING(MOUNT_PATH, TYPE_LOCK), (char*)ext, files);
	size = files.size();
	db_msg("--------[debug_jason]:the file cont is :%d------------\n",size);
	for (i= 0; i<size;i++)
	{
		file = files[i];
		db_msg("----the file name is :%s---------\n",(char *)file.c_str());
		if(strstr(file.c_str(), FLAG_SOS) != NULL) {
			file_size = fileSize(file);
			tmp = file_size;
			file_size_total += tmp;
			db_msg("file type is %s\n", FLAG_SOS);
			db_msg("----the file size is :%ld---------\n",file_size);
			db_msg("----the file file_size_total is :%ld---------\n",file_size_total);
		}
	}
	return file_size_total;
	
}

void StorageManager::dbExit()
{
	db_msg("dbexit");
}

/* create dirs
 * /mnt/extsd/video
 * /mnt/extsd/photo
 * /mnt/extsd/video/.thumb
 * /mnt/extsd/photo/.thumb
 */

int _mkdirs(const char *dir)
{
	int ret = 0;
#ifdef FATFS
	int dir_fd = -1;
	if ((dir_fd = fat_opendir(dir)) >= 0) {
		db_msg("success to open %s",dir);
		fat_closedir(dir_fd);
	} else {
		db_msg("fail to open %s, now create",dir);
		if (fat_mkdir(dir) < 0) {
			ret = -1;
			db_msg("fail to mkdir %s, error:%s", dir, strerror(errno));
		}
	}
#endif
	return ret;
}

int StorageManager::checkDirs()
{
	int retval = 0;

	return retval;
}

inline void addSingleQuotes(std::string &str)
{
	std::string tmp("'");
	tmp += str;
	tmp += ("'");
	str = tmp;
}

bool StorageManager::dbIsFileExist(std::string file)
{
	return true;
}

/* Count item, such as file, time, type, info etc.*/
int StorageManager::dbCount(char *item)
{
	std::unique_lock<std::mutex> lock(mDBLock);
	return 0;
}

void StorageManager::dbGetFiles(char *item, std::vector<std::string>&file)
{

}

void StorageManager::dbGetFile(char* item, std::string &file)
{

}

int StorageManager::deleteRecentlyFile()
{
	char *ptr;
	int total, retval;
	retval = dbCount((char*)TYPE_VIDEO);
	if(retval < 0) {
		db_error("get db count failed\n");
		return -1;
	}
	total = retval;
	retval = dbCount((char*)TYPE_PIC);
	if(retval < 0) {
		db_error("get db count failed\n");
		return -1;
	}
	total += retval;
	if(total<0){
		db_msg("no files to delete");
		return -1;
	}

	std::unique_lock<std::mutex> lock(mDBLock);
	return 0;
}

int StorageManager::dbGetSortByTimeRecord(dbRecord_t &record, int index)
{

	return 0;
}
void StorageManager::dbAddFile(Elem *elem)
{

}

int StorageManager::dbDelFile(char const *file)
{

	return 0;
}

void StorageManager::dbUpdateFile(const char* oldname, const char *newname)
{
	std::unique_lock<std::mutex> lock(mDBLock);
}


void StorageManager::dbUpdateFileType(const char* name, const char *type)
{
	std::unique_lock<std::mutex> lock(mDBLock);
}

void StorageManager::setFileInfo(const char* oldName, bool isLock1 ,std::string& nfile)
{
#if 0
	db_msg(">>>>>>>>>>>>oldname:%s ,isLock: %d  ",oldName ,isLock1 );
	char fileName[40]={0};
	string file(oldName) ;
	int ret=0 ;

	if(isLock1){
		file = file.getBasePath();
		file += FLAG_SOS;
		file += EXT_VIDEO_MP4;
		db_msg("lockFile:%s --> %s", oldName, (char*)file.string());

		string newfile = string(CON_2STRING(MOUNT_PATH,LOCK_DIR))+file.getPathLeaf();
		nfile = newfile;

		dbUpdateFile(oldName ,(char*)newfile.string());
		dbUpdateFileType((char*)newfile.string(),(char*)INFO_LOCK );
		reNameFile(oldName , newfile.string());
	}else{
		file = file.getBasePath();
		ret = strlen(file.string());
		strncpy(fileName ,(char*)file.string() ,ret-4);//"_SOS" 4λ
		if( !strncmp(EXT_VIDEO_MP4 ,".mov",4)){
			fileName[ret-4]='.';
			fileName[ret-3]='m';
			fileName[ret-2]='o';
			fileName[ret-1]='v';
			fileName[ret-0]= 0 ;
		}else{
		fileName[ret-4]='.';
		fileName[ret-3]='m';
		fileName[ret-2]='p';
		fileName[ret-1]='4';
		fileName[ret-0]= 0 ;
		}

		char NewFile[40]={0};
		strncpy(NewFile,(char*)file.string(),3);
		strcat(NewFile, TYPE_VIDEO);
		strcat(NewFile, fileName+7);
		db_msg("unlockFile:%s --> %s", oldName, NewFile);	
		dbUpdateFile(oldName ,(char*)NewFile);
		dbUpdateFileType((char*)NewFile,(char*)INFO_NORMAL);
		reNameFile(oldName , (char*)NewFile);
		nfile = string(NewFile);
	}
#endif
	db_msg(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>lock or unlock  ok! NewFile:%s",nfile.c_str());
}

int StorageManager::savePicture(void* data, int size, std::string &file)
{
	int bytes_write = 0;
	int bytes_to_write = size;
	void *ptr = data;


	return 0;
}

bool StorageManager::isDiskFull(void)
{
	if(needDeleteFiles() == RET_DISK_FULL)
		return true;
	return false;
}

void StorageManager::setEventListener(EventListener* pListener)
{
	mListener = pListener;
}

void StorageManager::storageSync()
{
	db_msg(" ");
	system("sync "STORAGE_POINT);
}

#ifdef CAMDROID
StorageManager::StorageSyncThread::StorageSyncThread(StorageManager *sm, bool loop)
{
	mSM = sm;

	mLoop = loop;
}

void StorageManager::StorageSyncThread::storageSyncLoop()
{
	while(mLoop) {
		mSM->storageSync();
		usleep(3*1000*1000);
	}
	mSM->storageSync();
}
#endif
void StorageManager::startSyncThread(bool loop)
{
#ifdef CAMDROID
	db_msg(" ");
	if (mSST == NULL) {
		mSST = new StorageSyncThread(this, loop);
	}
	mSST->start();
	db_msg("!!mSST is not null");
#endif
}

void StorageManager::stopSyncThread()
{
#ifdef CAMDROID
	db_msg(" ");
	if (mSST != NULL) {
		mSST->stopThread();
		mSST.clear();
		mSST = NULL;
		return ;
	}
	db_msg("!!mSST is null");
#else
	storageSync();
#endif
}

void StorageManager::dblock(bool flag)
{
	if (flag) {
		mDBInitL.lock();
	} else {
		mDBInitL.unlock();
	}
}

void StorageManager::stopUpdate()
{
	mStopDBUpdate = true;
}
