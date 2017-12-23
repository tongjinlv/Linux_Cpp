
#include "windows.h"
#include "coolIndicator.h"
#undef LOG_TAG
#define	 LOG_TAG "Menu.cpp"
#include "debug.h"

#define IDC_MENULIST			400
#define MENU_LIST_ITEM_HEIGHT	49
using namespace zeta;
int MenuProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	Menu* menu;
	db_msg("message is %s, wParam: 0x%x, lParam: 0x%lx\n", Message2Str(message), wParam, lParam);
	switch(message) {
		case MSG_CREATE:
			/* for switch ui Bug: ui from others to RecordPreview will not refresh */
			RECT rect;
			HWND hMainWin;
			HDC secDC, MainWindowDC;

			hMainWin = GetMainWindowHandle(hWnd);
			GetClientRect(hMainWin, &rect);
			secDC = GetSecondaryDC(hMainWin);
			MainWindowDC = GetDC(hMainWin);

			SetMemDCAlpha(secDC, 0, 0);
			BitBlt(secDC, 0, 0, RECTW(rect), RECTH(rect), secDC, 0, 0, 0);

			ReleaseDC(MainWindowDC);
			break;
		case MSG_CDR_KEY:
			menu = (Menu*)GetWindowAdditionalData(hWnd);
			return menu->keyProc(wParam, lParam);
		default:
			break;
	}

	return DefaultWindowProc(hWnd, message, wParam, lParam);
}

Menu::Menu(CdrMain* pCdrMain) : 
	mCdrMain(pCdrMain),
	mHwndIndicator(HWND_INVALID)
{
	CDR_RECT rect;
	ResourceManager* rm;

	db_info("Menu Constructor\n");

	rm = ResourceManager::getInstance();
	rm->getResRect(ID_MENU_LIST, rect);

	mHwnd = CreateWindowEx(WINDOW_MENU, "",
			WS_NONE,
			WS_EX_USEPARENTFONT,
			WINDOWID_MENU,
			rect.x, rect.y, rect.w, rect.h,
			mCdrMain->getHwnd(), (DWORD)this);
	if(mHwnd == HWND_INVALID) {
		db_error("create menu window failed\n");
		return;
	}
	ShowWindow(mHwnd, SW_HIDE);

	// ------------------ create menu indicator -----------------
	rm->getResRect(ID_MENU_LIST_INDICATOR, rect);
	mHwndIndicator = CreateWindowEx (CTRL_COOL_INDICATOR, "",
		WS_CHILD | WS_VISIBLE | WS_BORDER | CBS_BMP_CUSTOM, 
		WS_EX_USEPARENTFONT,
		ID_MENU_LIST_INDICATOR,
		rect.x, rect.y, rect.w, rect.h,
		mHwnd,
		MAKELONG(32, 28));
	if(mHwndIndicator == HWND_INVALID ) {
		db_error("create menu Indicator failed\n");
		return;
	}
	SetWindowBkColor(mHwndIndicator, rm->getResColor(ID_MENU_LIST_INDICATOR, COLOR_BGC));
	rm->setHwnd(WINDOWID_MENU, mHwnd);
}

Menu::~Menu()
{
}

int Menu::keyProc(int keyCode, int isLongPress)
{
	switch(keyCode) {
		case CDR_KEY_OK:
			return WINDOWID_MAINPREVIEW;
		case CDR_KEY_MENU:
		case CDR_KEY_MODE:
		case CDR_KEY_POWER:
		default:
			break;
	}

	return WINDOWID_MENU;
}

CdrMain * Menu::getCdrMain()
{
	if(mCdrMain)
		return mCdrMain;
	return NULL;
}
int Menu::releaseResource()
{
	return 0;
}

