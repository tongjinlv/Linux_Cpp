#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<sys/ioctl.h>
#include "net/if.h"
#include "arpa/inet.h"
#include "linux/sockios.h"


#include "windows.h"
#include "keyEvent.h"
#include "cdr_message.h"
#include "StorageManager.h"
#include "Dialog.h"
#undef LOG_TAG
#define LOG_TAG "other_preview.cpp"
#include "debug.h"

using namespace zeta;

#define MAIN_BJ 0
#define MAIN_BACK 1
#define FUNC_BJ 2
#define CHECK_IP_ADDR_TIMER 100
int OtherPreview::keyProc(int keyCode, int isLongPress)
{
	switch(keyCode) {
	case CDR_KEY_OK:
		break;
	case CDR_KEY_POWER:	
	case CDR_KEY_MODE:
		break;
	case CDR_KEY_LEFT:
		break;
	case CDR_KEY_RIGHT:
		break;
	default:
		break;
	}
	return WINDOWID_OTHERPREVIEW;
}

void OtherPreview::wifiEnable()
{
	if(!wifi_open){
		mCdrMain->wifiInit();
		mCdrMain->wifiEnable(true);
		wifi_ssid = mCdrMain->wifi_ssid;
		wifi_passwd = mCdrMain->wifi_passwd;
		wifi_open = true;
	}
}

void OtherPreview::wifiDisable()
{
	if(wifi_open) {
		mCdrMain->wifiEnable(false);
		wifi_open = false;
	}
}

void OtherPreview::usbDisable()
{
	mCdrMain->stopConnectPC();
}

void OtherPreview::usbEnable()
{
	mCdrMain->startConnectPC();
}

void OtherPreview::WebcamDisable()
{
	mCdrMain->stopPCCam();
}

void OtherPreview::WebcamEnable()
{
	mCdrMain->startPCCam();
}

void OtherPreview::HdmiDisable()
{
	mCdrMain->stopHdmi();
}

void OtherPreview::HdmiEnable()
{
	mCdrMain->startHdmi();
}

char* OtherPreview::Get_IpAddr()
{
	struct ifreq ifreq; 
	char *name = "eth0";
	char *ip_addr_err = "0.0.0.0";
	int sock; 
	strcpy(ifreq.ifr_name, name);
	if((sock = socket(AF_INET,SOCK_STREAM,0)) <0) 
	{ 
		db_msg("socket err\n"); 
		return ip_addr_err;
	} 
 
	if(ioctl(sock,SIOCGIFADDR,&ifreq) < 0) 
	{ 
	    db_msg("ioctl err\n"); 
		return ip_addr_err;
	} 

    //get inet addr
    struct sockaddr_in *addr = (struct sockaddr_in *)&(ifreq.ifr_addr);
    char *address = inet_ntoa(addr->sin_addr);
	return address;
	
}

int OtherPreview::HandleTouchMsg(int keyCode)
{
	if(mCdrMain->PlugInHdmiPlayFlag)
		return 0;
	switch(func_select){
		case HDMI_FUNC:
			HdmiDisable();
			break;
		case USB_FUNC:
			usbDisable();
			break;
		case WEBCAM_FUNC:
			WebcamDisable();
			break;
		case RS485_FUNC:
			break;
		case ETHERNET_FUNC:
			KillTimer(mMainHwnd, CHECK_IP_ADDR_TIMER);
			break;
		case WIFI_FUNC:
			wifiDisable();
			break;
		default:
			break;
		func_select = -1;	
	}
	
	SendMessage(mCdrMain->getHwnd(),MWM_CHANGE_WINDOW,WINDOWID_OTHERPREVIEW,keyCode);
	return 0;
}

