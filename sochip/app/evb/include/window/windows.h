#ifndef _WINDOWS_H
#define _WINDOWS_H
#include <platform.h>
#include <map>
#include <condition_variable>
#include <utils/Thread.h>
#include <ThreadDefs.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>


#define DE2


#include "RecordPreview.h"
#include "PlayBack.h"

#include "PowerManager.h"
#include "StorageManager.h"
#include "ResourceManager.h"
#include "cdr_widgets.h"
#include "CdrIcon.h"
#include "CdrLabel.h"
#include "EventManager.h"
#include "CdrServer.h"
#include "CdrTimer.h"
#include "debug.h"

//(2,0)
#define UI_LAY_ID 8
#define CAM_CNT 1
#define AFTIMEMS 0


#if 1//by hero camera
#include <ZetaCameraInterface.h>
#include <ZetaMediaPlayInterface.h>
#endif
static bool slow_record_flag = false;

class CdrServer;
class MainWindow;
typedef struct tPic {
    char* data;
    int data_len;
} tPic;


enum cmd_e {
	CMD_CLOSE_LAYER = 1,
	CMD_STOP_PREVIEW = 2,
	CMD_NO_NEED_STOP_PREVIEW = 4,
	CMD_CLEAN_LAYER = 8,
};

enum playerState {
	STATE_ERROR					= -1,
	STATE_IDLE					= 0,
	STATE_PREPARING				= 1,
	STATE_PREPARED				= 2,
	STATE_STARTED				= 3,
	STATE_PAUSED				= 4,
	STATE_PLAYBACK_COMPLETED	= 5,
	STATE_STOPPED				= 6,
	STATE_END					= 7
};

enum {
	FORBID_NORMAL = 0,
	FORBID_FORMAT,
};

enum AvaliableMenu{
	MENU_OBJ_START = 1,
	MENU_OBJ_SYSTEM = MENU_OBJ_START,
	MENU_OBJ_RECORDPREVIEW,
	MENU_OBJ_PHOTO,
	MENU_OBJ_CAMERA,
	MENU_OBJ_QUIT,
	MENU_OBJ_SETTINGS,
	MENU_OBJ_END
};

enum {
	WIFI_FUNC = 0,
	WEBCAM_FUNC,
	USB_FUNC,
	HDMI_FUNC,
	ETHERNET_FUNC,
	RS485_FUNC
};

#define WINDOW_STATUSBAR		"CDRStatusBar"
#define WINDOW_RECORDPREVIEW	"RecordPreview"
#define WINDOW_MENU				"CDRMenu"
#define WINDOW_PLAYBACKPREVIEW	"PlayBkPreview"
#define WINDOW_PLAYBACK			"PlayBack"
#define WINDOW_MAINPREVIEW           "MainPreview"
#define WINDOW_OTHERPREVIEW           "OtherPreview"
#define WINDOW_MODEPREVIEW           "ModePreview"


#ifdef HERO_OLEDUI
#define WINDOW_OLEDUI           "OledUi"
#endif
#define DOSBOOTBLOCKSIZE 		512

#define SELF_HAS_PROCESS        0
#define WANT_SYSTEM_PROCESS     1

#define UPDATE_FILE_PATH               MOUNT_PATH"full_img.fex"

extern int RegisterCDRWindows(void);
extern void UnRegisterCDRWindows(void);

typedef enum {
	STATUS_RECORDPREVIEW = 1,
	STATUS_PHOTOGRAPH = 2,
	STATUS_AWMD	= 3,
	RPWINDOWSTATE_INVALID,
}RPWindowState_t;

typedef enum {
	PARK_MON_VTL_10S = 10,
	PARK_MON_VTL_15S = 15,
	PARK_MON_VTL_20S = 20,
	PARK_MON_VTL_30S = 30,
}PARK_MON_VTL_t;




typedef struct {
	unsigned int curWindowID;
	RPWindowState_t RPWindowState;
}windowState_t;

class CdrMain;
class PowerManager;
class MainWindow
{
public:
	MainWindow(){mHwnd = 0;}
	virtual ~MainWindow(){};
	HWND getHwnd() {
		return mHwnd;
	}

	void updateWindowFont(void)
	{
		HWND hChild;

		hChild = GetNextChild(mHwnd, 0);
		while( hChild && (hChild != HWND_INVALID) )
		{
			SetWindowFont(hChild, GetWindowFont(mHwnd));
			hChild = GetNextChild(mHwnd, hChild);
		}
	}
	void showWindow(int cmd) {
		ShowWindow(mHwnd, cmd);
	}
	void keyProc(int keyCode, int isLongPress);
	virtual int releaseResource()=0;
protected:
	HWND mHwnd;
};

