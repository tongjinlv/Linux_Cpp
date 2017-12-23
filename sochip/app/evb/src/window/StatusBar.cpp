
#include "StatusBar.h"
#undef LOG_TAG
#define LOG_TAG "StatusBar.cpp"
#include "debug.h"

using namespace zeta;

int StatusBarProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	//db_msg("-----statusbar Proc message is %s  the wParamis %d---\n", Message2Str(message), wParam);
	StatusBar* stb = (StatusBar*)GetWindowAdditionalData(hWnd);
 	switch(message) {
	case MSG_FONTCHANGED:
		stb->updateWindowFont();
		break;
	case MSG_CREATE:
		{
			if(stb->createSTBWidgets(hWnd) < 0) {
				db_error("create sub widgets failed\n");	
			}
		}
		break;
	case MSG_MOUSEMOVE:
	{
		int x = LOWORD (lParam);
		int y = HIWORD (lParam);			
		db_msg("by hero StatusBarProc x = %d, y = %d\n", x, y);
		return 0;
	}		
	case MSG_SHOWWINDOW:
		break;
	case STBM_SHOW_BACKBT:
		stb->setStatusBarIcon(ID_STATUSBAR_ICON_BACK, wParam);
		stb->mIconOps->showIcon(ID_STATUSBAR_ICON_BACK);
		break;
	case STBM_SHOW_RECBT:
		stb->setStatusBarIcon(ID_STATUSBAR_ICON_REC, wParam);
		stb->mIconOps->showIcon(ID_STATUSBAR_ICON_REC);
		break;
	case STBM_SHOW_PICBT:
		stb->setStatusBarIcon(ID_STATUSBAR_ICON_PIC, wParam);
		stb->mIconOps->showIcon(ID_STATUSBAR_ICON_PIC);
		break;		
	case STBM_SHOW_HWND:
		if(wParam == WINDOWID_RECORDPREVIEW){
			ShowWindow(stb->mHwndTop, SW_SHOWNORMAL);
			ShowWindow(stb->mHwndBottom, SW_SHOWNORMAL);
		}else{
			stb->setStatusBarIcon(ID_STATUSBAR_ICON_TFCARD, 0);
			stb->mIconOps->showIcon(ID_STATUSBAR_ICON_TFCARD);	
			stb->setStatusBarIcon(ID_STATUSBAR_ICON_REC, 0);
			stb->mIconOps->showIcon(ID_STATUSBAR_ICON_REC);
			stb->setStatusBarIcon(ID_STATUSBAR_ICON_PIC, 0);
			stb->mIconOps->showIcon(ID_STATUSBAR_ICON_PIC);		
			ShowWindow(stb->mHwndTop, SW_HIDE);
			ShowWindow(stb->mHwndBottom, SW_HIDE);
		}
		break;
	case MSG_TIMER:
		break;
	case MSG_DESTROY:
		break;
	default:
		stb->msgCallback(message, wParam);
	}

	return DefaultWindowProc(hWnd, message, wParam, lParam);
}

//			StatusBar
//			|      |
// statusBarTop   statusBarBottom
StatusBar::StatusBar(CdrMain *cdrMain): 
	mCdrMain(cdrMain),
	mHwndTop(HWND_INVALID),
	mHwndBottom(HWND_INVALID)
{
	RECT rect;
	ResourceManager *rm;

	mCurWindowState.curWindowID = WINDOWID_MAINPREVIEW;
	mCurWindowState.RPWindowState = RPWINDOWSTATE_INVALID;

	rm = ResourceManager::getInstance();
	mIconOps = new CdrIconOps();
	mLabelOps = new CdrLabelOps();

	GetWindowRect(mCdrMain->getHwnd(), &rect);
	
	// create StatusBar Window, tranparent and Fill up the screen
	mHwnd = CreateWindowEx(WINDOW_STATUSBAR, "",
			WS_VISIBLE | WS_CHILD,
			WS_EX_NONE | WS_EX_USEPARENTFONT | WS_EX_TRANSPARENT,
			WINDOWID_STATUSBAR,
			0, 0, RECTW(rect), RECTH(rect),
			mCdrMain->getHwnd(), (DWORD)this);
	if(mHwnd == HWND_INVALID) {
		db_error("create status bar failed\n");
		return;
	}
	rm->setHwnd(WINDOWID_STATUSBAR, mHwnd);

	ShowWindow(mHwnd, SW_HIDE);
	
}

StatusBar::~StatusBar()
{
	db_msg("StatusBar Destructor\n");
	if(mIconOps != NULL) {
		delete mIconOps;
		mIconOps = NULL;
	}

	if(mLabelOps != NULL) {
		delete mLabelOps;
		mLabelOps = NULL;
	}
}