int OtherPreview::ShowWifiPreview()
{
	BITMAP main_image_;
	BITMAP back_image_;
	ResourceManager* mRM = ResourceManager::getInstance();
	HDC hdc=BeginPaint(mMainHwnd);
	SetBrushColor (hdc, RGBA2Pixel (hdc, 0x00, 0x00, 0x00, 0x00));
	FillBox(hdc, wp_rect[MAIN_BJ].x, wp_rect[MAIN_BJ].y, wp_rect[MAIN_BJ].w, wp_rect[MAIN_BJ].h);

	//main
	LoadBitmapFromFile(HDC_SCREEN,&main_image_,image_path[MAIN_BJ]);
	FillBoxWithBitmap(hdc, wp_rect[MAIN_BJ].x, wp_rect[MAIN_BJ].y, wp_rect[MAIN_BJ].w, wp_rect[MAIN_BJ].h,&main_image_);

	//back
	if(wp_select == MAIN_BACK){
		LoadBitmapFromFile(HDC_SCREEN,&back_image_,image_path[MAIN_BACK+1]);
	}else{
		LoadBitmapFromFile(HDC_SCREEN,&back_image_,image_path[MAIN_BACK]);
	}
	FillBoxWithBitmap(hdc, wp_rect[MAIN_BACK].x, wp_rect[MAIN_BACK].y, wp_rect[MAIN_BACK].w, wp_rect[MAIN_BACK].h,&back_image_);
	
	//func
	wifi_ssid = mCdrMain->wifi_ssid;
	wifi_passwd = mCdrMain->wifi_passwd;

	SetBkMode(hdc, BM_TRANSPARENT);    
	SetTextColor(hdc, RGB2Pixel(hdc, 0x44, 0x80, 0xEE));	//RGB2Pixel (mem_dc, 0x00, 0x00, 0xFF));
	TabbedTextOut(hdc, 350, 150, "SSID: ");
	TabbedTextOut(hdc, 350, 280, "PASSWD: ");	
	SetTextColor(hdc, RGB2Pixel(hdc, 0xff, 0xff, 0xff));	//RGB2Pixel (mem_dc, 0x00, 0x00, 0xFF));
	TabbedTextOut(hdc, 350, 192, wifi_ssid);
	TabbedTextOut(hdc, 350, 320, wifi_passwd);
	
	EndPaint(mMainHwnd,hdc); 
	
	if(back_image_.bmBits != NULL)
		UnloadBitmap(&back_image_); 

	if(main_image_.bmBits != NULL)
		UnloadBitmap(&main_image_); 

	return 0;
}

int OtherPreview::ShowHdmiPreview()
{
	BITMAP main_image_;
	BITMAP back_image_;
	BITMAP func_image_;
	HDC hdc=BeginPaint(mMainHwnd);
	SetBrushColor (hdc, RGBA2Pixel (hdc, 0x00, 0x00, 0x00, 0x00));
	FillBox(hdc, wp_rect[MAIN_BJ].x, wp_rect[MAIN_BJ].y, wp_rect[MAIN_BJ].w, wp_rect[MAIN_BJ].h);

	//main
	LoadBitmapFromFile(HDC_SCREEN,&main_image_,image_path[MAIN_BJ]);
	FillBoxWithBitmap(hdc, wp_rect[MAIN_BJ].x, wp_rect[MAIN_BJ].y, wp_rect[MAIN_BJ].w, wp_rect[MAIN_BJ].h,&main_image_);

	//back
	if(wp_select == MAIN_BACK){
		LoadBitmapFromFile(HDC_SCREEN,&back_image_,image_path[MAIN_BACK+1]);
	}else{
		LoadBitmapFromFile(HDC_SCREEN,&back_image_,image_path[MAIN_BACK]);
	}
	FillBoxWithBitmap(hdc, wp_rect[MAIN_BACK].x, wp_rect[MAIN_BACK].y, wp_rect[MAIN_BACK].w, wp_rect[MAIN_BACK].h,&back_image_);

	//func
	if(mCdrMain->PlugInHdmiPlayFlag){
		LoadBitmapFromFile(HDC_SCREEN,&func_image_,func_path[func_select+6]);
		FillBoxWithBitmap(hdc, wp_rect[FUNC_BJ].x, wp_rect[FUNC_BJ].y, wp_rect[FUNC_BJ].w, wp_rect[FUNC_BJ].h,&func_image_);
	}else{
		LoadBitmapFromFile(HDC_SCREEN,&func_image_,func_path[func_select]);
		FillBoxWithBitmap(hdc, wp_rect[FUNC_BJ].x, wp_rect[FUNC_BJ].y, wp_rect[FUNC_BJ].w, wp_rect[FUNC_BJ].h,&func_image_);
	}
	
	EndPaint(mMainHwnd,hdc); 

	if(func_image_.bmBits != NULL)
		UnloadBitmap(&func_image_);


	if(back_image_.bmBits != NULL)
		UnloadBitmap(&back_image_); 

	if(main_image_.bmBits != NULL)
		UnloadBitmap(&main_image_); 

	return 0;
}

