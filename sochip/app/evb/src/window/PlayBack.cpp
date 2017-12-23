#include "PlayBack.h"
#undef LOG_TAG
#define LOG_TAG "PlayBack"
#include "debug.h"

int PlayBack::Fastforwardandrewind(bool flag,int keystate)
{
	if(true == flag){
		seek(5000);				//right true  2000 = step 2S
		updateProgressBar();		
	}else if(false == flag){
		seek(-5000);			//left false
		updateProgressBar();		
	}
	if(MSG_KEYUP == keystate){
		if(getCurrentState() == STATE_STARTED){
			PlayBackPlayStatus(PlayStart);
		}else if(getCurrentState() == STATE_PAUSED)	{
			PlayBackPlayStatus(PlayStop);
		}
	}		
	return 0;
}

int PlayBack::keyProc(int keyCode, int isLongPress)
{
	HWND hIcon;
	switch(keyCode) {
	case CDR_KEY_OK:
		break;
	case CDR_KEY_MENU:
	case CDR_KEY_MODE:
	case CDR_KEY_POWER:
		mIZetaMediaPlayer->stop();
		usleep(100 * 1000);
		mCdrMain->stopPreviewOutside(0);		
		return WINDOWID_PLAYBACKPREVIEW;
		break;
	case CDR_KEY_LEFT:			
		break;
	case CDR_KEY_RIGHT:
		break;
	}
	return WINDOWID_PLAYBACK;
}


int PlayBackProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	PlayBack *playBack = (PlayBack*)GetWindowAdditionalData(hWnd);
	switch(message) {
	case MSG_CREATE:
		db_msg("PlayBackProc: --------MSG_CREATE\n");
		if(playBack->createSubWidgets(hWnd) < 0) {
			db_error("create subWidgets failed\n");
			break;
		}
		break;
	case MSG_CDR_KEY:
		return playBack->keyProc(wParam, lParam);
	case MSG_PREPARE_PLAYBACK:
		{
			std::string* file;
			file = (std::string*)wParam;
			db_msg("file is %s\n", file->c_str());
			playBack->mCurId = (int)lParam;
			return playBack->PreparePlayBack(*file);
		}
		break;
	case MWM_CHANGE_FROM_WINDOW:
		{
			std::string file = "/mnt/extsd/out.mp4";
			db_msg("MWM_CHANGE_FROM_WINDOW:---------------\n");		
			//playBack->prepareCamera4Playback();		/* 画面显示出来前clean layer */
		//	playBack->startPlayBack();
			playBack->PreparePlayBack(file);
			db_msg("xxxxxx\n");
			usleep(200 * 1000);		/* 延时， 保证在layer被渲染后绘制窗口, 避免闪屏 */
			set_ui_alpha(150);
			db_msg("xxxxxx\n");
		}
		break;
	case MSG_TIMER:
		if(wParam == PLAYBACK_TIMER)
		{
			#ifdef PLAY_LOOP
			playBack->sendToPBP(0);
			#endif
			playBack->updateProgressBar();
		}
		break;
	case MSG_PLB_COMPLETE:
		db_msg("MSG_PLB_COMPLETE: wParam is %d\n", wParam);
		playBack->sendToPBP(playBack->mCurId);
		#if 0
		playBack->mCurrentState = STATE_PLAYBACK_COMPLETED;
		if(IsTimerInstalled(hWnd, PLAYBACK_TIMER) == TRUE)
			KillTimer(hWnd, PLAYBACK_TIMER);
		set_ui_alpha(255);
		if (wParam == 1) {
			playBack->reset();
			/* if wParam is 1, the MSG_PLB_COMPLETE is send from the onCompletion, so playBack->stop is not need */
			//SendMessage(GetParent(hWnd), MWM_CHANGE_WINDOW, WINDOWID_PLAYBACK, WINDOWID_PLAYBACKPREVIEW);
			#ifndef PLAY_LOOP
			playBack->sendToPBP(playBack->mCurId);
			#endif
			playBack->noWork(true);
		} else {
			/* if wParam is 0, the MSG_PLB_COMPLETE is send from outside, the caller expected to stop the playback, so need to call playBack->stop */
			playBack->mStopFlag = 1;		/* set the flag, tell the onCompletion not send the MSG_PLB_COMPLETE msg */
			playBack->stop();
			playBack->reset();
		}
		#endif
		break;
	case MSG_SET_VOLUME:
		playBack->setPlayerVol(wParam);
		break;
	case MSG_SHOWWINDOW:
		if(wParam != SW_SHOWNORMAL) {
			playBack->releaseResource();
		}
		break;
	}

	return DefaultWindowProc(hWnd, message, wParam, lParam);
}

