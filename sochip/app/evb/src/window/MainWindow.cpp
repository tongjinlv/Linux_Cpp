#include <include_net/AWNetCmdData.h>
#include <include_net/AWNetInterface.h>
#include <include_wifi/wifi_ap.h>

#include "MainWindow.h"
#ifdef LOG_TAG 
#undef LOG_TAG
#endif
#define LOG_TAG "MainWindow.cpp"
#include "debug.h"

#include "posixTimer.h"
using namespace zeta;

static bool isOTAUpdate = false;
bool CdrMain::isRecording()
{
	return mRecordPreview->isRecording();
}

bool CdrMain::isPlaying()
{
	if (mPlayBack) {
		return mPlayBack->isStarted();
	}
	return false;
}

bool CdrMain::isPauseing()
{
	if (mPlayBack) {
		return mPlayBack->isPause();
	}
	return false;
}

/************* MainWindow Proc ***************/
static int CDRWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	CdrMain* mCdrMain = (CdrMain*)GetWindowAdditionalData(hWnd);

	//db_msg("--------------------message is %s\n", Message2Str(message), message);
	switch(message) {
	case MSG_FONTCHANGED:
		mCdrMain->updateWindowFont();
		break;
	case MSG_CREATE:
		break;
		/* + + + keyEvent + + + */
	case MSG_PAINT:
		#if 0
		if(mCdrMain->getIsLowPower()){
			HDC	hdc=BeginPaint(hWnd);
			BITMAP bitImage;
			char *filepath=(char *)"/etc/res/others/low_power_notice.png";
			LoadBitmapFromFile(HDC_SCREEN, &bitImage, filepath);
			int screenW,screenH;
			getScreenInfo(&screenW, &screenH);
			FillBoxWithBitmap(hdc,0,0,screenW,screenH,&bitImage);
			EndPaint(hWnd,hdc); 
			db_msg("---main window 1----\n");
		}
		#endif
		break;
	case MSG_KEYDOWN:
		if(mCdrMain->isKeyUp == true) {
			mCdrMain->downKey = wParam;	
			SetTimer(hWnd, ID_TIMER_KEY, LONG_PRESS_TIME);
			mCdrMain->isKeyUp = false;
		}
		break;
	case MSG_KEYUP:
		mCdrMain->isKeyUp = true;
		if(wParam == mCdrMain->downKey)	{
			KillTimer(hWnd, ID_TIMER_KEY);
			mCdrMain->keyProc(wParam, SHORT_PRESS);
		}
		break;
	case MSG_KEYLONGPRESS:
		mCdrMain->downKey = -1;
		db_msg("long press\n");
		mCdrMain->keyProc(wParam, LONG_PRESS);
		break;
	case MSG_TIMER:
		if(wParam == ID_TIMER_KEY) {
			mCdrMain->isKeyUp = true;			
			SendMessage(hWnd, MSG_KEYLONGPRESS, mCdrMain->downKey, 0);
			KillTimer(hWnd, ID_TIMER_KEY);
		}
		break;
	case MWM_CHANGE_WINDOW:
		mCdrMain->changeWindow(wParam, lParam);
		break;
	case MSG_CLOSE:
		DestroyMainWindow (hWnd);
		PostQuitMessage (hWnd);
		break;
	case MSG_WIFI_SWITCH:
		mCdrMain->wifiEnable((bool)wParam);
		break;
	case MSG_RESET_WIFI_PWD:
		mCdrMain->reset_wifi_flag = true;
		mCdrMain->fResetWifiPWD();
		break;
	case MSG_OTA_UPDATE:
		mCdrMain->otaUpdate();
		break;
	case MSG_DESTROY:
		break;
	case MSG_SHUT_DOWN:
		mCdrMain->shutdown();
		break;
	default:
		mCdrMain->msgCallback(message, wParam);
	}
	return DefaultWindowProc(hWnd, message, wParam, lParam);
}

static bool inline isKeyEvent(int msg)
{
	if (msg != MSG_KEYDOWN && msg != MSG_KEYUP && msg != MSG_KEYLONGPRESS) {
		return false;	
	}
	return true;
}

int MsgHook(void* context, HWND dst_wnd, int msg, WPARAM wparam, LPARAM lparam)
{
	int ret = HOOK_GOON;
	
	CdrMain *cdrMain = (CdrMain*)context;
	HWND hMainWnd = cdrMain->getHwnd();
	
	ResourceManager* rm = ResourceManager::getInstance();
	if(cdrMain->mShutdowning == true) {
		db_msg("shutdowning or resource problems\n");
		return !HOOK_GOON;
	}
	if (isKeyEvent(msg)) {
		if (wparam == CDR_KEY_POWER) 
		{  
			SendMessage(hMainWnd, msg, wparam,lparam);
			return !HOOK_GOON; 
		}
	}
	if (!cdrMain->isScreenOn()) { //screen off
		return !HOOK_GOON; //nothing to do
	}
	return ret;
}

CdrMain::CdrMain()
	: mShutdowning(false)
	, mSTB(NULL)
	, mRecordPreview(NULL)
	, mPlayBackPreview(NULL)
	, mPlayBack(NULL)
	, mMenu(NULL)
	, mMainPreview(NULL)
	, mOtherPreview(NULL)
	, mModePreview(NULL)
#ifdef HERO_OLEDUI
	, mOled(NULL)
#endif
	, mPM(NULL)
	, mEM(NULL)
	, mOtherWindowInited(false)
	, mSessionConnect(false)
	, mNeedCheckSD(false)
	, downKey(0)
	, isKeyUp(true)
	, isLongPress(false)
	, mFinishConfig(false)
	, mPCCam(false)
	, m2pcConnected(false)
	, reset_wifi_flag(false)
	, isDownLoadFlag(true)
	, PlugInHdmiPlayFlag(false)
	, PlugInEthFlag(false)
	, usbStorageConnected(false)
	, isStopPreview(false)
{
	
	struct timespec measureTime1;
	clock_gettime(CLOCK_MONOTONIC, &measureTime1);
//	db_msg("Test Time: start cdr, time is %ld secs, %ld nsecs\n", measureTime1.tv_sec, measureTime1.tv_nsec);
	
	mLowPower = EventManager::getBattery();
	if (mLowPower < 0) {
		
		mLowPower = LOW_POWER+1;
		db_msg("===no need to check battery capacity ===\n");
	} else {
		db_msg("===power capacity: %d%%/%d%%===\n", mLowPower, 100);
	}

	int batteryStatus = EventManager::getBatteryStatus();
	if(mLowPower > LOW_POWER || (batteryStatus == BATTERY_CHARGING)){
		mRecordPreview =  new RecordPreview(this);
	}
}