// 1. create statusBarTop widget and StatusBarBottom widget
// 2. create icons and labels on the statusBarTop and StatusBarBottom
// 3. set icons' attr or labels' attr
int StatusBar::createSTBWidgets(HWND hParent)
{
	CDR_RECT rectStatusBar;
	RECT rectWindow;
	ResourceManager *rm;
	XCreateParams createParam;
	CdrLabel* label = NULL;
	int retval = 0;

	rm = ResourceManager::getInstance();

	mBgColor = rm->getResColor(ID_STATUSBAR, COLOR_BGC);
	mFgColor = rm->getResColor(ID_STATUSBAR, COLOR_FGC);
	rm->getResRect(ID_STATUSBAR, rectStatusBar);
	GetWindowRect(mCdrMain->getHwnd(), &rectWindow);

	// 1. **************************************************
	// ---------- create statusBarTop ------------
	mHwndTop = CreateWindowEx(CTRL_STATIC, "",
			WS_CHILD,
			WS_EX_NONE | WS_EX_USEPARENTFONT,
			ID_STATUSBAR_TOP,
			0, 0, rectStatusBar.w, rectStatusBar.h,
			hParent, (DWORD)this);
	if(mHwndTop == HWND_INVALID) {
		db_error("create status bar failed\n");
		return -1;
	}
	SetWindowBkColor(mHwndTop, mBgColor);
	// ++++++++++ create statusBarTop ++++++++++++
	
	// ---------- create statusBarBottom ------------
	mHwndBottom = CreateWindowEx(CTRL_STATIC, "",
			WS_CHILD,
			WS_EX_NONE | WS_EX_USEPARENTFONT,
			ID_STATUSBAR_BOTTOM,
			0, RECTH(rectWindow) - rectStatusBar.h, rectStatusBar.w, rectStatusBar.h,
			hParent, (DWORD)this);
	if(mHwndBottom == HWND_INVALID) {
		db_error("create status bar failed\n");
		return -1;
	}
	SetWindowBkColor(mHwndBottom, mFgColor);
	// ++++++++++ create statusBarBottom ++++++++++++

	// 2. **************************************************
	db_msg("statusBarTop icon count :%u\n", TABLESIZE(s_IconSetsTop) );
	createParam.style = SS_CENTERIMAGE;
	createParam.exStyle = WS_EX_TRANSPARENT;
	createParam.hParent = mHwndTop;
	for(unsigned int i = 0; i < TABLESIZE(s_IconSetsTop); i++ ) {
		if( mIconOps->createCdrIcon(&s_IconSetsTop[i], &createParam) < 0)
			return -1;
	}
	db_msg("statusBarBottom icon count :%u\n", TABLESIZE(s_IconSetsBottom) );
	createParam.hParent = mHwndBottom;
	for(unsigned int i = 0; i < TABLESIZE(s_IconSetsBottom); i++ ) {
		if( mIconOps->createCdrIcon(&s_IconSetsBottom[i], &createParam) < 0)
			return -1;
	}
	db_msg("mIconOps size :%zu\n", mIconOps->size());


	db_msg("statusBarTop label count :%u\n", TABLESIZE(s_LabelSetsTop) );
	createParam.style = SS_SIMPLE;
	createParam.exStyle = WS_EX_TRANSPARENT | WS_EX_USEPARENTFONT;
	createParam.hParent = mHwndTop;
	for(unsigned int i = 0; i < TABLESIZE(s_LabelSetsTop); i++) {
		if( mLabelOps->createCdrLabel(&s_LabelSetsTop[i], &createParam) < 0 )
			return -1;
	}
	db_msg("statusBarBottom label count :%u\n", TABLESIZE(s_LabelSetsBottom) );
	createParam.hParent = mHwndBottom;
	for(unsigned int i = 0; i < TABLESIZE(s_LabelSetsBottom); i++) {
		if( mLabelOps->createCdrLabel(&s_LabelSetsBottom[i], &createParam) < 0 )
			return -1;
	}
	db_msg("mLabel size :%zu\n", mLabelOps->size());

	// hide icons not show on the recordPreview window
	//mIconOps->hideIcon(ID_STATUSBAR_ICON_PHOTO_RESOLUTION);
	//mIconOps->showIcon(ID_STATUSBAR_ICON_BAT);
	// 2. **************************************************
	// set attributes
	#if 0
	label = mLabelOps->getLabel(ID_STATUSBAR_LABEL1);
	if(label != NULL) {
		label->setAlign(CdrLabel::alLeft);
		db_msg("status bar fgc is 0x%lx\n", Pixel2DWORD(HDC_SCREEN, mFgColor));
		label->setTextColor(mFgColor);
	}
	#endif
	mIconOps->showIcon(ID_STATUSBAR_ICON_BACK);
	mIconOps->showIcon(ID_STATUSBAR_ICON_REC);
	mIconOps->showIcon(ID_STATUSBAR_ICON_PIC);
	ShowWindow(mHwndTop, SW_HIDE);
	ShowWindow(mHwndBottom, SW_HIDE);
	return 0;
}
	
void StatusBar::msgCallback(int msgType, unsigned int data)
{
	db_msg("recived msg %x, data is 0x%x\n", msgType, data);
}

