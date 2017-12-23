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
#define LOG_TAG "mode_preview.cpp"
#include "debug.h"

using namespace zeta;

int ModePreview::keyProc(int keyCode, int isLongPress)
{
	switch(keyCode) {
	case CDR_KEY_OK:
	case CDR_KEY_MODE:
	case CDR_KEY_LEFT:
	case CDR_KEY_RIGHT:
		InvalidateRect(mMainHwnd, NULL, TRUE);		
		ShowModePreview(keyCode);
		break;
	case CDR_KEY_POWER:	
		return WINDOWID_MAINPREVIEW;
	default:
		break;
	}
	
	return WINDOWID_MODEPREVIEW;
}

int ModePreview::HandleTouchMsg(int keyCode)
{
	SendMessage(mCdrMain->getHwnd(),MWM_CHANGE_WINDOW,WINDOWID_MODEPREVIEW,keyCode);
	return 0;
}

int ModePreview::ShowModePreview(int keyCode)
{
	//HDC hdc=BeginPaint(mMainHwnd);
	HDC hdc = GetClientDC(mMainHwnd);
	SetBrushColor (hdc, RGBA2Pixel (hdc, 0x00, 0x00, 0x00, 0x00));

	SetBkMode(hdc, BM_TRANSPARENT);    
	SetTextColor(hdc, RGB2Pixel(hdc, 0x44, 0x80, 0xEE));	//RGB2Pixel (mem_dc, 0x00, 0x00, 0xFF));
		
	switch(keyCode) {
	case CDR_KEY_OK:
		TabbedTextOut(hdc, 350, 280, "KEY K11 Down...");
		break;
	case CDR_KEY_POWER:	
		TabbedTextOut(hdc, 350, 280, "KEY Power Down...");
		break;
	case CDR_KEY_MODE:
		TabbedTextOut(hdc, 350, 280, "KEY K10 Down...");
		break;
	case CDR_KEY_LEFT:
		TabbedTextOut(hdc, 350, 280, "KEY K8 Down...");
		break;
	case CDR_KEY_RIGHT:
		TabbedTextOut(hdc, 350, 280, "KEY K9 Down...");
		break;
	default:
		break;
	}
	EndPaint(mMainHwnd,hdc); 
	return WINDOWID_MODEPREVIEW;
}

int ModePreviewProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    ModePreview* mMP = (ModePreview*)GetWindowAdditionalData(hWnd);
	ResourceManager* mRM = ResourceManager::getInstance();
	HDC hdc;
	static bool bdraw = false;
	static int pre_x, pre_y;	
    switch(message) {
		case MSG_CREATE:
			mMP->createSubWidgets(hWnd);
			break;		
		case MSG_PAINT:			
			break;		
		case MSG_CDR_KEY:
			return mMP->keyProc(wParam, lParam);			
		case MSG_LBUTTONDOWN:
			bdraw = true;
			//SetCapture(hWnd);
			pre_x = LOWORD (lParam);
			pre_y = HIWORD (lParam);
			break;
		case MSG_MOUSEMOVE:
		{
			int x = LOWORD (lParam);
			int y = HIWORD (lParam);			
			if(bdraw) {
				//ScreenToClient(hWnd, &x, &y);
				hdc = GetClientDC(hWnd);
				//SetPenWidth (hdc, 5);
				SetPenColor(hdc, PIXEL_red);
				MoveTo(hdc, pre_x, pre_y);
				LineTo(hdc, x, y);
				//ReleaseDC(hdc);
				pre_x = x;
				pre_y = y;
				EndPaint(hWnd,hdc); 
			}
			return 0;
		}
		case MSG_LBUTTONUP:
			bdraw = false;		
			//ReleaseCapture();
			InvalidateRect(hWnd, NULL, TRUE);			
			break;
		case MSG_RBUTTONDOWN:
			InvalidateRect(hWnd, NULL, TRUE);
			break;		
	    case MSG_CLOSE:
			DestroyAllControls (hWnd);
			DestroyMainWindow (hWnd);
			PostQuitMessage (hWnd);
            return 0;
    }
	return DefaultWindowProc(hWnd, message, wParam, lParam);
}

ModePreview::ModePreview(CdrMain* pCdrMain)
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
	mHwnd = CreateWindowEx(WINDOW_MODEPREVIEW, "",
			WS_NONE,
			WS_EX_USEPARENTFONT,
			WINDOWID_MODEPREVIEW,
			rect.left, rect.top, RECTW(rect), RECTH(rect),
			hParent, (DWORD)this);

    db_msg(" mHwnd :%d ", mHwnd);
    
	if(mHwnd == HWND_INVALID) {
		db_error("create Main Preview window failed\n");
		return;
	}

    SetWindowBkColor(mHwnd, RGBA2Pixel(HDC_SCREEN,  0x00, 0x00, 0x00, 0xFF)); 
	mRM->setHwnd(WINDOWID_MODEPREVIEW, mHwnd);
	mMainHwnd = mHwnd;
	ShowWindow(mHwnd, SW_HIDE);
	
}

ModePreview::~ModePreview()
{
	if(back_image.bmBits != NULL)
		UnloadBitmap(&back_image); 
	db_msg(" clear ~ModePreview");
}

int ModePreview::releaseResource()
{
	return 0;
}

int ModePreview::createSubWidgets(HWND hParent)
{
	HWND Hwnd;
	ResourceManager* mRM;
	CDR_RECT rect;
	int image_count = 0;
	mRM = ResourceManager::getInstance();

	mRM->getResRect(ID_MP_BJ_WINDOW, rect);
	
	Hwnd = CreateWindowEx(CTRL_STATIC, "",
			WS_CHILD | WS_VISIBLE | SS_CENTER,
			WS_EX_TRANSPARENT | WS_EX_USEPARENTFONT,
			ID_TP_BACK_WINDOW,
			rect.x, rect.y, rect.w, rect.h,
			hParent, 0);
	if(Hwnd == HWND_INVALID) {
		db_error("create playback previe image label failed\n");
		return -1;
	}	
	SetWindowElementAttr(Hwnd, WE_FGC_WINDOW,  Pixel2DWORD(HDC_SCREEN, 0xFFDCDCDC));
	SetWindowText(Hwnd, "Pls   press   the   Power   Button   to   return   back.");
	return 0;
}