CdrMain::~CdrMain()
{
#ifdef APP_WIFI
	if (mCdrServer) {
		delete mCdrServer;
		mCdrServer = NULL;
	}
#endif
	if(mRecordPreview) {
		delete mRecordPreview;
		mRecordPreview = NULL;
	}
	if (mEM) {
		delete mEM;
		mEM = NULL;
	}
	if (mPM) {
		delete mPM;
		mPM = NULL;
	}
	if(mSTB) {
		delete mSTB;
		mSTB = NULL;
	}
	if(mPlayBackPreview) {
		delete mPlayBackPreview;
		mPlayBackPreview = NULL;
	}
	if(mPlayBack) {
		delete mPlayBack;
		mPlayBack= NULL;
	}
	if(mMenu) {
		delete mMenu;
		mMenu= NULL;
	}
	if(mMainPreview) {
		delete mMainPreview;
		mMainPreview = NULL;
	}
	if(mOtherPreview) {
		delete mOtherPreview;
		mOtherPreview = NULL;
	}	
	if(mModePreview) {
		delete mModePreview;
		mModePreview = NULL;
	}
#ifdef HERO_OLEDUI
	if(mOled) {
		delete mOled;
		mOled = NULL;
	}
#endif	
	UnRegisterCDRWindows();
	UnRegisterCDRControls();
	ExitGUISafely(0);
	MainWindowThreadCleanup(mHwnd);
}

bool CdrMain::isBatteryOnline()
{
	if (mEM) {
		return mEM->isBatteryOnline();
	}
	return false;
}

void CdrMain::initPreview(void* none)
{
	mInitPT = new InitPreviewThread(this);
	mInitPT->startThread();
}

int CdrMain::initPreview_thread(void)
{
	//sleep(2);
	mRecordPreview->init();
	mRecordPreview->startPreview();
	return 0;
}

void CdrMain::initStage2(void* none)
{
	mInitStage2T = new InitStage2Thread(this);
	mInitStage2T->startThread();
}

int CdrMain::wifiInit()
{
	int ret;
	char buf[100] = {0};

	if(access("/var/run/hostapd", F_OK) != 0) {
		sprintf(buf, "mkdir %s", "/var/run/hostapd");
		system(buf);
	}
	if(access("/var/run/hostapd/hostapd.conf", F_OK) != 0) {
		memset(buf, 0, sizeof(buf));
		sprintf(buf,"touch %s", "/var/run/hostapd/hostapd.conf");
		system(buf);
	}

    ret = wifi_ap_init();
    if (ret) {
        db_msg("Do wifi_ap_init fail! ret:%d\n", ret);
        return -1;
    }

	return 0;
}

int CdrMain::wifiEnable(int bEnable, const char* ssid, const char* password, const char* encryption)
{

	int ret=-1;
#ifdef APP_WIFI
	switch (bEnable)
	{
	case NET_CONNECT_MANAGER_DISABLE_SOFTAP:
		{
			//disable softap
		    int ret = wifi_ap_stop("wlan0");
		    if (ret) {
		        db_msg("Do wifi_ap_stop fail! ret:%d\n", ret);
		        return -1;
		    }
		    usleep(80);

		    ret = wifi_ap_close("wlan0");
		    if (ret) {
		        db_msg("Do wifi_ap_close fail! ret:%d\n", ret);
		    }
		    usleep(80);

		    ret = wifi_ap_exit();
		    if (ret) {
		        db_msg("Do wifi_ap_exit fail! ret:%d\n", ret);
		    }			
			break;
		}
	case NET_CONNECT_MANAGER_ENABLE_SOFTAP:
		{
			if(reset_wifi_flag == true)
			{
				reset_wifi_flag = false;
				db_msg("++++++++++++++++++\n");
				fResetWifiPWD();
			}
			int ret = 0;
		    int cnt = 0;
		    int i   = 0;
		    WIFI_AP_CFG_S ap_cfg;
			memset(&ap_cfg, 0, sizeof(ap_cfg));
			
		    ret = wifi_ap_open("wlan0");
		    if (ret) {
		        db_msg("Do wifi_ap_open fail! ret:%d\n", ret);
		        return -1;
		    }
			
			if(ssid == NULL){
				char snum[10]={0};
				StorageManager *sm = StorageManager::getInstance();
				sm->readCpuInfo(snum);
				sprintf(ap_cfg.ssid,"ActionDV_%s",snum);			
			}else{
				strncpy(ap_cfg.ssid,  ssid, sizeof(ap_cfg.ssid)-1);
			}
			if(password == NULL){
				strncpy(ap_cfg.pswd,  "12345678", sizeof(ap_cfg.pswd)-1);
			}else{
				strncpy(ap_cfg.pswd,  password, sizeof(ap_cfg.pswd)-1);
			}
	
		    strncpy(ap_cfg.bssid, "a0:0b:ba:b4:af:3e", sizeof(ap_cfg.bssid)-1);
		    ap_cfg.channel  = 9;
		    ap_cfg.security = WIFI_AP_SECURITY_WPA_WPA2_PSK;
		    ap_cfg.hidden_ssid = 0;

		    ret = wifi_ap_start("wlan0", &ap_cfg);
		    if (ret) {
		        db_msg("Do wifi_ap_start fail! ret:%d\n", ret);
		        return -1;
		    }		

			system("ifconfig wlan0 192.168.10.2");
			system("udhcpd -f /etc/udhcpd.conf&");	

			memset(wifi_ssid, 0, sizeof(wifi_ssid));
			memset(wifi_passwd, 0, sizeof(wifi_passwd));

			strcpy(wifi_ssid, ap_cfg.ssid);
			strcpy(wifi_passwd, ap_cfg.pswd);
			break;
		}
	case NET_CONNECT_MANAGER_DISABLE_CONN_WIFI:
		break;
	case NET_CONNECT_MANAGER_ENABLE_CONN_WIFI:
		break;
	default:
		break;
	}
#endif
	return ret;
}