int OtherPreview::ShowUsbPreview()
{
	BITMAP main_image_;
	BITMAP back_image_;
	BITMAP func_image_;
	HDC hdc=BeginPaint(mMainHwnd);
	SetBrushColor (hdc, RGBA2Pixel (hdc, 0x00, 0x00, 0x00, 0x00));
	FillBox(hdc, wp_rect[MAIN_BJ].x, wp_rect[MAIN_BJ].y, wp_rect[MAIN_BJ].w, wp_rect[MAIN_BJ].h);

	//main
	LoadBitmapFromFile(HDC_SCREEN,&main_image_,image_path[MAIN_BJ]);
	FillBoxWithBitmap(hdc, wp_rect[MAIN_BJ].x, wp_rect[MAIN_BJ].y, wp_rect[MAIN_BJ].w, wp_rect[MAIN_BJ].h,&main_image_);

	//back
	if(wp_select == MAIN_BACK){
		LoadBitmapFromFile(HDC_SCREEN,&back_image_,image_path[MAIN_BACK+1]);
	}else{
		LoadBitmapFromFile(HDC_SCREEN,&back_image_,image_path[MAIN_BACK]);
	}
	FillBoxWithBitmap(hdc, wp_rect[MAIN_BACK].x, wp_rect[MAIN_BACK].y, wp_rect[MAIN_BACK].w, wp_rect[MAIN_BACK].h,&back_image_);

	//func
	if(mCdrMain->m2pcConnected){
		LoadBitmapFromFile(HDC_SCREEN,&func_image_,func_path[func_select+6]);
		FillBoxWithBitmap(hdc, wp_rect[FUNC_BJ].x, wp_rect[FUNC_BJ].y, wp_rect[FUNC_BJ].w, wp_rect[FUNC_BJ].h,&func_image_);
	}else{
		LoadBitmapFromFile(HDC_SCREEN,&func_image_,func_path[func_select]);
		FillBoxWithBitmap(hdc, wp_rect[FUNC_BJ].x, wp_rect[FUNC_BJ].y, wp_rect[FUNC_BJ].w, wp_rect[FUNC_BJ].h,&func_image_);
	}
	
	EndPaint(mMainHwnd,hdc); 

	if(func_image_.bmBits != NULL)
		UnloadBitmap(&func_image_);

	if(back_image_.bmBits != NULL)
		UnloadBitmap(&back_image_); 

	if(main_image_.bmBits != NULL)
		UnloadBitmap(&main_image_); 

	return 0;
}