class StatusBar : public MainWindow
{
public:
	StatusBar(CdrMain* cdrMain);
	~StatusBar();
	int createSTBWidgets(HWND hParent);
	void msgCallback(int msgType, unsigned int data);
	void setStatusBarIcon(ResourceID id, unsigned int index);
	void setCurrentWindowState(windowState_t windowState);
	void setCurrentRPWindowState(RPWindowState_t status);
	RPWindowState_t getCurrentRPWindowState(void);
	void setCurWindowId(unsigned int windowId);
	unsigned int getCurWindowId();
	void updateWindowPic(void);
	int releaseResource();
	HWND mHwndTop, mHwndBottom;	
	CdrIconOps* mIconOps;
	CdrLabelOps* mLabelOps;	
private:
	CdrMain* mCdrMain;
	windowState_t mCurWindowState;

	gal_pixel mFgColor;
	gal_pixel mBgColor;
	void changeStatusBarAlpha(void);
};

class RecordPreview : public MainWindow
{
public:
	RecordPreview(CdrMain* cdrMain);
	~RecordPreview();
	void init();
	int createWindow();
	int startRecord();
	int stopRecord();
	void startPreview();
	void stopPreview();
	void HdmiInitCamer();
	void HdmiPlugOut();
	int HandleTouchMsg(int keyCode);
	bool isRecording();
	bool isPreviewing();
	int keyProc(int keyCode, int isLongPress);
	int msgCallback(int msgType, int idx);	
	void otherScreen(int screen);
	void restoreZOrder();
	int releaseResource();
	void startPCCam();
	void stopPCCam();
	void setRectBig(int x,int y,int w,int h);
	void layerSwitchHdmiMode(int hlay, bool bSwitch);
	void layerSwitchHdmiModeinMenu(int hlay,bool flag);
	int initCamera();
	zeta::ZetaDisplay* getDisp();
	void stopPreviewOutside(int flag);
	RPWindowState_t mRPWindowState, mOldRPWindowState;
	HWND hStatusBar;
	bool show_statusbar;
	int rp_select;
	CDR_RECT rp_rect[3];
private:
	CdrMain *mCdrMain;
	bool HdmiPInMotionPhotos;
	ZETA_RECT rectBig, rectSmall;
	bool mHDMIon;
	bool isRecorder;
	bool Recording;
	zeta::ZetaCamera* mZetaCamera;
};

class PlayBackPreview : public MainWindow
{
public:
	PlayBackPreview(CdrMain* pCdrMain);
	~PlayBackPreview();
	int HandleTouchMsg(int keyCode);
	int ShowMainPreview();
	int keyProc(int keyCode, int isLongPress);
	int createSubWidgets(HWND hWnd);
	int getBmpPicFromVideo(const char* videoFile, const char* picFile);
	int releaseResource();
	
	const char *image_path[3];
	CDR_RECT wp_rect[3];
	int mCurID;
	HWND mMainHwnd;
	int wp_select;
private:
	CdrMain* mCdrMain;
	BITMAP bmpIcon;
};