int CdrMain::sendData_thread(void)
{
	int ret;
	int h264file = open("/mnt/extsd/hero.h264", O_CREAT | O_RDWR);
	FRAMEINFO_t frameInfo;
	AWNetInterface *mNetServer = (AWNetInterface *)mCdrServer->getSession();
	memset(&frameInfo, 0, sizeof(FRAMEINFO_t));
	char buff_head[64];
	char buff_264[1024*1024];
	memset(buff_264, 0, sizeof(buff_264));
	memset(buff_head, 0, sizeof(buff_head));
	read(h264file, buff_head, 23);
	read(h264file, buff_264, 3887);
	while(1)
	{
		if(!isDownLoadFlag){
			db_msg("send sendData_thread ing...\n");
			frameInfo.flags = IPC_FRAME_FLAG_IFRAME;				
			frameInfo.timestamp = 0;				
			ret = mNetServer->awSendVideoData(VSTREAM_P2P, (const char*)buff_head, 23, (const char*)&frameInfo, sizeof(FRAMEINFO_t), NULL);	
			if (ret == -1)				
			{					
				db_msg("send sps error");				
			}	
			usleep(5000);
			frameInfo.codec_id = MEDIA_CODEC_VIDEO_H264;	        	            
			frameInfo.flags = IPC_FRAME_FLAG_PBFRAME;	        
			frameInfo.timestamp = 0;			
			ret = mNetServer->awSendVideoData(VSTREAM_P2P, (const char*)buff_264, 3887, (const char*)&frameInfo, sizeof(FRAMEINFO_t),NULL);			
			if (ret == -1)			
			{				
				db_msg("send pps error");		
			}	
		}
		usleep(5000);
	}
	return 0;
}
#if 0
void CdrMain::onH264Data(void *data, int size)
{	
	db_msg("onH264Data\n");		
}
#endif
int CdrMain::initStage2_thread(void)
{
	StorageManager* sm;
	ResourceManager* rm;
	int result;
	int batter_capitcy= 0;
	db_msg("by hero nothing initStage2\n");
	
	/* wait for the preview is started */
	std::unique_lock<std::mutex> lck(mLock);
	mPOR.wait_for(lck, std::chrono::seconds(1));
	
	mPM = new PowerManager(this);
	
	mEM = new EventManager();
	mEM->setEventListener(this);
	mEM->init();

	sessionServerInit();
	//wifiInit();
	//wifiEnable(true);
	
	mSendData = new SendDataThread(this);
	mSendData->startThread();	

	sleep(5);
	mOtherWindowInited = true;
	SendMessage(mMainPreview->getHwnd(), MSG_MP_CHANGE_WINDOW, 0, 0);
	printf("by hero nothing createOtherWindows end\n");
	//UpdateWindow(mMainPreview->getHwnd(), true);
	db_msg("-----initStage2 end -----------------------------\n");
	return 0;
}
bool CdrMain::getNeedCheckSD()
{
	return mNeedCheckSD;
}

void CdrMain::setNeedCheckSD(bool flag)
{
	mNeedCheckSD = flag;
}

bool CdrMain::getTfCardStatus()
{
	return mEM->tf_flag;
}

void CdrMain::setTfCardStatus(bool flag)
{
	mEM->tf_flag = flag;
}

unsigned int getClustSize(char *devname)
{
    int fd;
    unsigned char block[DOSBOOTBLOCKSIZE];
    unsigned int BytesPerSec = 0;
    unsigned int SecPerClust = 0;
    unsigned int BytesPerClust = 0;
	
    fd = open(devname, O_RDONLY, 0);
    if (read(fd, block, sizeof(block) ) < sizeof(block) ) {
        db_msg("Could not read boot block\n");
        return 0;
    }
	close(fd);
	
    if (block[510] != 0x55 || block[511] != 0xaa) {
        db_msg("Invalid signature in boot block\n");
        return 0;
    }

    if(!memcmp(&block[3], "EXFAT   ", 8)){
        db_msg("exFAT filesystem is not supported\n");
        return 0;
    }

    if(!memcmp(&block[3], "NTFS", 4)){
        db_msg("NTFS filesystem is not supported\n");
        return 0;
    }
	
    if(!memcmp(&block[82], "FAT32   ", 8))
        db_msg("FAT32 filesystem is supported\n");

    BytesPerSec = block[11] + (block[12] << 8);
    if (BytesPerSec != 512 && BytesPerSec != 1024 && BytesPerSec != 2048 && BytesPerSec != 4096) {
        db_msg("Invalid sector in boot block\n");
		return 0;
    } 

    SecPerClust = block[13];
    if ((SecPerClust&(SecPerClust-1)) != 0) {
        db_msg("Invalid cluster in boot block\n");
        return 0;
    } 

    BytesPerClust = SecPerClust*BytesPerSec;
	db_msg("BytesPerClust %u\n",BytesPerClust);
	
    return BytesPerClust;
}

bool CdrMain::checkSDFs(){
	char device[256];
	const char *path = "/mnt/extsd";
	char mount_path[256];
	char rest[256];
	FILE *fp;
	int reval=0;
	char line[1024];

	if (!(fp = fopen("/proc/mounts", "r"))) {
		db_msg("Error opening /proc/mounts\n");
		return false;
	}

	while(fgets(line, sizeof(line), fp)) {
		line[strlen(line)-1] = '\0';
		sscanf(line, "%255s %255s %255s\n", device, mount_path, rest);
		if (!strcmp(mount_path, path)) {
			fclose(fp);
			goto checkfat;
		}
	}

	fclose(fp);
	db_msg("Volume is not mounted\n");
	return false;

checkfat:
	reval = getClustSize(device);
	if(reval==65536)
		return true;
	else
		return false;

}


void CdrMain::initCdrTime(){
   time_t timer;
   struct tm *tblock;
   timer = time(NULL);
   tblock = localtime(&timer);
   if(tblock->tm_year + 1900 >=2015)
   {
       return ;
   }else{
       tblock->tm_year=2015-1900;
       tblock->tm_mon=0;
       tblock->tm_mday=1;
       tblock->tm_hour=0;
       tblock->tm_min=0;
       tblock->tm_sec=0;
       if(setDateTime(tblock)!=0){
          db_msg("***init cdr time failed***\n");
       }
   }       
}

int CdrMain::sessionServerInit()
{
	int ret = 0;
	
	mCdrServer = new CdrServer(this);
	ret = mCdrServer->start();

	return ret;
}

void CdrMain::sessionChange(bool connect)
{
	db_msg("by hero nothing sessionChange\n");
}

void CdrMain::shutdown(int flag)
{
	db_msg("shutdown\n");
	ResourceManager* rm = ResourceManager::getInstance();
	mShutdowning = true;
	showShutdownLogo();
	rm->syncConfigureToDisk();
	mRecordPreview->stopPreview();
	mPlayBack->stopPlayback(false);
	mPM->powerOff();
}

void CdrMain::startConnectPC()
{
	if(usbStorageConnected == false){
		mEM->connect2PC();
		usbStorageConnected = true;
	}
}

void CdrMain::stopConnectPC()
{
	if(usbStorageConnected){
		mEM->disconnectFromPC();
		usbStorageConnected = false;
	}
}

void CdrMain::startHdmi()
{
	if(PlugInHdmiPlayFlag){
		mEM->hdmiAudioOn(true);
		mRecordPreview->HdmiInitCamer();
		mRecordPreview->otherScreen(1);
		int layer_id = 0;
		mRecordPreview->layerSwitchHdmiMode(layer_id, true);
	}
}