int OtherPreview::ShowWebcamPreview()
{
	BITMAP main_image_;
	BITMAP back_image_;
	BITMAP func_image_;
	HDC hdc=BeginPaint(mMainHwnd);
	SetBrushColor (hdc, RGBA2Pixel (hdc, 0x00, 0x00, 0x00, 0x00));
	FillBox(hdc, wp_rect[MAIN_BJ].x, wp_rect[MAIN_BJ].y, wp_rect[MAIN_BJ].w, wp_rect[MAIN_BJ].h);

	//main
	LoadBitmapFromFile(HDC_SCREEN,&main_image_,image_path[MAIN_BJ]);
	FillBoxWithBitmap(hdc, wp_rect[MAIN_BJ].x, wp_rect[MAIN_BJ].y, wp_rect[MAIN_BJ].w, wp_rect[MAIN_BJ].h,&main_image_);

	//back
	if(wp_select == MAIN_BACK){
		LoadBitmapFromFile(HDC_SCREEN,&back_image_,image_path[MAIN_BACK+1]);
	}else{
		LoadBitmapFromFile(HDC_SCREEN,&back_image_,image_path[MAIN_BACK]);
	}
	FillBoxWithBitmap(hdc, wp_rect[MAIN_BACK].x, wp_rect[MAIN_BACK].y, wp_rect[MAIN_BACK].w, wp_rect[MAIN_BACK].h,&back_image_);

	//func
	if(mCdrMain->m2pcConnected){
		LoadBitmapFromFile(HDC_SCREEN,&func_image_,func_path[func_select+6]);
		FillBoxWithBitmap(hdc, wp_rect[FUNC_BJ].x, wp_rect[FUNC_BJ].y, wp_rect[FUNC_BJ].w, wp_rect[FUNC_BJ].h,&func_image_);
	}else{
		LoadBitmapFromFile(HDC_SCREEN,&func_image_,func_path[func_select]);
		FillBoxWithBitmap(hdc, wp_rect[FUNC_BJ].x, wp_rect[FUNC_BJ].y, wp_rect[FUNC_BJ].w, wp_rect[FUNC_BJ].h,&func_image_);
	}
	
	EndPaint(mMainHwnd,hdc); 

	if(func_image_.bmBits != NULL)
		UnloadBitmap(&func_image_);


	if(back_image_.bmBits != NULL)
		UnloadBitmap(&back_image_); 

	if(main_image_.bmBits != NULL)
		UnloadBitmap(&main_image_); 

	return 0;
}

int OtherPreview::ShowRS485Preview()
{
	BITMAP main_image_;
	BITMAP back_image_;
	BITMAP func0_image_;
	BITMAP func1_image_;
	HDC hdc=BeginPaint(mMainHwnd);
	SetBrushColor (hdc, RGBA2Pixel (hdc, 0x00, 0x00, 0x00, 0x00));
	FillBox(hdc, wp_rect[MAIN_BJ].x, wp_rect[MAIN_BJ].y, wp_rect[MAIN_BJ].w, wp_rect[MAIN_BJ].h);

	//main
	LoadBitmapFromFile(HDC_SCREEN,&main_image_,image_path[MAIN_BJ]);
	FillBoxWithBitmap(hdc, wp_rect[MAIN_BJ].x, wp_rect[MAIN_BJ].y, wp_rect[MAIN_BJ].w, wp_rect[MAIN_BJ].h,&main_image_);


	//func
	LoadBitmapFromFile(HDC_SCREEN,&func0_image_,"/etc/res/others/hub_off.png");
	FillBoxWithBitmap(hdc, 105, 124, 282, 282,&func0_image_);

	LoadBitmapFromFile(HDC_SCREEN,&func1_image_,"/etc/res/others/hub_off.png");
	FillBoxWithBitmap(hdc, 467, 124, 282, 282,&func1_image_);
	
	//back
	if(wp_select == MAIN_BACK){
		LoadBitmapFromFile(HDC_SCREEN,&back_image_,image_path[MAIN_BACK+1]);
	}else{
		LoadBitmapFromFile(HDC_SCREEN,&back_image_,image_path[MAIN_BACK]);
	}
	FillBoxWithBitmap(hdc, wp_rect[MAIN_BACK].x, wp_rect[MAIN_BACK].y, wp_rect[MAIN_BACK].w, wp_rect[MAIN_BACK].h,&back_image_);

	EndPaint(mMainHwnd,hdc); 

	if(func0_image_.bmBits != NULL)
		UnloadBitmap(&func0_image_);

	if(func1_image_.bmBits != NULL)
		UnloadBitmap(&func1_image_);


	if(back_image_.bmBits != NULL)
		UnloadBitmap(&back_image_); 

	if(main_image_.bmBits != NULL)
		UnloadBitmap(&main_image_); 

	return 0;
}