void PlayBack::sendToPBP(int mId)
{
	//HWND mhwnd = mCdrMain->getWindowHandle(WINDOWID_PLAYBACKPREVIEW);
	//SendMessage(mhwnd,MSG_SET_CURRENT_FILEID,mId,0);
	mCdrMain->changeWindow(WINDOWID_PLAYBACK, WINDOWID_PLAYBACKPREVIEW);
	mCdrMain->stopPreviewOutside(0);
	usleep(100 * 1000);	
}

void PlayBack::setPlayerVol(int val)
{
	db_msg("by hero nothing setPlayerVol\n");
}

void PlayBack::stopPlaying()
{
	if ((mCurrentState == STATE_STARTED) ||(mCurrentState == STATE_PAUSED) ) {
		SendMessage(mHwnd, MSG_PLB_COMPLETE, 1, 0);
	}
}

PlayBack::PlayBack(CdrMain *cdrMain) : 
	mCurrentState(STATE_IDLE), 
	mTargetState(STATE_IDLE), 
	mStopFlag(0), 
	bmpIcon()
{
	RECT rect;
	HWND hMainWnd = cdrMain->getHwnd();
	db_msg("PlayBack:-----------------------\n");
	GetWindowRect(hMainWnd, &rect);

	mHwnd = CreateWindowEx(WINDOW_PLAYBACK, "",
			WS_NONE,
			WS_EX_TRANSPARENT | WS_EX_USEPARENTFONT,
			WINDOWID_PLAYBACK,
			0, 0, RECTW(rect), RECTH(rect),
			hMainWnd, (DWORD)this);
	if(mHwnd == HWND_INVALID) {
		return;
	}
	showWindow(SW_HIDE);

	mCdrMain = cdrMain;
	mIZetaMediaPlayer = new zeta::iZetaMediaPlayer();
	mPlayBackListener = new PlayBackListener(this);
	
	ResourceManager *rm = ResourceManager::getInstance();
	rm->setHwnd(WINDOWID_PLAYBACK, mHwnd);
	mCurId=0;
}

PlayBack::~PlayBack()
{

}

void PlayBack::cdrplaybackicon(HWND hWnd,ResourceID mId,int iCmdShow)
{
	int retval;
	CDR_RECT rect;
	ResourceManager* rm;
	HWND retWnd;

	rm = ResourceManager::getInstance();
	retval = rm->getResBmp(mId, BMPTYPE_BASE, bmpIcon);
	if(retval < 0) {
		db_error("get current playback icon bmp failed\n");
		//return -1;
	}	
	rm->getResRect(mId, rect);	
	db_msg("luo rect.x=%d rect.y=%d rect.w=%d rect.h=%d \n ",rect.x,rect.y,rect.w,rect.h);
	
	retWnd = CreateWindowEx(CTRL_STATIC, "",
			WS_VISIBLE | WS_CHILD | SS_BITMAP | SS_CENTERIMAGE,
			WS_EX_TRANSPARENT,
			mId,		
			rect.x, rect.y,rect.w,rect.h,
			hWnd, (DWORD)&bmpIcon);
	
	if(retWnd == HWND_INVALID) {
		db_error("create playback icon label failed\n");
		//return -1;
	}
	ShowWindow(retWnd, iCmdShow);	
//	return 0;
}