void CdrMain::stopHdmi()
{
#if 0
	if(!PlugInHdmiPlayFlag){
		mEM->hdmiAudioOn(false);
		mRecordPreview->layerSwitchHdmiModeinMenu(2,false);
		mRecordPreview->layerSwitchHdmiModeinMenu(9,false);
		mRecordPreview->otherScreen(0);
		int layer_id = 0;
		mRecordPreview->layerSwitchHdmiMode(layer_id, true);
		mRecordPreview->HdmiPlugOut();
	}
#endif
}

zeta::ZetaDisplay* CdrMain::get_ZetaDisp()
{
	return mRecordPreview->getDisp();
}


void CdrMain::standby()
{
	db_msg("standby\n");
	mPM->enterStandbyModem();
	mPM->screenOff();
}

//#define IGNORE_UVC_STATE
int CdrMain::notify(int message, int val)
{
	int ret = 0;
	StorageManager *smt = StorageManager::getInstance();
	ResourceManager* rm = ResourceManager::getInstance();
	if(mShutdowning == true) {
		db_info("shutting down, ignore events\n");	
		return 0;
	}

	switch(message) {
	case EVENT_CONNECT2PC:		
		{	
			m2pcConnected = true;
		}		
		break;
	case EVENT_DISCONNECFROMPC:		
		{
			m2pcConnected = false;
		}
		break;	
	case EVENT_HDMI_PLUGIN:
		{
			PlugInHdmiPlayFlag = true;
			if(getCurWindowId() == WINDOWID_OTHERPREVIEW && mOtherPreview->func_select == HDMI_FUNC){
				mEM->hdmiAudioOn(true);
				mRecordPreview->HdmiInitCamer();
				mRecordPreview->otherScreen(1);
				int layer_id = 0;
				mRecordPreview->layerSwitchHdmiMode(layer_id, true);
			}
		}	
		break;
	case EVENT_HDMI_PLUGOUT: 
		{
			PlugInHdmiPlayFlag = false;
			if(getCurWindowId() == WINDOWID_OTHERPREVIEW && mOtherPreview->func_select == HDMI_FUNC){
				mEM->hdmiAudioOn(false);
				mRecordPreview->layerSwitchHdmiModeinMenu(2,false);
				mRecordPreview->layerSwitchHdmiModeinMenu(9,false);
				mRecordPreview->otherScreen(0);
				int layer_id = 0;
				mRecordPreview->layerSwitchHdmiMode(layer_id, true);
				mRecordPreview->HdmiPlugOut();
				UpdateWindow(mOtherPreview->getHwnd(), true);
				//ShowWindow(mOtherPreview->getHwnd(), SW_SHOWNORMAL);
			}
		}
		break;
	case EVENT_MOUNT:
		db_msg("-----------EVENT_MOUNT---------SendMessage---STBM_MOUNT_TFCARD---------\n");
		break;
	case EVENT_REMOVESD:
	{
		db_msg("by hero sdcard remove\n");
		system("umount /mnt/extsd");

		SendMessage(mSTB->getHwnd(), STBM_MOUNT_TFCARD, 0, 0);
		SendMessage(mMenu->getHwnd(), STBM_MOUNT_TFCARD, 0, 0);
		StorageManager * sm = StorageManager::getInstance();
		//mNeedCheckSD = false;
		setTfCardStatus(false);
	}
		break;
	case EVENT_INSERTSD:
	{
		db_msg("by hero sdcard putin\n");
		system("mount -t vfat /dev/mmcblk0 /mnt/extsd");
		SendMessage(mSTB->getHwnd(), STBM_MOUNT_TFCARD, 1, 0);
		SendMessage(mMenu->getHwnd(), STBM_MOUNT_TFCARD, 1, 0);
		//mNeedCheckSD = true;
		setTfCardStatus(true);
		break;
	}
	case CDR_KEY_OK:
		db_msg("[notify]CDR_KEY_OK======val=%d=========",val);
		SendMessage(rm->getHwnd(WINDOWID_MAIN), val, CDR_KEY_OK, 0);
		break;
	case CDR_KEY_LEFT:
		db_msg("[notify]CDR_KEY_LEFT======val=%d=========",val);
		SendMessage(rm->getHwnd(WINDOWID_MAIN), val, CDR_KEY_LEFT, 0);
		break;
	case CDR_KEY_RIGHT:
		db_msg("[notify]CDR_KEY_RIGHT======val=%d=========",val);
		SendMessage(rm->getHwnd(WINDOWID_MAIN), val, CDR_KEY_RIGHT, 0);
		break;
	case CDR_KEY_POWER:
		db_msg("[notify]CDR_KEY_POWER======val=%d=========",val);
		SendMessage(rm->getHwnd(WINDOWID_MAIN), val, CDR_KEY_POWER, 0);
		break;
	case CDR_KEY_MODE:
		db_msg("[notify]CDR_KEY_POWER======val=%d=========",val);
		SendMessage(rm->getHwnd(WINDOWID_MAIN), val, CDR_KEY_MODE, 0);
		break;		
	}
	return ret;
}

int CdrMain::msgCallback(int msgType, unsigned int data)
{
	ResourceManager* rm = ResourceManager::getInstance();
	int idx = (int)data;
	int ret = 0;
	int ledSwitch;
	switch(msgType) {
	case MSG_RM_LANG_CHANGED:
		{
			PLOGFONT logFont;
			logFont = rm->getLogFont();
			if(logFont == NULL) {
				db_error("invalid log font\n");
				return -1;
			}
			db_msg("type:%s, charset:%s, family:%s\n", logFont->type, logFont->charset, logFont->family);
			SetWindowFont(mHwnd, logFont);
		}
		break;
	default:
		break;
	}
	return ret;
}


static void *lowPowerShutDown(void *ptx)
{
	db_error("low power shut down");
	sleep(5);
    return NULL;
}