void StatusBar::setStatusBarIcon(ResourceID id, unsigned int index)
{
	if(mIconOps != NULL) {
		mIconOps->setIconIndex(id, index);
	}
}
void StatusBar::setCurrentWindowState(windowState_t windowState)
{
	db_msg("setCurrentWindowState, curWindowID is %d\n", windowState.curWindowID);
	#if 0
	if(windowState.curWindowID < WINDOWID_INVALID && windowState.RPWindowState < RPWINDOWSTATE_INVALID) {
		mCurWindowState = windowState;
		updateWindowPic();
	}
	#endif
}

void StatusBar::setCurrentRPWindowState(RPWindowState_t status)
{
	db_msg("setRPwindowState, state is %d\n", status);
	#if 0
	if(status < RPWINDOWSTATE_INVALID) {
		mCurWindowState.RPWindowState = status;
		updateWindowPic();
	}
	#endif
}

RPWindowState_t StatusBar::getCurrentRPWindowState(void)
{
	return mCurWindowState.RPWindowState;
}

void StatusBar::setCurWindowId(unsigned int windowId)
{
	db_msg("setCurWindowId, curWindowID is %d\n", windowId);
	if(windowId < WINDOWID_INVALID) {
		mCurWindowState.curWindowID = windowId;
		//updateWindowPic();
	}
}


unsigned int StatusBar::getCurWindowId()
{
	return mCurWindowState.curWindowID;
}

// 1. show and hide statusBarTop or statusBarBottom
// 2. turn on or turn off the alpha element of the statusBar back color
// 3. show or hide icons in different window
void StatusBar::updateWindowPic(void)
{
#if 1
	ResourceManager *rm;
	RPWindowState_t state;
	unsigned int windowId;
	rm = ResourceManager::getInstance();
	windowId = getCurWindowId();
	state = mCurWindowState.RPWindowState;
	// 1. *************************************************
	/*
	 * in RecordPreview show statusBarBottom
	 * outside RecordPreview hide statusBarBottom
	 * */
	db_msg("by hero *** updateWindowPic windowId = %d\n", windowId);
	if(windowId == WINDOWID_RECORDPREVIEW) {
		db_msg("show statusBar\n");
		ShowWindow(mHwndTop, SW_SHOWNORMAL);
		ShowWindow(mHwndBottom, SW_SHOWNORMAL);
	}else {	
		db_msg("hide statusBar\n");
		ShowWindow(mHwndTop, SW_HIDE);
		ShowWindow(mHwndBottom, SW_HIDE);
	}
#endif
}

// --------------------------------------------------------------------
// in RecordPreview window statusBar may have alpha value
// outside RecordPreview window statusBar should turn off alpha value
void StatusBar::changeStatusBarAlpha(void)
{
	//db_msg("mBgColor DWORD is 0x%lx\n", Pixel2DWORD(HDC_SCREEN, mBgColor));
	if( GetAValue(Pixel2DWORD(HDC_SCREEN, mBgColor)) != 0xFF ) {
		// check if the backgroud color have alpha value
		gal_pixel colorTop, colorBottom, newColor;
		DWORD dwordTop, dwordBottom;
		colorTop = GetWindowBkColor(mHwndTop);
		dwordTop = Pixel2DWORD(HDC_SCREEN, colorTop);
		colorBottom = GetWindowBkColor(mHwndBottom);
		dwordBottom = Pixel2DWORD(HDC_SCREEN, colorBottom);
		//db_msg("dwordTop is 0x%lx\n", dwordTop);
		//db_msg("dwordBottom is 0x%lx\n", dwordBottom);
		if(getCurWindowId() == WINDOWID_RECORDPREVIEW) {
			// turn on statusBar alpha value
			if(GetAValue(dwordTop) == 0xFF) {
				SetWindowBkColor(mHwndTop, mBgColor);
				InvalidateRect(mHwndTop, NULL, TRUE);
			}
			if(GetAValue(dwordBottom) == 0xFF) {
				SetWindowBkColor(mHwndBottom, mFgColor);
				InvalidateRect(mHwndBottom, NULL, TRUE);
			}
		} else {
			// turn off statusBar alpha value
			//db_msg("dwordTop A Value is 0x%x\n", GetAValue(dwordTop));
			//db_msg("dwordBottom A Value is 0x%x\n", GetAValue(dwordBottom));
			if(GetAValue(dwordTop) < 0xFF) {
				newColor = RGBA2Pixel(HDC_SCREEN, GetRValue(dwordTop), GetGValue(dwordTop), GetBValue(dwordTop), 0xFF);
				SetWindowBkColor(mHwndTop, newColor);
				//db_msg("newColor DWORD is 0x%lx\n", Pixel2DWORD(HDC_SCREEN, newColor) );
				InvalidateRect(mHwndTop, NULL, TRUE);
			}
			if(GetAValue(dwordBottom) < 0xFF) {
				newColor = RGBA2Pixel(HDC_SCREEN, GetRValue(dwordBottom), GetGValue(dwordBottom), GetBValue(dwordBottom), 0xFF);
				SetWindowBkColor(mHwndBottom, newColor);
				//db_msg("newColor DWORD is 0x%lx\n", Pixel2DWORD(HDC_SCREEN, newColor) );
				InvalidateRect(mHwndBottom, NULL, TRUE);
			}
		}
	}
}

int StatusBar::releaseResource()
{
	return 0;
}
