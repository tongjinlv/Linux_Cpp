#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "windows.h"
#include "keyEvent.h"
#include "cdr_message.h"
#include "StorageManager.h"
#include "Dialog.h"
#undef LOG_TAG
#define LOG_TAG "main_preview.cpp"
#include "debug.h"

#define MAIN_NUM 10
#define MAINPREVIEW 401
using namespace zeta;
enum {
	MAIN_BJ = 0,
	RECORDER_N ,
	HDMI_N ,
	WIFI_N,
	ETHERNET_N,
	WEBCAM_N ,
	USBMODE_N,
	RS485_N ,	
	PLAYBACK_N,	
	TESTMODE_N 
};

int MainPreview::keyProc(int keyCode, int isLongPress)
{
	switch(keyCode) {
	case CDR_KEY_OK:
		db_msg("by hero *** ok\n");
		break;
	case CDR_KEY_POWER:	
	case CDR_KEY_MODE:
		db_msg("by hero *** mode\n");
		break;
	case CDR_KEY_LEFT:
		db_msg("by hero *** left\n");
		break;
	case CDR_KEY_RIGHT:
		db_msg("by hero *** right\n");
		break;
	default:
		break;
	}
	return WINDOWID_MAINPREVIEW;
}

int MainPreview::HandleTouchMsg(int keyCode)
{
	//if(keyCode == WINDOWID_RECORDPREVIEW){
		//mCdrMain->initPreview(NULL);
	//}
	SendMessage(mCdrMain->getHwnd(),MWM_CHANGE_WINDOW,WINDOWID_MAINPREVIEW,keyCode);
	
	if(mp_select == WIFI_N)
		SendMessage(mCdrMain->getWindowHandle(WINDOWID_OTHERPREVIEW), MSG_OW_CHANGE_FUNC, WIFI_FUNC, 0);
	else if(mp_select == HDMI_N)
		SendMessage(mCdrMain->getWindowHandle(WINDOWID_OTHERPREVIEW), MSG_OW_CHANGE_FUNC, HDMI_FUNC, 0);
	else if(mp_select == ETHERNET_N)
		SendMessage(mCdrMain->getWindowHandle(WINDOWID_OTHERPREVIEW), MSG_OW_CHANGE_FUNC, ETHERNET_FUNC, 0);
	else if(mp_select == WEBCAM_N)
		SendMessage(mCdrMain->getWindowHandle(WINDOWID_OTHERPREVIEW), MSG_OW_CHANGE_FUNC, WEBCAM_FUNC, 0);
	else if(mp_select == USBMODE_N)
		SendMessage(mCdrMain->getWindowHandle(WINDOWID_OTHERPREVIEW), MSG_OW_CHANGE_FUNC, USB_FUNC, 0);
	else if(mp_select == RS485_N)
		SendMessage(mCdrMain->getWindowHandle(WINDOWID_OTHERPREVIEW), MSG_OW_CHANGE_FUNC, RS485_FUNC, 0);	

	return 0;
}

int MainPreview::ShowMainPreview()
{
	BITMAP main_image_;
	BITMAP select_image_;
	if(!mCdrMain->get_InitFlag()){
		HDC hdc=BeginPaint(mMainHwnd);
		FillBox(hdc, mp_rect[MAIN_BJ].x, mp_rect[MAIN_BJ].y, mp_rect[MAIN_BJ].w, mp_rect[MAIN_BJ].h);
		LoadBitmapFromFile(HDC_SCREEN,&main_image_,"/etc/res/main/boot_logo.png");
		FillBoxWithBitmap(hdc, mp_rect[MAIN_BJ].x, mp_rect[MAIN_BJ].y, mp_rect[MAIN_BJ].w, mp_rect[MAIN_BJ].h,&main_image_);
		EndPaint(mMainHwnd,hdc);
		
		if(main_image_.bmBits != NULL){
			UnloadBitmap(&main_image_);
		}	
		return 0;
	}
		
	HDC hdc=BeginPaint(mMainHwnd);
	//HDC hdc=GetClientDC(mMainHwnd);
	FillBox(hdc, mp_rect[MAIN_BJ].x, mp_rect[MAIN_BJ].y, mp_rect[MAIN_BJ].w, mp_rect[MAIN_BJ].h);
	LoadBitmapFromFile(HDC_SCREEN,&main_image_,image_path_[MAIN_BJ]);
	FillBoxWithBitmap(hdc, mp_rect[MAIN_BJ].x, mp_rect[MAIN_BJ].y, mp_rect[MAIN_BJ].w, mp_rect[MAIN_BJ].h,&main_image_);
	if(mp_select != -1 && mp_select != 0){
		LoadBitmapFromFile(HDC_SCREEN,&select_image_,image_path_[mp_select]);
		FillBoxWithBitmap(hdc, mp_rect[mp_select].x, mp_rect[mp_select].y, mp_rect[mp_select].w, mp_rect[mp_select].h,&select_image_);	
	}
	
	EndPaint(mMainHwnd,hdc); 
	
	if(main_image_.bmBits != NULL){
		UnloadBitmap(&main_image_);
	}
	if(mp_select != -1 && mp_select != 0){
		if(select_image_.bmBits != NULL){
			UnloadBitmap(&select_image_);
		}	
	}
	return 0;
}


int MainPreviewProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	int ret = 0;
	//db_msg("message code: [%d], %s, wParam=%d, lParam=%lu\n", message, Message2Str(message), wParam, lParam);
    MainPreview* mpMP = (MainPreview*)GetWindowAdditionalData(hWnd);
	ResourceManager* mRM = ResourceManager::getInstance();
	switch(message) {
	case MSG_CREATE:	
		mpMP->createSubWidgets(hWnd);	
		break;
	case MSG_PAINT: 
		mpMP->ShowMainPreview();
		break;		
	case MSG_LBUTTONDOWN:
		{
			int pre_x = LOWORD (lParam);			
			int pre_y = HIWORD (lParam);
			if (mpMP->mp_select != -1)
				break;
			db_msg("by hero MSG_LBUTTONDOWN x = %d, y = %d\n", pre_x, pre_y);
			for(int i = 1; i <= MAIN_NUM; i++)
			{
				if(pre_x > mpMP->mp_rect[i].x && pre_x < (mpMP->mp_rect[i].x + mpMP->mp_rect[i].w) && pre_y > mpMP->mp_rect[i].y && (pre_y < mpMP->mp_rect[i].y + mpMP->mp_rect[i].h)){
					mpMP->mp_select = i;
					//UpdateWindow(hWnd, false);
					//mpMP->ShowMainPreview();
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
			UpdateWindow(hWnd, false);
			for(int i = 1; i <= MAIN_NUM; i++)
			{
				if(pre_x > mpMP->mp_rect[i].x && pre_x < (mpMP->mp_rect[i].x + mpMP->mp_rect[i].w) && pre_y > mpMP->mp_rect[i].y && (pre_y < mpMP->mp_rect[i].y + mpMP->mp_rect[i].h)){
					if(i == mpMP->mp_select){
						if(i == RECORDER_N)
							mpMP->HandleTouchMsg(WINDOWID_RECORDPREVIEW);
						if(i >= HDMI_N && i <= RS485_N)
							mpMP->HandleTouchMsg(WINDOWID_OTHERPREVIEW);
						if(i == TESTMODE_N)
							mpMP->HandleTouchMsg(WINDOWID_MODEPREVIEW);
						if(i == PLAYBACK_N)
							mpMP->HandleTouchMsg(WINDOWID_PLAYBACKPREVIEW);
						mpMP->mp_select = -1;
					}
					break;
				}
			}
			mpMP->mp_select = -1;
			//mpMP->ShowMainPreview();
			//UpdateWindow(hWnd, true);			
		}
		break;;
	case MSG_FONTCHANGED:		
		break;	
	case MSG_SHOWWINDOW:
		break;		
	case MSG_MP_CHANGE_WINDOW:
		printf("by hero MSG_MP_CHANGE_WINDOW\n");
		UpdateWindow(hWnd, true);
		break;
	case MSG_CDR_KEY:
		return mpMP->keyProc(wParam, lParam);			
	case MSG_DESTROY:
		break;	
	default:
		break;
	}
	return DefaultWindowProc(hWnd, message, wParam, lParam);
}

MainPreview::MainPreview(CdrMain* pCdrMain)
	: mCdrMain(pCdrMain)
	, mMainHwnd(NULL)
	, mp_select(-1)
{
	HWND hParent;
	HWND Time_hwnd;
	RECT rect;
	CDR_RECT rect_time;
	CDR_RECT STBRect;
	ResourceManager* mRM;
	hParent = mCdrMain->getHwnd();
	mRM = ResourceManager::getInstance();
	GetWindowRect(hParent, &rect);
	mHwnd = CreateWindowEx(WINDOW_MAINPREVIEW, "",
			WS_NONE,
			WS_EX_USEPARENTFONT,
			WINDOWID_MAINPREVIEW,
			rect.left, rect.top, RECTW(rect), RECTH(rect),
			hParent, (DWORD)this);

    db_msg(" mHwnd :%d ", mHwnd);
    
	if(mHwnd == HWND_INVALID) {
		db_error("create Main Preview window failed\n");
		return;
	}

    SetWindowBkColor(mHwnd, RGBA2Pixel(HDC_SCREEN,  0x00, 0x00, 0x00, 0xFF)); 
	mRM->setHwnd(WINDOWID_MAINPREVIEW, mHwnd);
	mMainHwnd = mHwnd;
	ShowWindow(mHwnd, SW_HIDE);
	
}

MainPreview::~MainPreview()
{
	db_msg(" clear ~MainPreview");
}

int MainPreview::releaseResource()
{
	return 0;
}

int MainPreview::createSubWidgets(HWND hParent)
{
	ResourceManager* mRM;
	CDR_RECT rect;
	int image_count = 0;
	mRM = ResourceManager::getInstance();
	image_path_[image_count] = "/etc/res/main/main.png";
	mRM->getResRect(ID_MP_BJ_WINDOW, mp_rect[image_count++]);
	image_path_[image_count] = "/etc/res/main/recorder_s.png";
	mRM->getResRect(ID_MP_RECORDER_WINDOW, mp_rect[image_count++]);
	image_path_[image_count] = "/etc/res/main/hdmi_s.png";
	mRM->getResRect(ID_MP_HDMI_WINDOW, mp_rect[image_count++]);
	image_path_[image_count] = "/etc/res/main/wifi_s.png";
	mRM->getResRect(ID_MP_WIFI_WINDOW, mp_rect[image_count++]);
	image_path_[image_count] = "/etc/res/main/ethernet_s.png";
	mRM->getResRect(ID_MP_ETHERNET_WINDOW, mp_rect[image_count++]);	
	image_path_[image_count] = "/etc/res/main/webcam_s.png";
	mRM->getResRect(ID_MP_WEBCAM_WINDOW, mp_rect[image_count++]);	
	image_path_[image_count] = "/etc/res/main/usbmode_s.png";
	mRM->getResRect(ID_MP_USBMODE_WINDOW, mp_rect[image_count++]);	
	image_path_[image_count] = "/etc/res/main/rs485_s.png";
	mRM->getResRect(ID_MP_RS485_WINDOW, mp_rect[image_count++]);	
	image_path_[image_count] = "/etc/res/main/playback_s.png";
	mRM->getResRect(ID_MP_PLAYBACK_WINDOW, mp_rect[image_count++]);	
	image_path_[image_count] = "/etc/res/main/testmode_s.png";
	mRM->getResRect(ID_MP_TESTMODE_WINDOW, mp_rect[image_count++]);	

	HWND Hwnd;

	Hwnd = CreateWindowEx(CTRL_STATIC, "",
			WS_CHILD | WS_VISIBLE,
			WS_EX_TRANSPARENT | WS_EX_USEPARENTFONT,
			ID_MP_MAINHWMD_WINDOW,
			mp_rect[MAIN_BJ].x, mp_rect[MAIN_BJ].y, mp_rect[MAIN_BJ].w, mp_rect[MAIN_BJ].h,
			hParent, 0);
	
	if(Hwnd == HWND_INVALID) {
		db_error("create playback previe image label failed\n");
		return -1;
	}	
	return 0;
}