int CdrMain::createMainWindows(void)
{
	//RegisterKeyMsgHook((void*)this, MsgHook);
	CDR_RECT rect;
	MAINWINCREATE CreateInfo;
	ResourceManager* rm = ResourceManager::getInstance();
	if(rm->initStage1() < 0) {
		db_error("ResourceManager initStage1 failed\n");
		return -1;
	}

	if(RegisterCDRControls() < 0) {
		db_error("register CDR controls failed\n");
		return -1;
	}
	if(RegisterCDRWindows() < 0) {
		db_error("register CDR windows failed\n");
		return -1;
	}

	CreateInfo.dwStyle = WS_VISIBLE;
	CreateInfo.dwExStyle = WS_EX_NONE | WS_EX_AUTOSECONDARYDC;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = CDRWinProc;

	rm->getResRect(ID_SCREEN, rect);
	CreateInfo.lx = rect.x;
	CreateInfo.ty = rect.y;
	CreateInfo.rx = rect.w;
	CreateInfo.by = rect.h;
	
	CreateInfo.iBkColor = RGBA2Pixel(HDC_SCREEN, 0x00, 0x00, 0x00, 0x00);		/* the color key */
	CreateInfo.dwAddData = (DWORD)this;
	CreateInfo.hHosting = HWND_DESKTOP;

	mHwnd = CreateMainWindow(&CreateInfo);
	if (mHwnd == HWND_INVALID) {
		db_error("create Mainwindow failed\n");
		return -1;
	}
	ShowWindow(mHwnd, SW_SHOWNORMAL);
	rm->setHwnd(WINDOWID_MAIN, mHwnd);

	if(mLowPower<=LOW_POWER ) {
       int batteryStatus = EventManager::getBatteryStatus();
       db_msg("***batteryStatus:%d***\n",batteryStatus);
       if(batteryStatus==BATTERY_DISCHARGING || batteryStatus==BATTERY_NOTCHARGING){
			MSG msg;
			pthread_t thread_id = 0;
			int err = pthread_create(&thread_id, NULL, lowPowerShutDown, NULL);
			if(err || !thread_id) {
				db_msg("Create lowPowerShutDown  thread error.\n");
				return -1;
			}
			while(GetMessage(&msg,mHwnd))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			MainWindowThreadCleanup(mHwnd);	
			return -1;
		}
	}

	mMainPreview = new MainPreview(this);
	if(mMainPreview->getHwnd() == HWND_INVALID) {
		db_error("create MainPreview failed\n");
		return -1;
	}
		
	mSTB = new StatusBar(this);
	if(mSTB->getHwnd() == HWND_INVALID) {
		db_error("create StatusBar failed\n");
		return -1;
	}	

	if(mRecordPreview->createWindow() < 0) {
		db_error("create RecordPreview Window failed\n");
		return -1;
	}

	if(rm->initStage2() < 0) {
		db_error("ResourceManager initStage2 failed\n");
		return -1;
	}
#if 1
	if(createOtherWindows() < 0) {
		db_error("Mainwindow createOtherWindows failed\n");
		return -1;
	}
#endif	

	ShowWindow(mMainPreview->getHwnd(), SW_SHOWNORMAL);
	//ShowWindow(mRecordPreview->getHwnd(), SW_SHOWNORMAL);
	
	return 0;
}

int CdrMain::createOtherWindows(void)
{

	db_msg("by hero nothing createOtherWindows start\n");
	/* create PlayBackPreview */

	mPlayBackPreview = new PlayBackPreview(this);
	if(mPlayBackPreview->getHwnd() == HWND_INVALID) {
		db_error("create PlayBackPreview Window failed\n");
		return -1;
	}
	db_msg("xxxxxxxxxxxxxxxxxxxxDEBUG------1\n");
	
	/* create PlayBack */
	mPlayBack = new PlayBack(this);
	if(mPlayBack->getHwnd() == HWND_INVALID) {
		db_error("create PlayBack Window failed\n");
		return -1;
	}
	db_msg("xxxxxxxxxxxxxxxxxxxxDEBUG----------2\n");

	/* CreateMenu */
	mMenu = new Menu(this);
	if(mMenu->getHwnd() == HWND_INVALID) {
		db_error("create PlayBack Window failed\n");
		return -1;
	}
	/* CreateOther */
	mOtherPreview = new OtherPreview(this);
	if(mOtherPreview->getHwnd() == HWND_INVALID) {
		db_error("create WifiPreview Window failed\n");
		return -1;
	}
	/* CreateMode */
	mModePreview = new ModePreview(this);
	if(mModePreview->getHwnd() == HWND_INVALID) {
		db_error("create ModePreview Window failed\n");
		return -1;
	}

#ifdef HERO_OLEDUI
	/* CreateOled */
	mOled = new OledUi(this);
	if(mOled->getHwnd() == HWND_INVALID) {
		db_error("create mOled Window failed\n");
		return -1;
	}
#endif	

	db_msg("xxxxxxxxxxxxxxxxxxxxDEBUG--------3\n");
	
	db_msg("by hero nothing createOtherWindows end\n");
	return 0;
}

void CdrMain::msgLoop(void)
{
	MSG Msg;
	while (GetMessage (&Msg, this->mHwnd)) {
		TranslateMessage (&Msg);
		DispatchMessage (&Msg);
	}
}

HWND CdrMain::getWindowHandle(unsigned int windowID)
{
	switch(windowID) {
	case WINDOWID_RECORDPREVIEW:
		return mRecordPreview->getHwnd();
		break;
	case WINDOWID_PLAYBACKPREVIEW:
		return mPlayBackPreview->getHwnd();
		break;
	case WINDOWID_PLAYBACK:
		return mPlayBack->getHwnd();
		break;
	case WINDOWID_MENU:
		return mMenu->getHwnd();
		break;
	case WINDOWID_MAINPREVIEW:
		return mMainPreview->getHwnd();
		break;		
	case WINDOWID_OTHERPREVIEW:
		return mOtherPreview->getHwnd();
		break;	
	case WINDOWID_MODEPREVIEW:
		return mModePreview->getHwnd();
		break;		
#ifdef HERO_OLEDUI
	case WINDOWID_OLEDUI:
		return mOled->getHwnd();
		break;		
#endif
	case WINDOWID_STATUSBAR:
		return mSTB->getHwnd();
		break;
	default:
		db_error("invalid window id: %d\n", windowID);
	}

	return HWND_INVALID;
}

MainWindow* CdrMain::windowID2Window(unsigned int windowID)
{
	switch(windowID) {
	case WINDOWID_RECORDPREVIEW:
		return mRecordPreview;
		break;
	case WINDOWID_PLAYBACKPREVIEW:
		return mPlayBackPreview;
		break;
	case WINDOWID_PLAYBACK:
		return mPlayBack;
		break;
	case WINDOWID_MENU:
		return mMenu;
		break;
	case WINDOWID_MAINPREVIEW:
		return mMainPreview;
		break;	
	case WINDOWID_OTHERPREVIEW:
		return mOtherPreview;
		break;		
#ifdef HERO_OLEDUI
	case WINDOWID_OLEDUI:
		return mOled;
		break;		
#endif
	case WINDOWID_STATUSBAR:
		return mSTB;
		break;
	case WINDOWID_MODEPREVIEW:
		return mModePreview;
		break;		
	default:
		db_error("invalid window id: %d\n", windowID);
	}

	return NULL;
}

void CdrMain::setCurrentWindowState(windowState_t windowState)
{
	if(mSTB != NULL)
		mSTB->setCurrentWindowState(windowState);
}