class PlayBack : public MainWindow
{
public:
	PlayBack(CdrMain *cdrMain);
	~PlayBack();
	int createSubWidgets(HWND hWnd);
	void cdrplaybackicon(HWND hWnd,ResourceID mId,int iCmdShow);
	int keyProc(int keyCode, int isLongPress);
	void setDisplay(int hlay);
	int preparePlay(std::string filePath);
	int PreparePlayBack(std::string filePath);
	int startPlayBack();
	void start();
	void pause();
	void seek(int msec);
	void stop();
	void release();
	void reset();
	playerState getCurrentState();
	void initProgressBar(void);
	void resetProgressBar(void);
	void updateProgressBar(void);
	bool isStarted();
	bool isPause();
	void noWork(bool idle);
	void prepareCamera4Playback();
	int stopPlayback(bool bRestoreAlpha=true);
	void sendToPBP(int mId);
	int mCurId;
	void setPlayerVol(int val);
	void complete();
#ifdef PLATFORM_0
	int restoreAlpha();
#endif
	void stopPlaying();
	class PlayBackListener : public iZetaMediaPlayer::iOnPreparedListener,
							 public iZetaMediaPlayer::iOnCompletionListener{	
		public:
			PlayBackListener(PlayBack *pb){mPB = pb;};
			void onPrepared(iZetaMediaPlayer *PiZetaMediaPlayer);
			void onCompletion(iZetaMediaPlayer *pIZetaMediaPlayer);
		private:
			PlayBack *mPB;
	};
	int releaseResource();
	int Fastforwardandrewind(bool flag,int keystate);
public:
	playerState mCurrentState;
	playerState mTargetState;
	/* if user send the msg MSG_PLB_COMPLETE with wParam 0, 
	 * then mStopFlag will be set to 1, indicate onCompletion not send MSG_PLB_COMPLETE again*/
	unsigned int mStopFlag;	
	enum PlayStatus{
		PlayStart,
		PlayStop,
		PlayRewind,
		PlayForward
	};
	void PlayBackPlayStatus(PlayStatus Status);
	HWND Hplaystart,Hplaystop,Hplayrewind,Hpalyforward;
private:
	CdrMain *mCdrMain;
	iZetaMediaPlayer *mIZetaMediaPlayer;
	PlayBackListener *mPlayBackListener;
	BITMAP bmpIcon;
};

class Menu : public MainWindow
{
public:
	Menu(CdrMain* pCdrMain);
	~Menu();
	int releaseResource();
	int keyProc(int keyCode, int isLongPress);
	CdrMain * getCdrMain();
private:
	CdrMain* mCdrMain;
	HWND mHwndIndicator;
};

typedef enum {
	TIMER_LOWPOWER_IDX,
	TIMER_NOWORK_IDX,
	TIMER_NOWORK_IDX2,
	TIMER_RECORD_IDX,
}timer_idx_enum;

typedef enum{
	KEY_SOUNDS = 1,
	TAKEPIC_SOUNDS,
	SHUTTER_3,
	PARKNOTICE = 4,
	SHUTTER_5,
	SHUTTER_10,
}sounds_type;

typedef enum {
	NET_CONNECT_MANAGER_DISABLE_SOFTAP,
	NET_CONNECT_MANAGER_ENABLE_SOFTAP,
	NET_CONNECT_MANAGER_DISABLE_CONN_WIFI,
	NET_CONNECT_MANAGER_ENABLE_CONN_WIFI,
}NET_CONNECT_MANAGER_TYPE;

class MainPreview : public MainWindow
{
public:	
	MainPreview(CdrMain* pCdrMain);	
	~MainPreview();
	int ShowMainPreview();
	int keyProc(int keyCode, int isLongPress);	
	int createSubWidgets(HWND hWnd);
	int releaseResource();
	int HandleTouchMsg(int keyCode);
	const char *image_path_[21];
	CDR_RECT mp_rect[21];
	int mp_select;
	HWND mMainHwnd;
private:	
	CdrMain* mCdrMain;	
};

class ModePreview : public MainWindow
{
public:	
	ModePreview(CdrMain* pCdrMain);	
	~ModePreview();
	int ShowModePreview(int keyCode);
	int keyProc(int keyCode, int isLongPress);	
	int createSubWidgets(HWND hWnd);
	int releaseResource();
	int HandleTouchMsg(int keyCode);
	const char *image_path_[21];
	CDR_RECT mp_rect[21];
	int mp_select;
	HWND mMainHwnd;
    BITMAP back_image;
private:	
	CdrMain* mCdrMain;	
};


class OtherPreview : public MainWindow
{
public:	
	OtherPreview(CdrMain* pCdrMain);	
	~OtherPreview();	
	int keyProc(int keyCode, int isLongPress);	
	int createSubWidgets(HWND hWnd);
	int releaseResource();
	int ShowOtherPreview(int type);
	int ShowUsbPreview();
	int ShowHdmiPreview();
	int ShowEthernetPreview();
	int ShowWebcamPreview();
	int ShowRS485Preview();
	int ShowWifiPreview();
	int HandleTouchMsg(int keyCode);
	void wifiEnable();
	void wifiDisable();
	void usbDisable();
	void usbEnable();
	void WebcamDisable();
	void WebcamEnable();
	void HdmiDisable();
	void HdmiEnable();
	char* Get_IpAddr();
	char *ip_addr;
	BITMAP png_image_[3];
	const char *image_path[3];
	const char *func_path[12];
	CDR_RECT wp_rect[3];
	CDR_RECT rback_bt;
	int wp_select;
	int func_select;
	HWND mMainHwnd;
	HWND mWifiHwnd;
	HWND mFuncHwnd;
	bool wifi_open;
	const char *wifi_ssid;
	const char *wifi_passwd;
private:	
	CdrMain* mCdrMain;	
};