void PlayBack::PlayBackPlayStatus(PlayStatus Status)
{
	switch(Status){
		case PlayStart:
			cdrplaybackicon(mHwnd,ID_PLAYBACK_ICON3,SW_HIDE);
			cdrplaybackicon(mHwnd,ID_PLAYBACK_ICON2,SW_HIDE);
			cdrplaybackicon(mHwnd,ID_PLAYBACK_ICON1,SW_HIDE);
			cdrplaybackicon(mHwnd,ID_PLAYBACK_ICON,SW_SHOWNORMAL);
			break;
		case PlayStop:
			cdrplaybackicon(mHwnd,ID_PLAYBACK_ICON3,SW_HIDE);
			cdrplaybackicon(mHwnd,ID_PLAYBACK_ICON2,SW_HIDE);
			cdrplaybackicon(mHwnd,ID_PLAYBACK_ICON,SW_HIDE);
			cdrplaybackicon(mHwnd,ID_PLAYBACK_ICON1,SW_SHOWNORMAL);
			break;
		case PlayRewind:
			cdrplaybackicon(mHwnd,ID_PLAYBACK_ICON3,SW_HIDE);			
			cdrplaybackicon(mHwnd,ID_PLAYBACK_ICON1,SW_HIDE);			
			cdrplaybackicon(mHwnd,ID_PLAYBACK_ICON,SW_HIDE);			
			cdrplaybackicon(mHwnd,ID_PLAYBACK_ICON2,SW_SHOWNORMAL);
			break;
		case PlayForward:			
			cdrplaybackicon(mHwnd,ID_PLAYBACK_ICON2,SW_HIDE);							
			cdrplaybackicon(mHwnd,ID_PLAYBACK_ICON1,SW_HIDE);							
			cdrplaybackicon(mHwnd,ID_PLAYBACK_ICON,SW_HIDE);				
			cdrplaybackicon(mHwnd,ID_PLAYBACK_ICON3,SW_SHOWNORMAL);
			break;
		default:
			db_msg("luo PlayBackPlayStatus error\n");
			break;			
	}
}

int PlayBack::createSubWidgets(HWND hWnd)
{
	int retval;
	CDR_RECT rect;
	ResourceManager* rm;
	HWND retWnd;
	db_msg("createSubWidgets:------------\n");

	rm = ResourceManager::getInstance();

	ProgressBarData_t PGBData;
	rm->getResRect(ID_PLAYBACK_PGB, rect);
	PGBData.bgcWidget = rm->getResColor(ID_PLAYBACK_PGB, COLOR_BGC);
	PGBData.fgcWidget = rm->getResColor(ID_PLAYBACK_PGB, COLOR_FGC);
	retWnd = CreateWindowEx(CTRL_CDRPROGRESSBAR, NULL,
			WS_VISIBLE,
			WS_EX_NONE,
			ID_PLAYBACK_PGB,		
			rect.x, rect.y, rect.w, rect.h,
			hWnd, (DWORD)&PGBData);
	if(retWnd == HWND_INVALID) {
		db_error("create playback progress bar failed\n");
		return -1;
	}

	return 0;
}

int PlayBack::startPlayBack()
{
	HWND hIcon;

	PlayBackPlayStatus(PlayStart);
	db_msg("xxxxxxxxxxxxxxx\n");
	SetTimer(mHwnd, PLAYBACK_TIMER, PROGRESSBAR_REFRESH_TIME);
	initProgressBar();
	mStopFlag = 0;
	start();

	return 0;
}