void CdrMain::setCurrentRPWindowState(RPWindowState_t status)
{
	if(mSTB != NULL)
		mSTB->setCurrentRPWindowState(status);
}

RPWindowState_t CdrMain::getCurrentRPWindowState(void)
{
	if(mSTB != NULL)
		return mSTB->getCurrentRPWindowState();

	return RPWINDOWSTATE_INVALID;
}

void CdrMain::setCurWindowId(unsigned int windowId)
{
	if(mSTB != NULL)
		mSTB->setCurWindowId(windowId);
}

unsigned int CdrMain::getCurWindowId()
{
	if(mSTB != NULL)
		return mSTB->getCurWindowId();

	return WINDOWID_INVALID;
}

void CdrMain::changeWindow(unsigned int windowID)
{
	//Mutex::Autolock _l(mLock);//-----
	HWND toHwnd;
	if (isShuttingdown() || isRecording()) {
		return ;
	}
	if ((getCurWindowId() == WINDOWID_RECORDPREVIEW) && isRecording()) {
		return ;
	}
	toHwnd = getWindowHandle(windowID);
	if(toHwnd == HWND_INVALID) {
		db_error("invalid toHwnd\n");	
		return;
	}
	db_debug("%d %d", toHwnd, windowID);
	
	/*
	 * hide curWindowID and show the toHwnd
	 * */
	ShowWindow(getWindowHandle(getCurWindowId()), SW_HIDE);

	/*
	 * csi layer overlays ui layer on hdmi mode,
	 * so there is a need to close csi layer on menu/playbackpreview window to avoid covering ui.
	 */
	 #if 0
	if (windowID == WINDOWID_MENU || windowID == WINDOWID_PLAYBACKPREVIEW) {
		int layer_id = 0;	//csi_layer
		if(windowID == WINDOWID_MENU)
			mRecordPreview->layerSwitchHdmiMode(layer_id, false);
		else
			mRecordPreview->layerSwitchHdmiMode(layer_id, false);		
	} else {
		int layer_id = 0;
		mRecordPreview->layerSwitchHdmiMode(layer_id, false);
	}
	
	if(windowID == WINDOWID_RECORDPREVIEW){
		int layer_id = 0;
		mRecordPreview->layerSwitchHdmiMode(layer_id, true);
	}else{
		int layer_id = 0;
		mRecordPreview->layerSwitchHdmiMode(layer_id, false);	
	}
	#endif
	ShowWindow(toHwnd, SW_SHOWNORMAL);

	#if 1
	if(windowID == WINDOWID_RECORDPREVIEW){
		ShowWindow(getWindowHandle(WINDOWID_STATUSBAR), SW_SHOWNORMAL);
		if(isStopPreview){
			isStopPreview = false;
			if(mRecordPreview){
				mRecordPreview->stopPreviewOutside(0);
			}
		}			
	}else{
		ShowWindow(getWindowHandle(WINDOWID_STATUSBAR), SW_HIDE);
	}
	SendNotifyMessage(toHwnd, MWM_CHANGE_FROM_WINDOW, getCurWindowId(), 0);
	db_debug(" ");

	setCurWindowId(windowID);
	
	#endif	
}

void CdrMain::changeWindow(unsigned int fromID, unsigned int toID)
{
	if(fromID == getCurWindowId()) {
		changeWindow(toID);
	}
}

void CdrMain::keyProc(int keyCode, int isLongPress)
{
	int ret;
	db_msg("key %x %d\n", keyCode, isLongPress);
	MainWindow* curWin;
	if(keyCode == CDR_KEY_POWER && isLongPress) {
		if (isLongPress) {
			CloseTipLabel();
			ret = DIALOG_OK;	//to shutdown directly
			if(ret == DIALOG_OK) {
				//				mPM->powerOff();
				shutdown();
			} else if(ret == DIALOG_CANCEL) {
				db_msg("cancel\n");	
			} else {
				db_msg("ret is %d\n", ret);	
			}
		} else {
			db_msg("xxxxxxxxxx\n");
			if (mPM) {
				mPM->screenSwitch();
			}
		}
		return;
	}
	if(mPM->getStandby_status()){ //消息处理时，关屏情况默认发送power键
		#if 1
		mPM->standbyOver();
		isAutoShutdownEnabled = true;
		#endif
		return;
	}
	
	if(!mPM->isScreenOn()){
		if (mPM) {
			mPM->screenSwitch();
			return;
		}
	}
	
	curWin = windowID2Window(getCurWindowId());
	db_msg("curWindowID is %d\n", getCurWindowId());
	if (curWin && (mOtherWindowInited==true)) {
		db_msg("xxxxxxxxxx\n");
		DispatchKeyEvent(curWin, keyCode, isLongPress);
	}
}

/*
 * DispatchKeyEvent message to mWin
 * */
void CdrMain::DispatchKeyEvent(MainWindow *mWin, int keyCode, int isLongPress)
{
	HWND hWnd;
	unsigned int windowID;
	unsigned int windowID_cur;
	windowID_cur = getCurWindowId();
	hWnd = mWin->getHwnd();
	if( hWnd && (hWnd != HWND_INVALID)) {
		if(windowID_cur == WINDOWID_RECORDPREVIEW && isWifiConnecting() == false ) {
			windowID = SendMessage(hWnd, MSG_CDR_KEY, keyCode, isLongPress);
			db_msg("------before_W %d-------now_W is %d\n", windowID, windowID_cur);
			if(windowID != windowID_cur)
				changeWindow(windowID);
		}else{
			windowID = SendMessage( hWnd, MSG_CDR_KEY, keyCode, isLongPress );		
			db_msg("-----before_W %d-------now_W is %d\n", getCurWindowId(), windowID);		
			if(windowID != getCurWindowId())			
				changeWindow(windowID);
		}
	}
}

bool CdrMain::isScreenOn()
{
	if (mPM) {
		return mPM->isScreenOn();
	}
	return true;
}

bool CdrMain::ScreenOn()
{
	if (mPM) {
		return mPM->screenOn();
	}
	return true;
}

bool CdrMain::ScreenOff()
{
	if (mPM) {
		return mPM->screenOff();
	}
	return true;
}

void CdrMain::isScreenOnOff()
{
	if (mPM) {
		db_msg("---------isScreenOnOff--------------%d\n",mPM);
		mPM->isScreenOnOff();
	}
}


void CdrMain::exit()
{
	delete mEM;
	mEM = NULL;
	delete mPM;
	mPM = NULL;
}