#ifdef HERO_OLEDUI
class OledUi : public MainWindow
{
public:	
	OledUi(CdrMain* pCdrMain);	
	~OledUi();	
	int keyProc(int keyCode, int isLongPress);	
	int createSubWidgets(HWND hWnd);
	int releaseResource();
	int getBatteryLevel();
	bool getTfCardStatus();
	bool isBatteryOnline();
	bool getHdmiStatus();
	int timer_ctrlfd;
	int bat_ctrlfd;
	int wifi_ctrlfd;
	int tf_ctrlfd;
	int mode_ctrlfd;
	int rec_ctrlfd;
	int pic_ctrlfd;
	int screen_ctrlfd;
	std::mutex mLockOledData;
private:	
	CdrMain* mCdrMain;	
};
#endif

class CdrMain : public MainWindow, EventListener
{
public:
	CdrMain();
	~CdrMain();
	void initPreview(void* none);
	void initStage2(void* none);
	int msgCallback(int msgType, unsigned int data);
	void exit();
	int createMainWindows(void);
	int createOtherWindows(void);
	void msgLoop();
	HWND getWindowHandle(unsigned int windowID);
	MainWindow* windowID2Window(unsigned int windowID);
	void setCurrentWindowState(windowState_t windowState);
	void setCurrentRPWindowState(RPWindowState_t status);
	RPWindowState_t getCurrentRPWindowState(void);
	void setCurWindowId(unsigned int windowId);
	unsigned int getCurWindowId();
	void changeWindow(unsigned int windowID);
	void changeWindow(unsigned int fromID, unsigned int toID);
	void DispatchKeyEvent(MainWindow *mWin, int keyCode, int isLongPress);
	void keyProc(int keyCode, int isLongPress);
	int notify(int message, int data);
	bool isScreenOn();
	bool ScreenOn();
	bool ScreenOff();
	void isScreenOnOff();
	void shutdown(int flag = false);
	void standby();
	void noWork(bool idle);
	bool isRecording();
	bool isPlaying();
	bool isPauseing();
	bool isBatteryOnline();
	void showShutdownLogo();
	bool isShuttingdown();
	bool mShutdowning;
	void setSTBRecordVTL(unsigned int vtl);

/* NAT */
	int wifiInit();
	int wifiEnable(int bEnable, const char* ssid = NULL, const char* password = NULL, const char* encryption = NULL);
	void sessionChange(bool connect);
	int sessionControl(int sid, int cmd, char* buf);
	int sessionServerInit();
	int sessionNotify(int cmd, char* buf, int param_len, int sid);
	int camChange(bool session, int param_in);
	bool isWifiConnecting();
	void otaUpdate();
	bool getNeedCheckSD();
	void setNeedCheckSD(bool flag);
    bool getTfCardStatus();
    void setTfCardStatus(bool flag);
	void startPCCam();
	void stopPCCam();
	bool getPCCamMode();
	void startConnectPC();
	void stopConnectPC();
	void startHdmi();
	void stopHdmi();
	void stopPreviewOutside(int flag);
	bool get_InitFlag();
	zeta::ZetaDisplay* get_ZetaDisp();
	class InitPreviewThread : public Thread
	{
	public:
		InitPreviewThread(CdrMain* pContext) 
			: mWindowContext(pContext){
		}
		bool threadLoop() {
			mWindowContext->initPreview_thread();	
			return false;
		}

		void startThread(){
			run("InitPreviewThread", PRIORITY_NORMAL);
		}

	private:
		CdrMain* mWindowContext;
	};
	
	class InitStage2Thread : public Thread
	{
	public:
		InitStage2Thread(CdrMain* pContext) 
			: mWindowContext(pContext){
		}

		bool threadLoop() {
			mWindowContext->initStage2_thread();	
			return false;
		}

		void startThread(){
			run("InitStage2Thread", PRIORITY_NORMAL);
		}

	private:
		CdrMain* mWindowContext;
	};
	
	class SendDataThread : public Thread
	{
	public:
		SendDataThread(CdrMain* pContext) 
			: mWindowContext(pContext){
		}
		
		bool threadLoop() {
			//mWindowContext->sendData_thread();	
			return false;
		}
		
