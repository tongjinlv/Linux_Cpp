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
#define LOG_TAG "PlayBackPreview.cpp"
#include "debug.h"

#define MAIN_NUM 2 
#define MP4_PATH "/mnt/extsd/out.mp4"
enum {
	MAIN_BJ = 0,
	MAIN_BACK,
	BUTTON_1,
	BUTTON_2,
	BUTTON_3,
	BUTTON_4,
	BUTTON_5,
	BUTTON_6,
	BUTTON_7,
	BUTTON_8,
	BUTTON_P,
	BUTTON_N	
};


using namespace zeta;
int PlayBackPreview::keyProc(int keyCode, int isLongPress)
{
	switch(keyCode) {
	case CDR_KEY_OK:
		break;
	case CDR_KEY_POWER:
		return WINDOWID_MAINPREVIEW;
	case CDR_KEY_MENU:
		break;
	case CDR_KEY_LEFT:
		break;
	case CDR_KEY_RIGHT:
		break;
	default:
		break;
	}
	return WINDOWID_PLAYBACKPREVIEW;
}

int PlayBackPreview::HandleTouchMsg(int keyCode)
{
	if(keyCode == WINDOWID_PLAYBACK){
		if(access(MP4_PATH, F_OK) != 0) 
			return 0;
	}
	
	SendMessage(mCdrMain->getHwnd(),MWM_CHANGE_WINDOW,WINDOWID_PLAYBACKPREVIEW,keyCode);
	SendMessage(mCdrMain->getHwnd(),MWM_CHANGE_WINDOW,WINDOWID_PLAYBACKPREVIEW,keyCode);
	return 0;
}

int PlayBackPreview::ShowMainPreview()
{
	db_msg("by hero *** ShowMainPreview start\n");
	BITMAP main_image_;
	BITMAP back_image_;
	BITMAP bt0_image_;
	BITMAP func1_image_;
	HDC hdc=BeginPaint(mMainHwnd);
	SetBrushColor (hdc, RGBA2Pixel (hdc, 0x00, 0x00, 0x00, 0x00));
	FillBox(hdc, wp_rect[MAIN_BJ].x, wp_rect[MAIN_BJ].y, wp_rect[MAIN_BJ].w, wp_rect[MAIN_BJ].h);

	//main
	LoadBitmapFromFile(HDC_SCREEN,&main_image_,image_path[MAIN_BJ]);
	FillBoxWithBitmap(hdc, wp_rect[MAIN_BJ].x, wp_rect[MAIN_BJ].y, wp_rect[MAIN_BJ].w, wp_rect[MAIN_BJ].h,&main_image_);

	//bt test
	if(access(MP4_PATH, F_OK) == 0) {
		LoadBitmapFromFile(HDC_SCREEN,&bt0_image_,"/etc/res/others/pb_video.png");
		FillBoxWithBitmap(hdc, wp_rect[BUTTON_1].x, wp_rect[BUTTON_1].y, wp_rect[BUTTON_1].w, wp_rect[BUTTON_1].h, &bt0_image_);
		SetBkMode(hdc, BM_TRANSPARENT);  
		SetTextColor(hdc, RGB2Pixel(hdc, 0xFF, 0xFF, 0xFF));	
		TabbedTextOut(hdc, wp_rect[BUTTON_1].x, wp_rect[BUTTON_1].y + wp_rect[BUTTON_1].h, "out.mp4");
	}
	
	//back
	if(wp_select == MAIN_BACK){
		LoadBitmapFromFile(HDC_SCREEN,&back_image_,image_path[MAIN_BACK+1]);
	}else{
		LoadBitmapFromFile(HDC_SCREEN,&back_image_,image_path[MAIN_BACK]);
	}
	
	FillBoxWithBitmap(hdc, wp_rect[MAIN_BACK].x, wp_rect[MAIN_BACK].y, wp_rect[MAIN_BACK].w, wp_rect[MAIN_BACK].h,&back_image_);

	EndPaint(mMainHwnd,hdc); 
	
	if(access(MP4_PATH, F_OK) == 0) {
		if(bt0_image_.bmBits != NULL)
			UnloadBitmap(&bt0_image_);
	}
	if(back_image_.bmBits != NULL)
		UnloadBitmap(&back_image_); 

	if(main_image_.bmBits != NULL)
		UnloadBitmap(&main_image_); 
	db_msg("by hero *** ShowMainPreview end\n");

	return 0;

}


int PlayBackPreviewProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	PlayBackPreview* mPLBPreview = (PlayBackPreview*)GetWindowAdditionalData(hWnd);
	int ret = 0;
	//db_msg("message code: [%d], %s, wParam=%d, lParam=%lu\n", message, Message2Str(message), wParam, lParam);
	switch(message) {
		case MSG_CREATE:
			mPLBPreview->createSubWidgets(hWnd);
			break;
		case MSG_PAINT: 
			db_msg("by hero **** PlayBackPreviewProc MSG_PAINT\n");
			mPLBPreview->ShowMainPreview();
			return 0;		
		case MSG_CDR_KEY:
			return mPLBPreview->keyProc(wParam, lParam);
		case MSG_LBUTTONDOWN:
		{
			int pre_x = LOWORD (lParam);			
			int pre_y = HIWORD (lParam);			
			for(int i = 1; i <= MAIN_NUM; i++)
			{
				if(pre_x > mPLBPreview->wp_rect[i].x && pre_x < (mPLBPreview->wp_rect[i].x + mPLBPreview->wp_rect[i].w) && pre_y > mPLBPreview->wp_rect[i].y && (pre_y < mPLBPreview->wp_rect[i].y + mPLBPreview->wp_rect[i].h)){
					mPLBPreview->wp_select = i;
					//UpdateWindow(hWnd, true);
					break;
				}
			}
		}
		break;
		case MSG_LBUTTONUP:		
		{
			int pre_x = LOWORD (lParam);			
			int pre_y = HIWORD (lParam);	
			
			for(int i = 1; i <= MAIN_NUM; i++)
			{
				if(pre_x > mPLBPreview->wp_rect[i].x && pre_x < (mPLBPreview->wp_rect[i].x + mPLBPreview->wp_rect[i].w) && pre_y > mPLBPreview->wp_rect[i].y && (pre_y < mPLBPreview->wp_rect[i].y + mPLBPreview->wp_rect[i].h)){
					if(i == mPLBPreview->wp_select){
						if(i == MAIN_BACK){
							mPLBPreview->HandleTouchMsg(WINDOWID_MAINPREVIEW);
						}
						if(i == BUTTON_1)
						{
							mPLBPreview->HandleTouchMsg(WINDOWID_PLAYBACK);
						}
					}
					break;
				}
			}
			mPLBPreview->wp_select = -1;
			//UpdateWindow(hWnd, true);	
		}
		break;			
		case MWM_CHANGE_FROM_WINDOW:
			break;
		case MSG_STOP_PREVIEW:
			break;
		case MSG_UPDATE_LIST:
			break;
		case MSG_SET_CURRENT_FILEID:
			break;
		default:
			break;
	}
	return DefaultWindowProc(hWnd, message, wParam, lParam);
}

PlayBackPreview::PlayBackPreview(CdrMain* pCdrMain)
	: mCdrMain(pCdrMain)
	, bmpIcon()
{
	HWND hParent;
	RECT rect;
	CDR_RECT STBRect;
	ResourceManager* mRM;
	hParent = mCdrMain->getHwnd();
	
	mRM = ResourceManager::getInstance();
	GetWindowRect(hParent, &rect);
	mHwnd = CreateWindowEx(WINDOW_PLAYBACKPREVIEW, "",
			WS_NONE,
			WS_EX_TRANSPARENT | WS_EX_USEPARENTFONT,
			WINDOWID_PLAYBACKPREVIEW,
			rect.left, rect.top, RECTW(rect), RECTH(rect),
			hParent, (DWORD)this);
	if(mHwnd == HWND_INVALID) {
		db_error("create PlayBack Preview window failed\n");
		return;
	}
	
	mRM->setHwnd(WINDOWID_PLAYBACKPREVIEW, mHwnd);
	mCurID = 0;
	ShowWindow(mHwnd, SW_HIDE);
	mMainHwnd = mHwnd;
	db_msg("by hero PlayBackPreview end\n");
}
PlayBackPreview::~PlayBackPreview()
{
}

int PlayBackPreview::createSubWidgets(HWND hParent)
{
	ResourceManager* mRM;
	CDR_RECT rect;
	int image_count = 0;
	int button_count = 0;
	mRM = ResourceManager::getInstance();

	mRM->getResRect(ID_MP_BJ_WINDOW, wp_rect[image_count]);
	image_path[image_count++] = "/etc/res/others/other_main.png";
	mRM->getResRect(ID_STATUSBAR_ICON_BACK, wp_rect[image_count]);
	image_path[image_count++] = "/etc/res/topbar/back_normal.png";
	wp_rect[image_count].x = 121;
	wp_rect[image_count].y = 98;
	wp_rect[image_count].w = 138;
	wp_rect[image_count].h = 138;
	image_path[image_count] = "/etc/res/topbar/back_press.png";
	return 0;
}

int PlayBackPreview::getBmpPicFromVideo(const char* videoFile, const char* picFile)
{
	db_msg("by hero nothing getBmpPicFromVideo\n");
	return 0;
}
int PlayBackPreview::releaseResource()
{
	db_msg("by hero nothing releaseResource\n");
	return 0;
}