static unsigned char *getShutdownLogo()
{
	int fd;
	unsigned char *buf = NULL;
	int buf_size = 512*1024;

	fd = open("/dev/block/mtdblock5", O_RDWR);
    if( fd < 0) {
    	db_error("open shutlogo fail\n");
    }
	buf  = (unsigned char *)malloc(buf_size);
	read(fd, buf, buf_size);
#ifdef DEBUG_SHUTDOWN_LOGO	
	for(int i = 0; i< 100; i++) {
			db_error("buf[%d]=0x%x\n",i*4,*(unsigned int *)(buf+i*4));
	}
#endif
	return buf;

}

void CdrMain::showShutdownLogo(void)
{
	HWND hStatusBar, hMenu, hPBP;
	unsigned char *buf = NULL;

	hStatusBar = getWindowHandle(WINDOWID_STATUSBAR);
	hMenu = getWindowHandle(WINDOWID_MENU);
	hPBP = getWindowHandle(WINDOWID_PLAYBACKPREVIEW);
	ShowWindow(hStatusBar, SW_HIDE);
	SendMessage(hStatusBar, MSG_CLOSE, 0, 0);	//cancel statusbar timer
	ShowWindow(hMenu, SW_HIDE);
	ShowWindow(hPBP, SW_HIDE);
	buf = getShutdownLogo();
	
	CloseTipLabel();
    showImageWidget(mHwnd, IMAGEWIDGET_POWEROFF, buf);
	  free(buf);

}

bool CdrMain::isShuttingdown(void)
{
	return mShutdowning;
}

void CdrMain::setSTBRecordVTL(unsigned int vtl)
{
	
}

int CdrMain::camChange(bool session, int param_in)
{
	int ret = 0;

	if (session) {
		if (param_in < CAM_CNT) {
			db_msg("uvc\n");
		}
	} else {
		if (param_in) {
			ret = mRecordPreview->startRecord();
		} else {
			ret = mRecordPreview->stopRecord();
		}
	}
	return ret;
}

bool CdrMain::get_InitFlag()
{
	return mOtherWindowInited;
}

typedef struct __get_thumbnail_argvlist {
	int sid;
	char *buf;
	CdrServer* mCdrServer;
	PlayBackPreview *mPlayBackPreview;
} get_thumbnail_argvlist;
static void* get_thumbnail(void*argv) 
{
	get_thumbnail_argvlist* arglist = (get_thumbnail_argvlist*)argv;
	aw_cdr_get_thumbnail *req = (aw_cdr_get_thumbnail*)arglist->buf;
	aw_cdr_get_thumbnail_resp resp;
	char* filename = strrchr(req->path, '/');
	char* fileext = strrchr(req->path, '.');
	if (filename != NULL && fileext != NULL) {
		filename += sizeof('/');
		fileext += sizeof('.');
		memset(&resp, 0, sizeof(aw_cdr_get_thumbnail_resp));
		strncpy(resp.path, req->path, filename - req->path);
		strncat(resp.path, ".thumb/", strlen(".thumb/"));
		strncat(resp.path, filename, fileext - filename);
		strncat(resp.path, "bmp", strlen("bmp"));
		resp.result = arglist->mPlayBackPreview->getBmpPicFromVideo(req->path, resp.path);
		db_msg("NAT_CMD_GET_THUMBNAIL result:%d, path:%s", resp.result, resp.path);
	} else {
		resp.result = -1;
	}
	arglist->mCdrServer->reply(arglist->sid, NAT_CMD_GET_THUMBNAIL_RESP, 
		(char *)(&resp), sizeof(aw_cdr_get_thumbnail_resp));
	delete [] arglist->buf;
	free(arglist);
	return NULL;
}

