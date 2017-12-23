#include "RecordPreview.h"
#undef LOG_TAG
#define LOG_TAG "RecordPreview"
#include "debug.h"
#include "EventManager.h"
#include "StorageManager.h"
#include "windows.h"
#include <stdlib.h>

enum {
	RP_BACK = 0,
	RP_REC ,
	RP_PIC
};

#define BT_NUM 3

int RecordPreview::keyProc(int keyCode, int isLongPress)
{
	switch(keyCode) {
		case CDR_KEY_OK:
		{
			db_msg("by hero CDR_KEY_OK\n");
			break;
		}
		case CDR_KEY_POWER:
		{
			db_msg("by hero CDR_KEY_POWER\n");
			return WINDOWID_MAINPREVIEW;
			break;
		}
		case CDR_KEY_LEFT:
		{
			db_msg("by hero CDR_KEY_LEFT\n");
			break;
		}
		case CDR_KEY_RIGHT:
		{
			db_msg("by hero CDR_KEY_RIGHT\n");
			break;
		}
	}
	return WINDOWID_RECORDPREVIEW;
	
}


void RecordPreview::setRectBig(int x,int y,int w,int h)
{
	rectBig.x = x;
	rectBig.y = y;
	rectBig.w = w;
	rectBig.h = h;
}

int RecordPreview::HandleTouchMsg(int keyCode)
{	
	if(show_statusbar){
		show_statusbar = false;	
		switch(keyCode){
			case RP_BACK:
				if(isRecording()){
					stopRecord();
				}
				SendMessage(mCdrMain->getWindowHandle(WINDOWID_STATUSBAR),STBM_SHOW_BACKBT, 0, 0);
				SendMessage(mCdrMain->getWindowHandle(WINDOWID_STATUSBAR),STBM_SHOW_HWND, WINDOWID_MAINPREVIEW, 0);
				SendMessage(mCdrMain->getHwnd(),MWM_CHANGE_WINDOW,WINDOWID_RECORDPREVIEW,WINDOWID_MAINPREVIEW); 
				break;
			case RP_REC:
				if(!mCdrMain->getTfCardStatus())
					break;
				if(!isRecording()){
					SendMessage(mCdrMain->getWindowHandle(WINDOWID_STATUSBAR),STBM_SHOW_RECBT, 1, 0);
					startRecord();
				}else{
					SendMessage(mCdrMain->getWindowHandle(WINDOWID_STATUSBAR),STBM_SHOW_RECBT, 0, 0);
					stopRecord();
				}
				break;
			case RP_PIC:
				break;
		}			
	}else{
		SendMessage(mCdrMain->getWindowHandle(WINDOWID_STATUSBAR),STBM_SHOW_HWND, WINDOWID_RECORDPREVIEW, 0);
		show_statusbar = true;
	}

	return 0;
}


int RecordPreviewProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	RecordPreview* recordPreview = (RecordPreview*)GetWindowAdditionalData(hWnd);
	db_msg("message code: [%x], %s, wParam=%d, lParam=%lu\n", message, Message2Str(message), wParam, lParam);
	switch(message) {
	case MSG_CREATE:
		break;
	case MSG_CDR_KEY:
		db_debug("key code is %d\n", wParam);
		return recordPreview->keyProc(wParam, lParam);
	case MSG_SHOWWINDOW:
		break;
	case MSG_LBUTTONDOWN: 
		{
			int pre_x = LOWORD (lParam);			
			int pre_y = HIWORD (lParam);
			db_msg("by hero MSG_LBUTTONUP x = %d, y = %d\n", pre_x, pre_y);
			for(int i = 0; i < BT_NUM; i++)
			{
				if(pre_x > recordPreview->rp_rect[i].x && pre_x < (recordPreview->rp_rect[i].x + recordPreview->rp_rect[i].w) && pre_y > recordPreview->rp_rect[i].y && (pre_y < recordPreview->rp_rect[i].y + recordPreview->rp_rect[i].h)){
					recordPreview->rp_select = i;
					break;
				}
			}		
		}
		break;
	case MSG_LBUTTONUP:		
		{
			int pre_x = LOWORD (lParam);			
			int pre_y = HIWORD (lParam);
			db_msg("by hero MSG_LBUTTONUP x = %d, y = %d\n", pre_x, pre_y);
			if(!recordPreview->show_statusbar){
				recordPreview->HandleTouchMsg(0);
				break;
			}			
			for(int i = 0; i < BT_NUM; i++)
			{
				if(pre_x > recordPreview->rp_rect[i].x && pre_x < (recordPreview->rp_rect[i].x + recordPreview->rp_rect[i].w) && pre_y > recordPreview->rp_rect[i].y && (pre_y < recordPreview->rp_rect[i].y + recordPreview->rp_rect[i].h)){
					recordPreview->rp_select = i;
					//if(i == recordPreview->rp_select)
					{
						recordPreview->HandleTouchMsg(i);
					}
					break;
				}
			}	
		}
		break;		
	case MWM_CHANGE_FROM_WINDOW:
		break;
	case MSG_TIMER:
		break;
	default:
		db_msg("---------default------\n");
		recordPreview->msgCallback(message, (int)wParam);
		break;
	}

	return DefaultWindowProc(hWnd, message, wParam, lParam);
}

int RecordPreview::initCamera()
{
	db_msg("by hero initCamera\n");
	mZetaCamera->initCamera(1920, 1080, 640, 360, 60, 10, 1, 90);
	return 0;
}

void RecordPreview::startPreview()
{
	db_msg("by hero startPreview\n");
	mZetaCamera->startPreview();
}

bool RecordPreview::isPreviewing()
{
	db_msg("by hero nothing isPreviewing\n");
	return false;
}
int RecordPreview::createWindow(void)
{
	RECT rect;
	CDR_RECT STBRect;
	ResourceManager* rm;
	HWND hParent;
	int image_count = 0;
	//HWND hStatusBar;

	rm = ResourceManager::getInstance();
	hParent = mCdrMain->getHwnd();
	GetWindowRect(hParent, &rect);
	mHwnd = CreateWindowEx(WINDOW_RECORDPREVIEW, "",
			WS_CHILD | WS_VISIBLE,
			WS_EX_TRANSPARENT | WS_EX_USEPARENTFONT,
			WINDOWID_RECORDPREVIEW,
			rect.left, rect.top, RECTW(rect), RECTH(rect),
			hParent, (DWORD)this);
	if(mHwnd == HWND_INVALID) {
		db_error("create record Preview window failed\n");
		return -1;
	}
	rm->setHwnd(WINDOWID_RECORDPREVIEW, mHwnd);

	hStatusBar = mCdrMain->getWindowHandle(WINDOWID_STATUSBAR);

	rm->getResRect(ID_STATUSBAR_ICON_BACK, rp_rect[RP_BACK]);
	rm->getResRect(ID_STATUSBAR_ICON_REC, rp_rect[RP_REC]);
	rp_rect[RP_REC].y += 388; 
	rm->getResRect(ID_STATUSBAR_ICON_PIC, rp_rect[RP_PIC]);
	rp_rect[RP_PIC].y += 388; 

	
	ShowWindow(mHwnd , SW_HIDE);
	return 0;
}

RecordPreview::RecordPreview(CdrMain *cdrMain)
	: mCdrMain(cdrMain),
	mRPWindowState(STATUS_RECORDPREVIEW),
	mOldRPWindowState(STATUS_RECORDPREVIEW),
	show_statusbar(false), 
	isRecorder(false)
	
{
	db_msg("by hero nothing RecordPreview\n");
}

RecordPreview::~RecordPreview()
{
	db_msg("by hero nothing ~RecordPreview\n");
}