int PlayBack::PreparePlayBack(std::string filePath)
{
	zeta::ZetaDisplay *mZD = mCdrMain->get_ZetaDisp();
	mIZetaMediaPlayer->setOnPreparedListener(mPlayBackListener);
	mIZetaMediaPlayer->setOnCompletionListener(mPlayBackListener);
	db_msg("PreparePlayBack: filePath:%s\n", filePath.c_str());

	mCdrMain->stopPreviewOutside(1);
	usleep(100 * 1000);
	mIZetaMediaPlayer->setDisplay(mZD->getHandle());			
	mIZetaMediaPlayer->setDataSource(filePath.c_str());		
	mIZetaMediaPlayer->prepare();
	db_msg("PreparePlayBack: mIZetaMediaPlayer->prepare()\n");
	return 0;
}

playerState PlayBack::getCurrentState()
{
	return mCurrentState;
}

void PlayBack::setDisplay(int hlay)
{
	db_msg("by hero nothing setDisplay\n");
}

int PlayBack::preparePlay(std::string filePath)
{
	db_msg("by hero nothing preparePlay\n");
	return 0;
}

void PlayBack::start()
{
	db_msg("by hero nothing start\n");
}

void PlayBack::pause()
{
	db_msg("by hero nothing pause\n");
}

bool PlayBack::isStarted()
{
	return (mTargetState == STATE_STARTED);
}

bool PlayBack::isPause()
{
	return (mTargetState == STATE_PAUSED);
}
void PlayBack::seek(int msec)
{
	db_msg("by hero nothing seek\n");
}

void PlayBack::stop()
{
	db_msg("by hero nothing stop\n");
}

void PlayBack::release()
{
	db_msg("by hero nothing release\n");
}

void PlayBack::reset()
{
	db_msg("by hero nothing reset\n");
}

void PlayBack::noWork(bool idle)
{
	db_msg("by hero nothing noWork\n");
}

void PlayBack::PlayBackListener::onPrepared(iZetaMediaPlayer *PiZetaMediaPlayer)
{

	db_msg("PlayBackListener::onPrepared:-------------\n");
	mPB->mCurrentState = STATE_PREPARED;
	//if(mPB->mTargetState == STATE_STARTED)
	{
		db_msg("PlayBackListener::onPrepared:start ZetaMediaPlay\n");
		PiZetaMediaPlayer->start();
	}		
#if 0
	mPB->mCurrentState = STATE_PREPARED;
	if(mPB->mTargetState == STATE_STARTED)
	{
		db_msg("start CedarMediaPlayer\n");
		mPB->start();
	}
#endif	
}
void PlayBack::PlayBackListener::onCompletion(iZetaMediaPlayer *PiZetaMediaPlayer)
{
	db_msg("PlayBackListener::onCompletion:-------------\n");
	if(mPB->mStopFlag == 1) {
		mPB->mStopFlag = 0;
	} else {
		SendMessage(mPB->getHwnd(), MSG_PLB_COMPLETE, 1, 0);
	}
#if 0
	if(mPB->mStopFlag == 1) {
		mPB->mStopFlag = 0;
	} else {
		SendMessage(mPB->getHwnd(), MSG_PLB_COMPLETE, 1, 0);
	}
#endif
	
}
void PlayBack::initProgressBar(void)
{
	db_msg("by hero nothing initProgressBar\n");
}

void PlayBack::resetProgressBar(void)
{
	db_msg("by hero nothing resetProgressBar\n");
}

void PlayBack::updateProgressBar(void)
{
	
	db_msg("by hero nothing updateProgressBar\n");
	
}

void PlayBack::prepareCamera4Playback(void)
{
	db_msg("by hero nothing prepareCamera4Playback\n");
}

int PlayBack::stopPlayback(bool bRestoreAlpha)
{
	db_msg("stop playback\n");
	if(IsTimerInstalled(mHwnd, PLAYBACK_TIMER) == TRUE)
		KillTimer(mHwnd, PLAYBACK_TIMER);
	stop();
	reset();
	if (bRestoreAlpha) {
		set_ui_alpha(255);
	}
	return 0;
}
#ifdef PLATFORM_0
int PlayBack::restoreAlpha()
{
	return set_ui_alpha(255);
}
#endif

int PlayBack::releaseResource()
{
	return 0;
}