int CdrMain::sessionControl(int sid, int cmd, char* buf)
{	
    db_msg(" ");
	
	ResourceManager* rm = ResourceManager::getInstance();
	int ret;
	switch (cmd) {
		case  NAT_CMD_GET_CONFIG:
			rm->setRecordingStatus(mRecordPreview->isRecording());
			break;
		case NAT_CMD_CHANGE_CAM:
			{
				int param_in = ((aw_cdr_change_cam*)buf)->index;
				aw_cdr_change_cam_resp resp;
				resp.result = camChange(true, param_in);
				return mCdrServer->reply(sid, NAT_CMD_CHANGE_CAM_RESP, 
					(char *)(&resp), sizeof(resp));
			}
		case NAT_CMD_RECORD_ON_OFF:
			{
				int current1;
				HWND hwnd;
				int param_in = ((aw_cdr_recorder_onoff*)buf)->index;
				aw_cdr_recorder_onoff_resp resp;
				db_msg("---------[debug_jason]:this is NAT_CMD_RECORD_ON_OFF the param is %d ---sid: %d-- buffer:%s----\n",param_in, sid, buf);
				//add by jason
				if(mRecordPreview->mRPWindowState != STATUS_RECORDPREVIEW)
				{
					mRecordPreview->mRPWindowState = STATUS_RECORDPREVIEW;
					setCurrentRPWindowState(STATUS_RECORDPREVIEW);
				}
				if(param_in == 1)
				{
					db_msg("---------[debug_jason]:this is NAT_CMD_RECORD_ON_OFF the param is %d xxx ---------\n",param_in);
					//mRecordPreview->stopRecord(CAM_CSI_NET);
					hwnd = getWindowHandle(WINDOWID_RECORDPREVIEW);
					current1 = rm->getSubMenuCurrentIndex(ID_MENU_LIST_VIDEO_RESOLUTION);
					SendMessage(hwnd,MSG_RM_VIDEO_RESOLUTION,current1,0);
					current1 = rm->getSubMenuCurrentIndex(ID_MENU_LIST_VIDEO_DELAYP);
					SendMessage(hwnd,MSG_RM_VIDEO_DELAYP,current1,0);
					//current1 = rm->getSubMenuCurrentIndex(ID_MENU_LIST_VIDEO_BITRATE);
					//SendMessage(hwnd,MSG_RM_VIDEO_BITRATE,current1,0);
					//mRecordPreview->startRecord(CAM_CSI_NET);
					
				}
				
				resp.result = camChange(false, param_in);
				return mCdrServer->reply(sid, NAT_CMD_RECORD_ON_OFF_RESP, 
					(char *)(&resp), sizeof(resp));
			}
		case NAT_CMD_SLOW_RECORD_ON_OFF:
			{
				int current2;
				HWND hwnd;
				int param_in = ((aw_cdr_recorder_onoff*)buf)->index;
				db_msg("---------[debug_jason]:this cmd is NAT_CMD_SLOW_RECORD_ON_OFF ---------\n");
				db_msg("---------[debug_jason]:the param is %d ---------\n",param_in);
				aw_cdr_recorder_onoff_resp resp;
				//add by jason
				//slow_record_flag
				if(mRecordPreview->mRPWindowState != STATUS_AWMD)
				{
					mRecordPreview->mRPWindowState = STATUS_AWMD;
					setCurrentRPWindowState(STATUS_AWMD);
				}
				
				if(param_in == 1)
				{
					db_msg("---------[debug_jason]:the param is   xxxx %d ---------\n",param_in);
					//mRecordPreview->stopRecord(CAM_CSI_NET);
					hwnd = getWindowHandle(WINDOWID_RECORDPREVIEW);
					current2 = rm->getSubMenuCurrentIndex(ID_MENU_LIST_VIDEO_SLOWP);
					SendMessage(hwnd,MSG_RM_VIDEO_SLOWP,current2,0);
					//mRecordPreview->startRecord(CAM_CSI_NET);
				}
				
				resp.result = camChange(false, param_in);
				return mCdrServer->reply(sid, NAT_CMD_SLOW_RECORD_ON_OFF_RESP, 
					(char *)(&resp), sizeof(resp));
			}
		case NAT_CMD_SET_VIDEO_SIZE:
			{
				aw_cdr_set_video_quality_index *p = (aw_cdr_set_video_quality_index*)buf;
				aw_cdr_set_video_quality_index_resp resp;
				resp.result = 0;
				//mRecordPreview->changeResolution(p->video_quality_index);
				return mCdrServer->reply(sid, NAT_CMD_SET_VIDEO_SIZE_ACK, 
					(char *)(&resp), sizeof(resp));
			}
		case NAT_CMD_NET_CONNECT_STATE:
			{
				int stat = ((aw_cdr_net_state*)buf)->index;
				if (stat == 1)
				{
					mSessionConnect = true;
					/*
					if(mRecordPreview->isRecording()== true)
					{
						mRecordPreview->stopRecord();
					}
					*/
					//SendMessage(mHwnd, MSG_SESSIONCONNECT, 0, 0);
				}
				else
				{
					mSessionConnect = false;
					//SendMessage(mHwnd, MSG_SESSIONDISCONNECT, 0, 0);
				}
			}
			break;
		case AW_IOTYPE_USER_IPCAM_ON_LINE:
			break;
		case AW_IOTYPE_USER_IPCAM_OFF_LINE:
			mSessionConnect = false;
			//SendMessage(mHwnd, MSG_SESSIONDISCONNECT, 0, 0);
			break;
		case NAT_CMD_GET_THUMBNAIL:
			{
				pthread_t threadid;
				get_thumbnail_argvlist *argvList = (get_thumbnail_argvlist*)malloc(sizeof(get_thumbnail_argvlist));
				argvList->sid = sid;
				argvList->buf = new char [sizeof(aw_cdr_get_thumbnail_resp)];
				memcpy(argvList->buf, buf, sizeof(aw_cdr_get_thumbnail_resp));
				argvList->mCdrServer = mCdrServer;
				argvList->mPlayBackPreview = mPlayBackPreview;
				pthread_create(&threadid, NULL, get_thumbnail, argvList);
				pthread_detach(threadid);
			}
			break;
		case NAT_CMD_STOP_NET_RECORD_PREVIEW:
			{
			db_msg("[debug_jaosn]:this is stop record\n");
			isDownLoadFlag = true;
			aw_cdr_stop_net_record_resp resp;
			resp.result = 0;
			sessionChange(false);	//when is upload stop net record
			return mCdrServer->reply(sid, NAT_CMD_STOP_NET_RECORD_PREVIEW_RESP, 
					(char *)(&resp), sizeof(resp));
			}
			break;
		case NAT_CMD_START_NET_RECORD_PREVIEW:
			{
			isDownLoadFlag = false;
			db_msg("[debug_jaosn]:this is start record\n");
			aw_cdr_start_net_record_resp resp;
			resp.result = 0;
			sessionChange(true);	//when is upload finish start net record
			return mCdrServer->reply(sid, NAT_CMD_START_NET_RECORD_PREVIEW_RESP, 
			(char *)(&resp), sizeof(resp));	
			}
			break;
		default:
			break;
	}
	db_msg(" ");
	return rm->sessionControl(sid, cmd, buf, mCdrServer);
}

int CdrMain::sessionNotify(int cmd, char* param_in, int param_len, int sid)
{
	int ret = 0;
#ifdef APP_WIFI
	if (mCdrServer) {
		ret = mCdrServer->notify(cmd, param_in, param_len, sid);
	}
#endif
	return ret;
}

bool CdrMain::isWifiConnecting()
{
	return mSessionConnect;
}

void CdrMain::otaUpdate()
{
	db_msg("***getpid=%d***\n",getpid());
    if(mRecordPreview->isRecording())
		mRecordPreview->stopRecord();
	mRecordPreview->stopPreview();
	mPM->enterStandby(getpid());
}

void CdrMain::fResetWifiPWD()
{
	//by hero nothing
}

bool CdrMain::getIsLowPower()
{
	if(mLowPower<=LOW_POWER) {
		return TRUE;
	}else{
		return false;
	}
}

StatusBar * CdrMain::getStatusBar(){
	return mSTB;
}

void CdrMain::startPCCam()
{
	if(mPCCam == false){
		mPCCam = true;
		mRecordPreview->startPCCam();		
	}
}

void CdrMain::stopPCCam()
{
	if(mPCCam){
		mRecordPreview->stopPCCam();
		mPCCam = false;
	}

}

bool CdrMain::getPCCamMode()
{
	return mPCCam;
}

int CdrMain::releaseResource()
{
       return 0;
}

RecordPreview * CdrMain::getRecordPrevew()
{
	return mRecordPreview;
}

void CdrMain::stopPreviewOutside(int flag)
{
	if(flag){
		if(mRecordPreview){
			mRecordPreview->stopPreviewOutside(flag);
		}
	}else{
		isStopPreview = true;
	}
}

int CDRMain(int argc, const char *argv[], CdrMain* cdrMain)
{
	struct timespec measureTime1;
	clock_gettime(CLOCK_MONOTONIC, &measureTime1);
	db_msg("Test Time: int CDRMain time is %ld secs, %ld nsecs\n", measureTime1.tv_sec, measureTime1.tv_nsec);
	
	if(cdrMain->createMainWindows() < 0) {
		delete cdrMain;
		return -1;
	}
	clock_gettime(CLOCK_MONOTONIC, &measureTime1);
	db_msg("Test Time: after createMainWindows time is %ld secs, %ld nsecs\n", measureTime1.tv_sec, measureTime1.tv_nsec);

	cdrMain->initStage2(NULL);
	clock_gettime(CLOCK_MONOTONIC, &measureTime1);
	db_msg("Test Time: after initStage2 time is %ld secs, %ld nsecs\n", measureTime1.tv_sec, measureTime1.tv_nsec);

	cdrMain->msgLoop();

	delete cdrMain;
	return 0;
}

#ifdef _MGRM_THREADS
#include <minigui/dti.c>
#endif