int OtherPreview::ShowEthernetPreview()
{
	BITMAP main_image_;
	BITMAP back_image_;
	BITMAP func_image_;
	HDC hdc=BeginPaint(mMainHwnd);
	SetBrushColor (hdc, RGBA2Pixel (hdc, 0x00, 0x00, 0x00, 0x00));
	FillBox(hdc, wp_rect[MAIN_BJ].x, wp_rect[MAIN_BJ].y, wp_rect[MAIN_BJ].w, wp_rect[MAIN_BJ].h);
	//main
	LoadBitmapFromFile(HDC_SCREEN,&main_image_,image_path[MAIN_BJ]);
	FillBoxWithBitmap(hdc, wp_rect[MAIN_BJ].x, wp_rect[MAIN_BJ].y, wp_rect[MAIN_BJ].w, wp_rect[MAIN_BJ].h,&main_image_);

	//back
	if(wp_select == MAIN_BACK){
		LoadBitmapFromFile(HDC_SCREEN,&back_image_,image_path[MAIN_BACK+1]);
	}else{
		LoadBitmapFromFile(HDC_SCREEN,&back_image_,image_path[MAIN_BACK]);
	}
	FillBoxWithBitmap(hdc, wp_rect[MAIN_BACK].x, wp_rect[MAIN_BACK].y, wp_rect[MAIN_BACK].w, wp_rect[MAIN_BACK].h,&back_image_);

	//func
	if(strcmp(ip_addr, "0.0.0.0")){
		SetBkMode(hdc, BM_TRANSPARENT);    
		SetTextColor(hdc, RGB2Pixel(hdc, 0x44, 0x80, 0xEE));	//RGB2Pixel (mem_dc, 0x00, 0x00, 0xFF));
		TabbedTextOut(hdc, 350, 188, "IP ADDRESS: ");
		SetTextColor(hdc, RGB2Pixel(hdc, 0xff, 0xff, 0xff));	//RGB2Pixel (mem_dc, 0x00, 0x00, 0xFF));
		TabbedTextOut(hdc, 350, 250, (const char *)ip_addr);		
	}else{
		LoadBitmapFromFile(HDC_SCREEN,&func_image_,func_path[func_select]);
		FillBoxWithBitmap(hdc, wp_rect[FUNC_BJ].x, wp_rect[FUNC_BJ].y, wp_rect[FUNC_BJ].w, wp_rect[FUNC_BJ].h,&func_image_);
	}
	
	EndPaint(mMainHwnd,hdc); 
	
	if(func_image_.bmBits != NULL && !strcmp(ip_addr, "0.0.0.0"))
		UnloadBitmap(&func_image_);

	if(back_image_.bmBits != NULL)
		UnloadBitmap(&back_image_); 

	if(main_image_.bmBits != NULL)
		UnloadBitmap(&main_image_); 
	return 0;
}

int OtherPreview::ShowOtherPreview(int type)
{
	switch(type){
		case WIFI_FUNC:
			ShowWifiPreview();
			break;
		case HDMI_FUNC:
			ShowHdmiPreview();
			break;
		case USB_FUNC:
			ShowUsbPreview();
			break;
		case WEBCAM_FUNC:
			ShowWebcamPreview();
			break;
		case RS485_FUNC:
			ShowRS485Preview();
			break;
		case ETHERNET_FUNC:
			ShowEthernetPreview();
			break;		
		default:
			break;
	}
	
	return 0;
}


int OtherPreviewProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	int ret = 0;
    OtherPreview* mWP = (OtherPreview*)GetWindowAdditionalData(hWnd);
	ResourceManager* mRM = ResourceManager::getInstance();
	switch(message) {
	case MSG_CREATE:	
		mWP->createSubWidgets(hWnd);
		break;
	case MSG_PAINT: 	
		mWP->ShowOtherPreview(mWP->func_select);
		break;
	case MSG_LBUTTONDOWN:
		{
			int pre_x = LOWORD (lParam);			
			int pre_y = HIWORD (lParam);			
			for(int i = 1; i <= MAIN_BACK; i++)
			{
				//if(pre_x > mWP->wp_rect[i].x && pre_x < (mWP->wp_rect[i].x + mWP->wp_rect[i].w) && pre_y > mWP->wp_rect[i].y && (pre_y < mWP->wp_rect[i].y + mWP->wp_rect[i].h)){
				if(pre_x > mWP->rback_bt.x && pre_x < mWP->rback_bt.w && pre_y >mWP->rback_bt.y && pre_y < mWP->rback_bt.h){
					mWP->wp_select = i;
					//UpdateWindow(hWnd, false);
					break;
				}
			}
		}
		break;
	case MSG_LBUTTONUP:		
		{
			int pre_x = LOWORD (lParam);			
			int pre_y = HIWORD (lParam);	
			
			for(int i = 1; i <= MAIN_BACK; i++)
			{
				//if(pre_x > mWP->wp_rect[i].x && pre_x < (mWP->wp_rect[i].x + mWP->wp_rect[i].w) && pre_y > mWP->wp_rect[i].y && (pre_y < mWP->wp_rect[i].y + mWP->wp_rect[i].h)){
				if(pre_x > mWP->rback_bt.x && pre_x < mWP->rback_bt.w && pre_y > mWP->rback_bt.y && pre_y < mWP->rback_bt.h){
					if(i == mWP->wp_select){
						if(i == MAIN_BACK){
							mWP->HandleTouchMsg(WINDOWID_MAINPREVIEW);
						}
					}
					break;
				}
			}
			mWP->wp_select = -1;
			//UpdateWindow(hWnd, false);	
		}
		break;	
	case MSG_OW_CHANGE_FUNC:
		{
			mWP->wp_select = -1;
			mWP->func_select = (int)wParam;
			if(wParam == WIFI_FUNC)
				mWP->wifiEnable();
			else if(wParam == USB_FUNC)
				mWP->usbEnable();
			else if(wParam == WEBCAM_FUNC)
				mWP->WebcamEnable();
			else if(wParam == HDMI_FUNC)
				mWP->HdmiEnable();
			else if(wParam == ETHERNET_FUNC){
				mWP->ip_addr = mWP->Get_IpAddr();
				SetTimer(hWnd, CHECK_IP_ADDR_TIMER, 100);
			}
			//ShowWindow(mWP->mFuncHwnd,true);
		}
		break;
		case MSG_TIMER:
		{
			if(wParam == CHECK_IP_ADDR_TIMER){
				if(!strcmp(mWP->ip_addr, "0.0.0.0")){
					mWP->ip_addr = mWP->Get_IpAddr();
					UpdateWindow(hWnd, false);
				}
			}
		}
		break;
	case MSG_FONTCHANGED:		
		break;	
	case MSG_SHOWWINDOW:
		break;		
	case MSG_CDR_KEY:
		return mWP->keyProc(wParam, lParam);			
	case MSG_DESTROY:
		db_msg("MSG_DESTROY\n");
		break;	
	default:
		break;
	}
	return DefaultWindowProc(hWnd, message, wParam, lParam);
}