void RecordPreview::init(void)
{	
	db_msg("by hero nothing init\n");
	ZETA_RECT rect;
	rect.x = 0; 
	rect.y = 0; 
	rect.w = 480; 
	rect.h = 854;
	mZetaCamera = new zeta::ZetaCamera(rect, 0);
	initCamera();	
}
static void display_setareapercent(ZETA_RECT *rect, int percent)
{
	int valid_width  = (percent*rect->w)/100;
	int valid_height = (percent*rect->h)/100;
	
	rect->x = (rect->w - valid_width) / 2;
	rect->w = valid_width;
	rect->y = (rect->h - valid_height) / 2;
	rect->h = valid_height;
}

//rect align with parent_rect on the top-right
static void display_setareapercent(ZETA_RECT *rect, const ZETA_RECT *parent_rect, int percent)
{
	int valid_width  = (percent*rect->w)/100;
	int valid_height = (percent*rect->h)/100;
	
	rect->x = (parent_rect->x + parent_rect->w) - valid_width;
	rect->w = valid_width;
	rect->y = parent_rect->y;
	rect->h = valid_height;
	db_msg("********[debug_jason]:x:%d  w:%d  y:%d  h:%d***********\n",rect->x,rect->w,rect->y,rect->h);
}


void RecordPreview::layerSwitchHdmiModeinMenu(int hlay,bool flag)
{
		
	if (mHDMIon) {
		zeta::ZetaDisplay *mZD = mZetaCamera->getDisp();
		db_msg("****************layerSwitchHdmiMode*************\n");
		if(flag){
			db_msg("layer is open\n");
			mZD->open(hlay);
		}else{
			db_msg("layer is close\n");
			mZD->close(hlay);
			mZD->hdmisetBottom();
		}
	}
}

void RecordPreview::layerSwitchHdmiMode(int hlay, bool bSwitch)
{
	if(mHDMIon){
		zeta::ZetaDisplay *mZD = mZetaCamera->getDisp();
		db_msg("****************layerSwitchHdmiMode*************\n");
		if (bSwitch) {
			db_msg("******[debug_jaosn]:the uvc is not exit  111***********\n");
			mZD->open(hlay);
			mZD->clean();
			mZD->hdmisetTop();
		} else {
			db_msg("clsoe chanle 0 layer  0\n");
		
			mZD->close(hlay);

			mZD->hdmisetBottom();
		}
	}
}

zeta::ZetaDisplay* RecordPreview::getDisp()
{
	zeta::ZetaDisplay *mZD = mZetaCamera->getDisp();
	return mZD;
}

void RecordPreview::otherScreen(int screen)
{
	zeta::ZetaDisplay *mZD = mZetaCamera->getDisp();
	int hlay_uvc = 0;
	if (mZD) {
		mZD->otherScreen(screen, hlay_uvc);
		db_msg("****************otherScreen*************\n");
	}
	if (screen == 1) {	//hdmi on
		rectBig.x = 0;
		rectBig.y = 0;
		rectBig.w = 1280; // 1024;
		rectBig.h = 720;
		rectSmall.x = 920;
		rectSmall.y = 0;
		rectSmall.w = 360;
		rectSmall.h = 240;
		display_setareapercent(&rectSmall, &rectBig, 95);
		mHDMIon = true;
	} else if (screen == 2) {
		rectBig.x = 0;
		rectBig.y = 0;
		rectBig.w = 720;
		rectBig.h = 480;
		display_setareapercent(&rectBig, 95);
		rectSmall.x = 480;
		rectSmall.y = 0;
		rectSmall.w = 240;
		rectSmall.h = 160;
		display_setareapercent(&rectSmall, &rectBig, 95);
	} else {	//hdmi off
		if(mRPWindowState != STATUS_PHOTOGRAPH)
		{
			rectBig.x = 0;
			rectBig.y = 0;
			rectBig.w = BIG_RECT_H;
			rectBig.h = BIG_RECT_W;
			rectSmall.x = 300;
			rectSmall.y = 22;
			rectSmall.w = 180;
			rectSmall.h = 102;
		}else
		{
			rectBig.x = 0;
			rectBig.y = 0;
			rectBig.w = BIG_RECT_H;
			rectBig.h = BIG_RECT_W;
			rectSmall.x = 300;
			rectSmall.y = 22;
			rectSmall.w = 180;
			rectSmall.h = 102;
		}
		mHDMIon = false;
	}
	restoreZOrder();
	db_msg("by hero nothing otherScreen\n");
}