		void startThread(){
			run("SendDataThread", PRIORITY_NORMAL);
		}
		
	private:
		CdrMain* mWindowContext;
	};

	void fResetWifiPWD();
	void initCdrTime();
	bool checkSDFs();
	bool getIsLowPower();
	StatusBar * getStatusBar();

	int releaseResource();
	RecordPreview * getRecordPrevew();
	PowerManager *mPM;
	char wifi_ssid[32];
	char wifi_passwd[32];
	bool isStopPreview;
private:
	
	int initPreview_thread();
	int initStage2_thread();
	int sendData_thread();
	StatusBar *mSTB;
	RecordPreview *mRecordPreview;
	PlayBackPreview* mPlayBackPreview;
	PlayBack *mPlayBack;
	Menu* mMenu;
	MainPreview* mMainPreview;
	OtherPreview* mOtherPreview;
	ModePreview* mModePreview;
#ifdef HERO_OLEDUI
	OledUi* mOled;
#endif
	InitPreviewThread *mInitPT;
	InitStage2Thread *mInitStage2T;
	SendDataThread *mSendData;
//	PowerManager *mPM;
	EventManager *mEM;
	std::condition_variable mPOR;
	std::mutex mLock;
	CdrServer *mCdrServer;
	bool mSessionConnect;
	int mLowPower;
	bool mNeedCheckSD;
	bool mPCCam;
	bool isAutoShutdownEnabled;
	bool usbStorageConnected;
	bool mOtherWindowInited;
public:
	WPARAM downKey;
	bool isKeyUp;
	bool isLongPress;
	bool mFinishConfig;
	bool forword_rewind_flag;
	bool reset_wifi_flag;
	bool isDownLoadFlag;
	bool PlugInHdmiPlayFlag;
	bool PlugInEthFlag;
	bool m2pcConnected;
};

class PowerManager
{
public:
	PowerManager(CdrMain *cm);
	~PowerManager();

	void setOffTime(unsigned int time);
	void pulse();
	void powerOff();
	int screenOff();
	int screenOn();
	int poweroff_thread();
	int readyToOffScreen();
	void screenSwitch();
	void isScreenOnOff();
	int setBatteryLevel();
	int getBatteryLevel(void);
	void reboot();
	bool isScreenOn();
	void adjustCpuFreq();
	int getStandby_status();
	void setStandby_status(bool flag);
	void connect2StandbyService();
	void enterStandby();
	void watchdogRun();
	void watchdogThread();
	void enterStandby(int pid=-1);
	void enterStandbyModem();
	int standbySuspend();
	int standbyResume();
	void standbyOver();
		
	class ScreenSwitchThread : public Thread
	{
	public:
		ScreenSwitchThread(PowerManager *pm)
			: mPM(pm)
		{
		}
		void startThread()
		{
			run("ScreenSwitchThread", PRIORITY_NORMAL);
		}
		void stopThread()
		{
			requestExitAndWait();
		}
		bool threadLoop()
		{
			mPM->readyToOffScreen();
			return true;
		}
	private:
		PowerManager *mPM;
	};

	class poweroffThread : public Thread
	{
	public:
		poweroffThread(PowerManager* pm) : mPM(pm){}
		~poweroffThread(){}
		virtual bool threadLoop() {
			mPM->poweroff();	
			return false;
		}

		status_t start() {
			return run("poweroffThread", PRIORITY_NORMAL);
		}

	private:
		PowerManager* mPM;
	};

	class WatchDogThread : public Thread
	{
	public:
		WatchDogThread(PowerManager *pm)
			: mPM(pm)
		{
		}
		void startThread()
		{
			run("WatchDogThread", PRIORITY_NORMAL);
		}
		void stopThread()
		{
			requestExitAndWait();
		}
		bool threadLoop()
		{
			mPM->watchdogThread();
			return false;
		}
	private:
		PowerManager *mPM;
	};


private:
	CdrMain *mCdrMain;
	ScreenSwitchThread *mSS;
	poweroffThread *mPO;
	WatchDogThread *mWD;
	int mDispFd;
	unsigned int mOffTime;
	std::mutex mLock;
	std::condition_variable mCon;
	int mState;
	int mBatteryLevel;
	int poweroff();
	bool isEnterStandby;
	bool isEnterStandby2;
	bool standbyFlag;
	bool mEnterStandby;
	bool mRestoreScreenOn;
};


#endif //_WINDOW_H