OtherPreview::OtherPreview(CdrMain* pCdrMain)
	: mCdrMain(pCdrMain)
	, mMainHwnd(NULL)
	, mFuncHwnd(NULL)
	, wp_select(-1)
	, func_select(-1)
	, wifi_open(false)

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
	mHwnd = CreateWindowEx(WINDOW_OTHERPREVIEW, "",
			WS_NONE,
			WS_EX_USEPARENTFONT,
			WINDOWID_OTHERPREVIEW,
			rect.left, rect.top, RECTW(rect), RECTH(rect),
			hParent, (DWORD)this);

    db_msg(" mHwnd :%d ", mHwnd);
    
	if(mHwnd == HWND_INVALID) {
		db_error("create Main Preview window failed\n");
		return;
	}
    SetWindowBkColor(mHwnd, RGBA2Pixel(HDC_SCREEN,  0x00, 0x00, 0x00, 0xFF)); 
	mRM->setHwnd(WINDOWID_OTHERPREVIEW, mHwnd);
	ShowWindow(mHwnd, SW_HIDE);
	mMainHwnd = mHwnd;
}

OtherPreview::~OtherPreview()
{
	db_msg(" clear ~OtherPreview");
}

int OtherPreview::releaseResource()
{
	return 0;
}

int OtherPreview::createSubWidgets(HWND hParent)
{
	HWND hWnd;
	ResourceManager* mRM;
	CDR_RECT rect;
	int image_count = 0;
	int func_count = 0;
	mRM = ResourceManager::getInstance();

	mRM->getResRect(ID_MP_BJ_WINDOW, wp_rect[image_count]);
	image_path[image_count++] = "/etc/res/others/other_main.png";
	mRM->getResRect(ID_STATUSBAR_ICON_BACK, wp_rect[image_count]);
	image_path[image_count++] = "/etc/res/topbar/back_normal.png";
	image_path[image_count] = "/etc/res/topbar/back_press.png";
	mRM->getResRect(ID_OP_FUNC_WINDOW, wp_rect[image_count]);
	
	func_path[func_count++] = "/etc/res/others/other_main.png";
	func_path[func_count++] = "/etc/res/others/webcamoff.png";
	func_path[func_count++] = "/etc/res/others/usb_off.png";
	func_path[func_count++] = "/etc/res/others/hdmioff.png";
	func_path[func_count++] = "/etc/res/others/ethernet_off.png";
	func_path[func_count++] = "/etc/res/others/rs485.png";
	func_path[func_count++] = "/etc/res/others/other_main.png";
	func_path[func_count++] = "/etc/res/others/webcamon.png";
	func_path[func_count++] = "/etc/res/others/usb_on.png";
	func_path[func_count++] = "/etc/res/others/hdmion.png";
	func_path[func_count++] = "/etc/res/others/ethernet_off.png";
	func_path[func_count++] = "/etc/res/others/rs485.png";

	rback_bt.x = 0;
	rback_bt.y = 0;
	rback_bt.w = wp_rect[MAIN_BACK].w+40;
	rback_bt.h = wp_rect[MAIN_BACK].h+40;

	mFuncHwnd = CreateWindowEx(CTRL_STATIC, "",
			WS_CHILD | WS_VISIBLE,
			WS_EX_TRANSPARENT | WS_EX_USEPARENTFONT,
			ID_OP_FUNCTION_WINDOW,
			//250, 140, 354, 200,
			wp_rect[MAIN_BJ].x, wp_rect[MAIN_BJ].y, wp_rect[MAIN_BJ].w, wp_rect[MAIN_BJ].h,
			hParent, 0);
	
	if(mFuncHwnd == HWND_INVALID) {
		db_error("create playback previe image label failed\n");
		return -1;
	}

	SetWindowBkColor(mFuncHwnd, RGBA2Pixel(HDC_SCREEN,  0x00, 0x00, 0x00, 0xFF)); 
	return 0;
}