void RecordPreview::restoreZOrder()
{
	zeta::ZetaDisplay *mZD = mZetaCamera->getDisp();

	if (mZD) {
		mZD->setRect(rectBig);
		mZD->setBottom();
	}
}

int RecordPreview::startRecord()
{
#if 1
	if(!isRecorder){
		db_msg("by hero nothing startRecord\n");
		if(access("/mnt/extsd/out.mp4", F_OK) == 0) {
			system("rm /mnt/extsd/out.mp4");
		}
		isRecorder = true;
		
		mZetaCamera->setVideoRecordParam(1920, 1080, 30, 6*1024*1024);
		mZetaCamera->prepareRecord();

		mZetaCamera->startRecord();		
	}
#endif
	return 0;
}

int RecordPreview::stopRecord()
{
#if 1
	if(isRecorder){
		db_msg("by hero nothing stopRecord\n");	
		mZetaCamera->stopRecord();
		isRecorder = false;
	}
#endif
	return 0;
}

void RecordPreview::startPCCam()
{
	system("echo 0 > /sys/class/android_usb/android0/enable");
	system("echo 1f3a > /sys/class/android_usb/android0/idVendor");
	system("echo 100e > /sys/class/android_usb/android0/idProduct");
	system("echo webcam,adb > /sys/class/android_usb/android0/functions");
	system("echo 1 > /sys/class/android_usb/android0/enable");
	system("chmod 0666 /dev/video1");
	mZetaCamera->setUvcMode(1);
}

void RecordPreview::stopPCCam()
{
	mZetaCamera->setUvcMode(0); 
	system("echo 0 > /sys/class/android_usb/android0/enable");
	system("echo 1f3a > /sys/class/android_usb/android0/idVendor");
	system("echo 1002 > /sys/class/android_usb/android0/idProduct");
	system("echo mass_storage > /sys/class/android_usb/android0/functions");
	system("echo 1 > /sys/class/android_usb/android0/enable");	
}


void RecordPreview::HdmiInitCamer()
{
	db_msg("by hero onthing HdmiInitCamer\n");
	zeta::ZetaDisplay *mZD = mZetaCamera->getDisp();
	mZD->close();
	stopPreview();
	//mZetaCamera->initCamera(1920, 1080, 640, 360, 60, 10, 1, 0);
	mZetaCamera->setPreviewRotation(0);
	startPreview();
}


void RecordPreview::HdmiPlugOut()
{
	db_msg("by hero onthing HdmiPlugOut\n");
	zeta::ZetaDisplay *mZD = mZetaCamera->getDisp();
	stopPreview();
	//mZetaCamera->initCamera(1920, 1080, 640, 360, 60, 10, 1, 90);
	mZetaCamera->setPreviewRotation(90);
	mZD->clean();
	startPreview();	
}
void RecordPreview::stopPreviewOutside(int flag)
{
	zeta::ZetaDisplay *mZD = mZetaCamera->getDisp();
	
	db_msg("stopPreviewOutside:--------flag:%d.\n", flag);
	if(flag){
//		mZD->close();
		stopPreview();
//		mZD->clean();
	}else{
//		mZD->clean();
		mZetaCamera->initCamera(1920, 1080, 640, 360, 60, 10, 1, 90);
		startPreview();	
	}
	
//	restoreZOrder();

}

void RecordPreview::stopPreview()
{
	mZetaCamera->stopPreview();
	db_msg("by hero onthing stopPreview\n");
}

int RecordPreview::msgCallback(int msgType, int idx)
{
	db_msg("by hero nothing msgCallback end\n");
	return 0;
}
bool RecordPreview::isRecording()
{
	db_msg("by hero nothing isRecording\n");
	return isRecorder;
}
int RecordPreview::releaseResource()
{
	return 0;
}
