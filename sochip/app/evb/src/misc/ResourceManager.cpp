
#include "ResourceManager.h"

#include <string.h>
#include <minigui/minigui.h>
#include <minigui/window.h>

#include <mutex>

#include "cdr.h"
#include "resource_impl.h"
#include "Rtc.h"

#include "include_net/AWNetInterface.h"
#include "platform.h"
#include "StorageManager.h"
#include "CdrServer.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "ResourceManager.cpp"
#include "debug.h"

static bool mSlowRecordInit = true;
typedef struct {
	const char* mainkey;
	void* addr;
}configTable2;

typedef struct {
	const char* mainkey;
	const char* subkey;
	void* addr;
}configTable3;

typedef struct{
	int vol;
	float voice;
}volumeTable;

volumeTable VolNum[]={{0,0.9},{1,0.6},{2,0.3}};

using namespace zeta;

static ResourceManager *gInstance;
static std::mutex gInstanceMutex;

void ResourceManager::setHwnd(unsigned int win_id, HWND hwnd)
{
	mHwnd[win_id] = hwnd;
}

HWND ResourceManager::getHwnd(unsigned int win_id)
{
	return mHwnd[win_id] ;
}


ResourceManager::ResourceManager()
	: configMenu("/tmp/menu.cfg")
	, defaultConfigMenu("/etc/res/data/menu.cfg")
	, mCfgFileHandle(0)
	, mCfgMenuHandle(0)
	, mRecording(false)
	, mSignaled(false)
{
	int big_w, big_h;
	getScreenInfo(&big_w, &big_h);
	db_msg("-----------------the big_w= %d  : big_h = %d ------------\n",big_w, big_h);
	sprintf(configFile, "/etc/res/cfg/%dx%d.cfg", big_w, big_h);
	defaultConfigFile = configFile;
	memset(mHwnd, 0, sizeof(HWND)*WIN_CNT);
	db_msg("ResourceManager Constructor\n");
	if(getCfgMenuHandle() < 0) {
		db_info("restore menu.cfg\n");
		if(copyFile(defaultConfigMenu, configMenu) < 0) {
			db_error("copyFile config file failed\n");	
		}
	} else {
		if (verifyConfig() == false){
			db_info("restore menu.cfg\n");
			if(copyFile(defaultConfigMenu, configMenu) < 0) {
				db_error("copyFile config file failed\n");
			}
		}
		releaseCfgMenuHandle();
	}
	db_info("by hero nothing ResourceManager end2\n");
	rMenuList.AWMDEnable = false;
	db_info("by hero nothing ResourceManager end3\n");
	#if 0
	int retx = LoadBitmapFromFile(HDC_SCREEN,&mNoVideoImage,"/etc/res/others/no_video_image.png");
	if(retx!= ERR_BMP_OK){
		ALOGD("load the mNoVideoImage bitmap error"); 
		mNoVideoImage.bmWidth= -1;
	}
	#endif
	db_info("by hero nothing ResourceManager end\n");
}

ResourceManager::~ResourceManager()
{
	UnloadBitmap(&mNoVideoImage);
	db_msg("ResourceManager Destructor\n");
}

int ResourceManager::initLangAndFont(LANGUAGE language)
{
#if 1
	PLOGFONT logFont = INV_LOGFONT;
	int ret = -1;

	if(language >= LANG_ERR) {
		db_error("get language failed\n");
		return -1;
	}
	mLogFont_other= CreateLogFont("sxf", "arialuni", "UTF-8",
        FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
		FONT_OTHER_AUTOSCALE, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 36, 0);
		
	mLogFont_ch = CreateLogFont("sxf", "msyh", "UTF-8",
		FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
		FONT_OTHER_AUTOSCALE, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 20, 0);		
#if 0
	if(language == LANG_CN || language == LANG_TW)
		logFont = mLogFont_ch;
	else
#endif
		logFont = mLogFont_other;
	db_msg("by hero *** lang = %d\n", lang);
	if(logFont != INV_LOGFONT) {
		/* init lang labels */
		if(initLabel(language) == 0) {
			lang = language;
			mLogFont = logFont;
			SendMessage(mHwnd[WINDOWID_MAIN], MSG_RM_LANG_CHANGED, (WPARAM)&lang, 0);
			ret = 0;
		} else {
			db_error("initLabel failed, destroy the logFont\n");
			DestroyLogFont(logFont);
		}
	}
#else
	initLabel(language);
#endif

	db_msg("by hero nothing initLangAndFont\n");
	return 0;
}

ResourceManager* ResourceManager::getInstance()
{
	std::unique_lock<std::mutex> lock(gInstanceMutex);
	if(gInstance != NULL) {
		return gInstance;
	}
	
	gInstance = new ResourceManager();
	return gInstance;
}

int ResourceManager::initStage1(void)
{
	int err_code;
	int retval = 0;

	db_msg("initStage1\n");


	if(getCfgFileHandle() < 0) {
		db_error("get file handle failed\n");
		return -1;
	}
	if(getCfgMenuHandle() < 0) {
		db_error("get menu handle failed\n");
		retval = -1;
		goto out;
	}

	err_code = getRectFromFileHandle("rect_screen", &rScreenRect);
	if(err_code != ETC_OK) {
		db_error("get screen rect failed: %s\n", pEtcError(err_code));
		retval = -1;
		goto out;
	}

    if(initMainPreviewResource() < 0) {
        db_error("init MainPreview resource failed\n");
        retval = -1;
        goto out;
    }

	if(initStatusBarResource() < 0) {
		db_error("init status bar resource failed\n");	
		retval = -1;
		goto out;
	}

	if(initMenuResource() < 0) {
		db_error("init menu resource failed\n");
		retval = -1;
		goto out;
	}

	syncIconCurrent();

out:
	releaseCfgFileHandle();
	releaseCfgMenuHandle();
	return retval;
}

int ResourceManager::notifyAll()
{
	return 0;
}
int ResourceManager::initStage2(void)
{
	int retval = 0;
	db_msg("by hero nothing initStage2 start\n");
	//db_msg("initStage2\n");

	initLangAndFont( getLangFromConfigFile() );

	if(getCfgFileHandle() < 0) {
		db_error("get file handle failed\n");
		return -1;
	}

	if(initPlayBackPreviewResource() < 0) {
		db_error("init PlayBackPreview resource failed\n");
		retval = -1;
		goto out;
	}

	if(initPlayBackResource() < 0) {
		db_error("init PlayBack resource failed\n");
		retval = -1;
		goto out;
	}

	if(initMenuListResource() < 0) {
		db_error("init Menu List resource failed\n");
		retval = -1;
		goto out;
	}
	/* ----------------------- */

	if(initOtherResource() < 0) {
		db_error("init other resource failed\n");
		retval = -1;
		goto out;	
	}
	db_msg("by hero nothing initStage2 end\n");
	notifyAll();
	
	db_msg("by hero nothing notifyAll end\n");
out:
	releaseCfgFileHandle();
	releaseCfgMenuHandle();
	db_msg("by hero nothing end\n");
	return retval;
}

int ResourceManager::initStatusBarResource(void)
{
	int err_code;
	int retval = 0;

	/*++++ Rect ++++*/
	configTable2 configTableRect[] = {
		{CFG_FILE_STB_BACK,		(void*)&rStatusBarData.backRect},
		{CFG_FILE_STB_REC,		(void*)&rStatusBarData.recRect},
		{CFG_FILE_STB_PIC,		(void*)&rStatusBarData.picRect},	
		{CFG_FILE_STB,			(void*)&rStatusBarData.STBRect },
		{CFG_FILE_STB_LABEL1,	(void*)&rStatusBarData.label1Rect },
	};

	/*++++ get color ++++*/
	configTable3 configTableColor[] = {
		{CFG_FILE_STB, FGC_WIDGET, (void*)&rStatusBarData.fgc},
		{CFG_FILE_STB, BGC_WIDGET, (void*)&rStatusBarData.bgc},
	};

	/*++++ get the current icon info ++++*/
	configTable2 configTableIcon[] = {
		{CFG_FILE_STB_BACK,		(void*)&rStatusBarData.backIcon},
		{CFG_FILE_STB_REC,		(void*)&rStatusBarData.recIcon},
		{CFG_FILE_STB_PIC,		(void*)&rStatusBarData.picIcon},
	};

	/*---- Rect ----*/
	for(unsigned int i = 0; i < TABLESIZE(configTableRect); i++) {
		err_code = getRectFromFileHandle(configTableRect[i].mainkey, (CDR_RECT*)configTableRect[i].addr );
		if(err_code != ETC_OK) {
			db_error("get %s rect failed: %s\n", configTableRect[i].mainkey,  pEtcError(err_code));
			retval = -1;
			goto out;
		}
	}

	/*---- get color ----*/
	for(unsigned int i = 0; i < TABLESIZE(configTableColor); i++) {
		err_code = getABGRColorFromFileHandle(configTableColor[i].mainkey, configTableColor[i].subkey, *(gal_pixel*)configTableColor[i].addr);
		if(err_code != ETC_OK) {
			db_error("get %s %s failed: %s\n", configTableColor[i].mainkey, configTableColor[i].subkey, pEtcError(err_code));
			retval = -1;
			goto out;
		}
		//		db_msg("%s %s color is 0x%lx\n", configTableColor[i].mainkey, configTableColor[i].subkey, 
		//				Pixel2DWORD(HDC_SCREEN, *(gal_pixel*)configTableColor[i].addr) );
	}

	/*---- get the current icon info ----*/
	unsigned int current;
	for(unsigned int i = 0; i < TABLESIZE(configTableIcon); i++) {
		err_code = fillCurrentIconInfoFromFileHandle(configTableIcon[i].mainkey, *(currentIcon_t*)configTableIcon[i].addr);
		if(err_code != ETC_OK) {
			db_error("get %s icon failed: %s\n", configTableIcon[i].mainkey, pEtcError(err_code) );
			retval = -1;
			goto out;
		}
		current = (*(currentIcon_t*)configTableIcon[i].addr).current;
		//		db_msg("%s current icon is %s\n", configTableIcon[i].mainkey, (*(currentIcon_t*)configTableIcon[i].addr).icon.[current].c_str() );
	}

	/************* end of init status bar resource ******************/

out:
	return retval;
}

void ResourceManager::setCurrentIconValue(enum ResourceID resID, int cur_val)
{
	currentIcon_t* pIconVector = NULL;
	size_t val = cur_val;
	switch(resID) {
	case ID_STATUSBAR_ICON_BACK:
		pIconVector = &rStatusBarData.backIcon;
		break;
	case ID_STATUSBAR_ICON_REC:
		pIconVector = &rStatusBarData.recIcon;
		break;
	case ID_STATUSBAR_ICON_PIC:
		pIconVector = &rStatusBarData.picIcon;
		break;		
	default:
		db_error("invalide resID: %d\n", resID);
		break;
	}

	if(pIconVector != NULL) {
		if(val < pIconVector->icon.size()) {
			pIconVector->current = val;
		}
	}
}

int ResourceManager::getCurrentIconValue(enum ResourceID resID)
{
	switch(resID) {
	case ID_STATUSBAR_ICON_BACK:
		return rStatusBarData.backIcon.current;
		break;
	case ID_STATUSBAR_ICON_REC:
		return rStatusBarData.recIcon.current;
		break;
	case ID_STATUSBAR_ICON_PIC:
		return rStatusBarData.picIcon.current;
		break;		
	default:
		db_error("invalide resID: %d\n", resID);
	}

	return 0;
}

int ResourceManager::initPlayBackPreviewResource(void)
{
	int err_code;
	int retval = 0;

	/*++++ Rect ++++*/
	configTable2 configTableRect[] = {
		/*  session name in the configFile			addr to store the value */
		{CFG_FILE_PLBPREVIEW_IMAGE,			(void*)&rPlayBackPreviewData.imageRect},
		{CFG_FILE_PLBPREVIEW_ICON,			(void*)&rPlayBackPreviewData.iconRect},
		{CFG_FILE_PLBPREVIEW_MENU,			(void*)&rPlayBackPreviewData.popupMenuRect},
		{CFG_FILE_PLBPREVIEW_MESSAGEBOX,	(void*)&rPlayBackPreviewData.messageBoxRect},
	};

	/*++++ item_width and item_height ++++*/
	configTable3 configTableIntValue[] = {
		{CFG_FILE_PLBPREVIEW_MENU,			"item_width",  (void*)&rPlayBackPreviewData.popupMenuItemWidth  },
		{CFG_FILE_PLBPREVIEW_MENU,			"item_height", (void*)&rPlayBackPreviewData.popupMenuItemHeight },
		{CFG_FILE_PLBPREVIEW_MESSAGEBOX,	"item_width",  (void*)&rPlayBackPreviewData.messageBoxItemWidth },
		{CFG_FILE_PLBPREVIEW_MESSAGEBOX,	"item_height", (void*)&rPlayBackPreviewData.messageBoxItemHeight}
	};

	/*++++ get color ++++*/
	configTable3 configTableColor[] = {
		{CFG_FILE_PLBPREVIEW_MENU, BGC_WIDGET,			(void*)&rPlayBackPreviewData.popupMenuBgcWidget },
		{CFG_FILE_PLBPREVIEW_MENU, BGC_ITEM_NORMAL,		(void*)&rPlayBackPreviewData.popupMenuBgcItemNormal },
		{CFG_FILE_PLBPREVIEW_MENU, BGC_ITEM_FOCUS,		(void*)&rPlayBackPreviewData.popUpMenuBgcItemFocus },
		{CFG_FILE_PLBPREVIEW_MENU, MAINC_THREED_BOX,	(void*)&rPlayBackPreviewData.popUpMenuMain3dBox },

		{CFG_FILE_PLBPREVIEW_MESSAGEBOX, BGC_WIDGET,			(void*)&rPlayBackPreviewData.messageBoxBgcWidget },
		{CFG_FILE_PLBPREVIEW_MESSAGEBOX, FGC_WIDGET,			(void*)&rPlayBackPreviewData.messageBoxFgcWidget },
		{CFG_FILE_PLBPREVIEW_MESSAGEBOX, BGC_ITEM_NORMAL,		(void*)&rPlayBackPreviewData.messageBoxBgcItemNormal },
		{CFG_FILE_PLBPREVIEW_MESSAGEBOX, BGC_ITEM_FOCUS,		(void*)&rPlayBackPreviewData.messageBoxBgcItemFous },
		{CFG_FILE_PLBPREVIEW_MESSAGEBOX, MAINC_THREED_BOX,		(void*)&rPlayBackPreviewData.messageBoxMain3dBox },
	};


	/*---- Rect ----*/
	for(unsigned int i = 0; i < TABLESIZE(configTableRect); i++) {
		err_code = getRectFromFileHandle(configTableRect[i].mainkey, (CDR_RECT*)configTableRect[i].addr);
		if(err_code != ETC_OK) {
			db_error("get %s rect failed: %s\n", configTableRect[i].mainkey, pEtcError(err_code));
			retval = -1;
			goto out;
		}
	}

	/* currentIcon_t */
	err_code = fillCurrentIconInfoFromFileHandle(CFG_FILE_PLBPREVIEW_ICON, rPlayBackPreviewData.icon);
	if(err_code != ETC_OK) {
		db_error("get playback preview current icon info failed: %s\n", pEtcError(err_code));
		retval = -1;
		goto out;
	}


	/*---- item_width and item_height ----*/
	for(unsigned int i = 0; i < TABLESIZE(configTableIntValue); i++) {
		err_code = getIntValueFromHandle(mCfgFileHandle, configTableIntValue[i].mainkey, configTableIntValue[i].subkey);
		if(err_code < 0) {
			db_error("get %s %s failed\n", configTableIntValue[i].mainkey, configTableIntValue[i].subkey);
			retval = -1;
			goto out;
		}
		*(unsigned int*)configTableIntValue[i].addr = err_code;
	}


	/*---- get color ----*/
	for(unsigned int i = 0; i < TABLESIZE(configTableColor); i++) {
		err_code = getABGRColorFromFileHandle(configTableColor[i].mainkey, configTableColor[i].subkey, *(gal_pixel*)configTableColor[i].addr);
		if(err_code != ETC_OK) {
			db_error("get %s %s failed: %s\n", configTableColor[i].mainkey, configTableColor[i].subkey, pEtcError(err_code));
			retval = -1;
			goto out;
		}
		//		db_msg("%s %s color is 0x%x\n", configTableColor[i].mainkey, configTableColor[i].subkey, 
		//				Pixel2DWORD(HDC_SCREEN, *(gal_pixel*)configTableColor[i].addr) );
	}

	/**** end of init the resource for PlayBackPreview window *****/

out:
	return retval;
}


int ResourceManager::initMainPreviewResource(void)
{
    int err_code;
    int retval = 0;

    /*++++ get the current icon rect info for mediafilepreview++++*/
    configTable2 configTableRect2[] = {
        {CFG_FILE_MP,           	   (void*)&rMainPreviewData.mainRect},
        {CFG_FILE_MP_RECORDER,          (void*)&rMainPreviewData.recorderRect},
        {CFG_FILE_MP_HDMI,             (void*)&rMainPreviewData.hdmiRect},
        {CFG_FILE_MP_WIFI,             (void*)&rMainPreviewData.wifiRect},
        {CFG_FILE_MP_ETHERNET,             (void*)&rMainPreviewData.ethernetRect},
        {CFG_FILE_MP_WEBCAM,         (void*)&rMainPreviewData.webcamRect},
        {CFG_FILE_OP_FUNC,         (void*)&rMainPreviewData.funcRect},
        {CFG_FILE_MP_PLAYBACK,         (void*)&rMainPreviewData.playbackRect},
        {CFG_FILE_MP_USBMODE,         (void*)&rMainPreviewData.usbmodeRect},
        {CFG_FILE_MP_RS485,         (void*)&rMainPreviewData.rs480Rect},
        {CFG_FILE_MP_TESTMODE,         (void*)&rMainPreviewData.testmodeRect},		
    };


   /*------MainPreview----------*/
   for(unsigned int i = 0; i < TABLESIZE(configTableRect2); i++) {
        err_code = getRectFromFileHandle(configTableRect2[i].mainkey, (CDR_RECT*)configTableRect2[i].addr);
        if(err_code != ETC_OK) {
            db_msg(" get %s rect failed: %s\n", configTableRect2[i].mainkey, pEtcError(err_code));
            retval = -1;
            goto out;
        }
   }
  		   
out:
    return retval;
}
int ResourceManager::initPlayBackResource(void)
{
	int err_code;
	int retval = 0;

	/* init the resource PlayBack window */
	err_code = getRectFromFileHandle(CFG_FILE_PLB_ICON, &rPlayBackData.iconRect);
	if(err_code != ETC_OK) {
		db_error("get playback icon rect failed: %s\n", pEtcError(err_code));
		retval = -1;
		goto out;
	}
	err_code = fillCurrentIconInfoFromFileHandle(CFG_FILE_PLB_ICON, rPlayBackData.icon);
	if(err_code != ETC_OK) {
		db_error("get playback current icon info failed: %s\n", pEtcError(err_code));
		retval = -1;
		goto out;
	}
	/* init the resource PlayBack window */
	err_code = getRectFromFileHandle(CFG_FILE_PLB_ICON1, &rPlayBackData.iconRect1);
	if(err_code != ETC_OK) {
		db_error("get playback icon rect failed: %s\n", pEtcError(err_code));
		retval = -1;
		goto out;
	}
	err_code = fillCurrentIconInfoFromFileHandle(CFG_FILE_PLB_ICON1, rPlayBackData.icon1);
	if(err_code != ETC_OK) {
		db_error("get playback current icon info failed: %s\n", pEtcError(err_code));
		retval = -1;
		goto out;
	}
	/* init the resource PlayBack window */
	err_code = getRectFromFileHandle(CFG_FILE_PLB_ICON2, &rPlayBackData.iconRect2);
	if(err_code != ETC_OK) {
		db_error("get playback icon rect failed: %s\n", pEtcError(err_code));
		retval = -1;
		goto out;
	}
	err_code = fillCurrentIconInfoFromFileHandle(CFG_FILE_PLB_ICON2, rPlayBackData.icon2);
	if(err_code != ETC_OK) {
		db_error("get playback current icon info failed: %s\n", pEtcError(err_code));
		retval = -1;
		goto out;
	}
	/* init the resource PlayBack window */
	err_code = getRectFromFileHandle(CFG_FILE_PLB_ICON3, &rPlayBackData.iconRect3);
	if(err_code != ETC_OK) {
		db_error("get playback icon rect failed: %s\n", pEtcError(err_code));
		retval = -1;
		goto out;
	}
	err_code = fillCurrentIconInfoFromFileHandle(CFG_FILE_PLB_ICON3, rPlayBackData.icon3);
	if(err_code != ETC_OK) {
		db_error("get playback current icon info failed: %s\n", pEtcError(err_code));
		retval = -1;
		goto out;
	}
	
	err_code = getRectFromFileHandle(CFG_FILE_PLB_PGB, &rPlayBackData.PGBRect);
	if(err_code != ETC_OK) {
		db_error("get playback ProgressBar rect failed: %s\n", pEtcError(err_code));
		retval = -1;
		goto out;
	}
	err_code = getABGRColorFromFileHandle(CFG_FILE_PLB_PGB, BGC_WIDGET, rPlayBackData.PGBBgcWidget);
	if(err_code != ETC_OK) {
		db_error("get playback bgc_widget failed: %s\n", pEtcError(err_code));
		retval = -1;
		goto out;
	}
	err_code = getABGRColorFromFileHandle(CFG_FILE_PLB_PGB, FGC_WIDGET, rPlayBackData.PGBFgcWidget);
	if(err_code != ETC_OK) {
		db_error("get playback fgc_widget failed: %s\n", pEtcError(err_code));
		retval = -1;
		goto out;
	}

out:
	return retval;
}

int ResourceManager::initMenuListResource(void)
{
	int err_code;
	int retval = 0;

	/*++++ get the menulist picture name ++++*/
	configTable3 configTableValue[] = {
		{CFG_FILE_ML, "choice",						(void*)&rMenuList.choicePic },
		{CFG_FILE_ML, "checkbox_checked_normal",	(void*)&rMenuList.checkedNormalPic },
		{CFG_FILE_ML, "checkbox_checked_press",		(void*)&rMenuList.checkedPressPic },
		{CFG_FILE_ML, "checkbox_unchecked_normal",	(void*)&rMenuList.uncheckedNormalPic },
		{CFG_FILE_ML, "checkbox_unchecked_press",	(void*)&rMenuList.uncheckedPressPic },
		{CFG_FILE_ML, "unfold_normal",	(void*)&rMenuList.unfoldNormalPic },
		{CFG_FILE_ML, "unfold_press",	(void*)&rMenuList.unfoldPressPic },
		{CFG_FILE_ML, "firme_ok",	(void*)&rMenuList.firme_ok },
	};

	/*++++ get color ++++*/
	configTable3 configTableColor[] = {
		{CFG_FILE_ML, FGC_WIDGET,		(void*)&rMenuList.fgcWidget },
		{CFG_FILE_ML, BGC_WIDGET,		(void*)&rMenuList.bgcWidget },
		{CFG_FILE_ML, LINEC_ITEM,		(void*)&rMenuList.linecItem },
		{CFG_FILE_ML, STRINGC_NORMAL,	(void*)&rMenuList.stringcNormal },
		{CFG_FILE_ML, STRINGC_SELECTED, (void*)&rMenuList.stringcSelected },
		{CFG_FILE_ML, VALUEC_NORMAL,	(void*)&rMenuList.valuecNormal },
		{CFG_FILE_ML, VALUEC_SELECTED,	(void*)&rMenuList.valuecSelected },
		{CFG_FILE_ML, SCROLLBARC,		(void*)&rMenuList.scrollbarc },
		{CFG_FILE_SUBMENU, LINEC_TITLE,		(void*)&rMenuList.subMenuLinecTitle },
		{CFG_FILE_ML_MB, FGC_WIDGET,	(void*)&rMenuList.messageBoxFgcWidget },
		{CFG_FILE_ML_MB, BGC_WIDGET,	(void*)&rMenuList.messageBoxBgcWidget },
		{CFG_FILE_ML_MB, LINEC_TITLE,	(void*)&rMenuList.messageBoxLinecTitle },
		{CFG_FILE_ML_MB, LINEC_ITEM,	(void*)&rMenuList.messageBoxLinecItem },
		{CFG_FILE_ML_DATE, BGC_WIDGET,	(void*)&rMenuList.dateBgcWidget },
		{CFG_FILE_ML_DATE, "fgc_label",	(void*)&rMenuList.dateFgc_label },
		{CFG_FILE_ML_DATE, "fgc_number",	(void*)&rMenuList.dateFgc_number },
		{CFG_FILE_ML_DATE, LINEC_TITLE,	(void*)&rMenuList.dateLinecTitle },
		{CFG_FILE_ML_DATE, "borderc_selected",	(void*)&rMenuList.dateBordercSelected },
		{CFG_FILE_ML_DATE, "borderc_normal",	(void*)&rMenuList.dateBordercNormal },

		{CFG_FILE_ML_INDICATOR, BGC_WIDGET,	(void*)&rMenuList.bgcIndicator },
		{CFG_FILE_ML_INDICATOR, SCROLLBARC,	(void*)&rMenuList.scrollbarcIndicator },
	};

	/*++++ Rect ++++*/
	configTable2 configTableRect[] = {
		{CFG_FILE_ML,		(void*)&rMenuList.menuListRect},
		{CFG_FILE_SUBMENU,	(void*)&rMenuList.subMenuRect},
		{CFG_FILE_ML_MB,	(void*)&rMenuList.messageBoxRect},
		{CFG_FILE_ML_DATE,	(void*)&rMenuList.dateRect},
		{CFG_FILE_ML_INDICATOR,	(void*)&rMenuList.indicatorRect},
	};

	/*++++ get the current icon info ++++*/
	configTable2 configTableIcon[] = {
		{CFG_FILE_ML_INDICATOR,	(void*)&rMenuList.indicatorIcon},

		// system menu
		{CFG_FILE_ML_PARK,	(void*)&rMenuList.parkIcon},
		{CFG_FILE_ML_GSENSOR,	(void*)&rMenuList.gsensorIcon},
		{CFG_FILE_ML_RECORD_SOUND,	(void*)&rMenuList.recordSoundIcon},
		{CFG_FILE_ML_VOICEVOL,	(void*)&rMenuList.voicevolIcon},
		{CFG_FILE_ML_KEYTONE,	(void*)&rMenuList.keyToneIcon},
		{CFG_FILE_ML_LIGHT_FREQ,	(void*)&rMenuList.lightFreqIcon},
		{CFG_FILE_ML_TFCARD_INFO,	(void*)&rMenuList.tfcardInfoIcon},
		{CFG_FILE_ML_DELAY_SHUTDOWN,	(void*)&rMenuList.delayShutdownIcon},
		{CFG_FILE_ML_FORMAT,	(void*)&rMenuList.formatIcon},
		{CFG_FILE_ML_LANG,		(void*)&rMenuList.languageIcon},
		{CFG_FILE_ML_DATE,		(void*)&rMenuList.dateIcon},
		{CFG_FILE_ML_SS,		(void*)&rMenuList.screenSwitchIcon},
		{CFG_FILE_ML_FIRMWARE,		(void*)&rMenuList.firmwareIcon},
		{CFG_FILE_ML_STANDBY_MODE,		(void*)&rMenuList.standbyModeIcon},
		{CFG_FILE_ML_FACTORYRESET,	(void*)&rMenuList.factoryResetIcon},
        // {CFG_FILE_ML_DRIVING_MODE,	(void*)&rMenuList.drivingModeIcon},
		{CFG_FILE_ML_WIFI,	(void*)&rMenuList.wifiIcon},
		{CFG_FILE_ML_IMAGE_ROTATION,	(void*)&rMenuList.imagerotationIcon},
		{CFG_FILE_ML_LED_LIGHTS,	(void*)&rMenuList.ledlightsIcon},
		{CFG_FILE_ML_ALIGNLINE,                (void*)&rMenuList.alignlineIcon},

		{CFG_FILE_ML_VIDEO_RESOLUTION,	(void*)&rMenuList.videoResolutionIcon},
		{CFG_FILE_ML_VIDEO_BITRATE,		(void*)&rMenuList.videoBitrateIcon},
		{CFG_FILE_ML_PHOTO_RESOLUTION,	(void*)&rMenuList.photoResolutionIcon},
		{CFG_FILE_ML_PHOTO_COMPRESSION_QUALITY,	(void*)&rMenuList.photoCompressionQualityIcon},
		{CFG_FILE_ML_PHOTO_TIMETAKEPHOTOS,	    (void*)&rMenuList.photoTimeTakePhotosIcon},
		{CFG_FILE_ML_PHOTO_AUTOMATICTAKEPHOTOS,	(void*)&rMenuList.photoAutomaticTakePhotosIcon},
		{CFG_FILE_ML_PHOTO_MOTIONTAKEPHOTOS,	(void*)&rMenuList.photoMotionTakePhotosIcon},
		{CFG_FILE_ML_VIDEO_DELAYP,		(void*)&rMenuList.delaypIcon},
		{CFG_FILE_ML_VIDEO_SLOWP,		(void*)&rMenuList.slowpIcon},
		{CFG_FILE_ML_VTL,	(void*)&rMenuList.timeLengthIcon},
		{CFG_FILE_ML_AWMD,	(void*)&rMenuList.moveDetectIcon},
		{CFG_FILE_ML_WB,	(void*)&rMenuList.whiteBalanceIcon},
		{CFG_FILE_ML_CONTRAST,	(void*)&rMenuList.contrastIcon},
		{CFG_FILE_ML_EXPOSURE,	(void*)&rMenuList.exposureIcon},
		{CFG_FILE_ML_POR,		(void*)&rMenuList.PORIcon},
		{CFG_FILE_ML_TWM,		(void*)&rMenuList.TWMIcon},
		{CFG_FILE_ML_PHOTO_WM,	(void*)&rMenuList.photoWMIcon},
		{CFG_FILE_ML_LICENSE_PLATE_WM,	(void*)&rMenuList.licensePlateWMIcon},
#ifdef APP_ADAS
		{CFG_FILE_ML_SMARTALGORITHM,                (void*)&rMenuList.smartalgorithm},
#endif
	};

	configTable3 configTableIntValue[] = {
		{CFG_FILE_ML_DATE,		"title_height",	(void*)&rMenuList.dateTileRectH  },
		{CFG_FILE_ML_DATE,		"hBorder",		(void*)&rMenuList.dateHBorder  },
		{CFG_FILE_ML_DATE,		"yearW",		(void*)&rMenuList.dateYearW  },
		{CFG_FILE_ML_DATE,		"dateLabelW",	(void*)&rMenuList.dateLabelW  },
		{CFG_FILE_ML_DATE,		"numberW",		(void*)&rMenuList.dateNumberW  },
		{CFG_FILE_ML_DATE,		"boxH",			(void*)&rMenuList.dateBoxH  }
	};

	/*---- get the menulist picture name ----*/
	char buf[50];
	for(unsigned int i = 0; i < TABLESIZE(configTableValue); i++) {
		err_code = getValueFromFileHandle(configTableValue[i].mainkey, configTableValue[i].subkey, buf, sizeof(buf));
		if(err_code != ETC_OK) {
			db_error("get %s %s pic failed: %s\n", configTableValue[i].mainkey, configTableValue[i].subkey, pEtcError(err_code));
			retval = -1;
			goto out;
		}
		*(std::string*)configTableValue[i].addr = buf;
		//		db_error("get %s %s pic is %s\n", configTableValue[i].mainkey, configTableValue[i].subkey, (*(string*)configTableValue[i].addr).c_str() );
	}

	/*---- get color ----*/
	for(unsigned int i = 0; i < TABLESIZE(configTableColor); i++) {
		err_code = getABGRColorFromFileHandle(configTableColor[i].mainkey, configTableColor[i].subkey, *(gal_pixel*)configTableColor[i].addr);
		if(err_code != ETC_OK) {
			db_error("get %s %s failed: %s\n", configTableColor[i].mainkey, configTableColor[i].subkey, pEtcError(err_code));
			retval = -1;
			goto out;
		}
		//		db_error("get %s %s color is 0x%x\n", configTableColor[i].mainkey, configTableColor[i].subkey, 
		//				Pixel2DWORD(HDC_SCREEN, *(gal_pixel*)configTableColor[i].addr) );
	}

	/*---- Rect ----*/
	for(unsigned int i = 0; i < TABLESIZE(configTableRect); i++) {
		err_code = getRectFromFileHandle(configTableRect[i].mainkey, (CDR_RECT*)configTableRect[i].addr);
		if(err_code != ETC_OK) {
			db_error("get %s rect failed: %s\n", configTableRect[i].mainkey, pEtcError(err_code));
			retval = -1;
			goto out;
		}
	}

	/*---- get the current icon info ----*/
	for(unsigned int i = 0; i < TABLESIZE(configTableIcon); i++) {
		err_code = fillCurrentIconInfoFromFileHandle(configTableIcon[i].mainkey, *(currentIcon_t*)configTableIcon[i].addr);
		if(err_code != ETC_OK) {
			db_error("get %s icon failed: %s\n", configTableIcon[i].mainkey, pEtcError(err_code) );
			retval = -1;
			goto out;
		}
	}

	/*  init inval */
	for(unsigned int i = 0; i < TABLESIZE(configTableIntValue); i++) {
		err_code = getIntValueFromHandle(mCfgFileHandle, configTableIntValue[i].mainkey, configTableIntValue[i].subkey);
		if(err_code < 0) {
			db_error("get %s %s failed\n", configTableIntValue[i].mainkey, configTableIntValue[i].subkey);
			retval = -1;
			goto out;
		}
		*(unsigned int*)configTableIntValue[i].addr = err_code;
	}

out:
	return retval;
}

void ResourceManager::dump()
{

	db_msg("menuDataLang: count %d,  current %d\n", rMenuList.menuDataLang.count, rMenuList.menuDataLang.current);
	db_msg("menuDataVideoResolution: count %d,	current %d\n",rMenuList.menuDataVideoResolution.count, rMenuList.menuDataVideoResolution.current);
	db_msg("menuDataPhotoResolution: count %d, current %d\n", rMenuList.menuDataPhotoResolution.count, rMenuList.menuDataPhotoResolution.current);
	db_msg("menuDataVTL: count %d, current %d\n", rMenuList.menuDataVTL.count, rMenuList.menuDataVTL.current);
	db_msg("menuDataWB: count %d, current %d\n", rMenuList.menuDataWB.count, rMenuList.menuDataWB.current);
	db_msg("menuDataContrast: count %d, current %d\n", rMenuList.menuDataContrast.count, rMenuList.menuDataContrast.current);
	db_msg("menuDataExposure: count %d, current %d\n", rMenuList.menuDataExposure.count, rMenuList.menuDataExposure.current);
	db_msg("menuDataSS: count %d, current %d\n", rMenuList.menuDataSS.count, rMenuList.menuDataSS.current);
	db_msg("menuDataPark: count %d, current %d\n", rMenuList.menuDataPark.count, rMenuList.menuDataPark.current);
	db_msg("recordSoundEnable %d\n", rMenuList.recordSoundEnable);
	db_msg("TWMEnable %d\n", rMenuList.TWMEnable);
	db_msg("AWMDEnable %d\n", rMenuList.AWMDEnable);
	db_msg("delayShutDown: count %d,  current %d\n", rMenuList.menuDataShutDown.count, rMenuList.menuDataShutDown.current);

}

int ResourceManager::initMenuResource(void)
{
	int retval = 0;

	configTable2 configTableMenuItem[] = {
		{CFG_MENU_LANG, (void*)&rMenuList.menuDataLang},
		{CFG_MENU_VIDEO_RESOLUTION,	(void*)&rMenuList.menuDataVideoResolution},
		{CFG_MENU_VIDEO_BITRATE,	(void*)&rMenuList.menuDataVideoBitrate},
		{CFG_MENU_PHOTO_RESOLUTION,	(void*)&rMenuList.menuDataPhotoResolution},
		{CFG_MENU_PHOTO_COMPRESSION_QUALITY,	(void*)&rMenuList.menuDataPhotoCompressionQuality},
		{CFG_MENU_PHOTO_TIMETAKEPHOTOS,	        (void*)&rMenuList.menuDataPhotoTimeTakePhotos},
		{CFG_MENU_PHOTO_AUTOMATICTAKEPHOTOS,	(void*)&rMenuList.menuDataPhotoAutomaticTakePhotos},
		{CFG_MENU_PHOTO_MOTIONTAKEPHOTOS,	    (void*)&rMenuList.menuDataPhotoMotionTakePhotos},
		{CFG_MENU_VIDEO_DELAYP,	(void*)&rMenuList.menuDataDelayP},
		{CFG_MENU_VIDEO_SLOWP,	(void*)&rMenuList.menuDataSlowP},
		{CFG_MENU_VTL,	(void*)&rMenuList.menuDataVTL},
		{CFG_MENU_WB,	(void*)&rMenuList.menuDataWB},
		{CFG_MENU_CONTRAST, (void*)&rMenuList.menuDataContrast},
		{CFG_MENU_EXPOSURE, (void*)&rMenuList.menuDataExposure},
		{CFG_MENU_SS,		(void*)&rMenuList.menuDataSS},
		{CFG_MENU_GSENSOR,		(void*)&rMenuList.menuDataGsensor},
		{CFG_MENU_VOICEVOL,		(void*)&rMenuList.menuDataVoiceVol},

		// menu system
		{CFG_MENU_PARK, (void*)&rMenuList.menuDataPark},
		{CFG_MENU_LIGHT_FREQ, (void*)&rMenuList.menuDataLightFreq},
		{CFG_MENU_DELAY_SHUTDOWN, (void*)&rMenuList.menuDataShutDown},
		{CFG_MENU_TF_CARD, (void*)&rMenuList.menuDataTFcard},
	};

	configTable2 configTableSwitch[] = {
		{CFG_MENU_DRIVING_MODE, (void*)&rMenuList.drivingModeEnable},
		{CFG_MENU_WIFI, (void*)&rMenuList.wifiEnable},
		{CFG_MENU_POR, (void*)&rMenuList.POREnable},
		{CFG_MENU_RECORD_SOUND,	(void*)&rMenuList.recordSoundEnable},
		{CFG_MENU_IMAGE_ROTATION,	(void*)&rMenuList.ImageRotation},
		{CFG_MENU_LED_LIGHTS,	(void*)&rMenuList.LEDLights},		
		{CFG_MENU_TWM,		(void*)&rMenuList.TWMEnable},
		{CFG_MENU_PHOTO_WM,	(void*)&rMenuList.photoWMEnable},
		{CFG_MENU_AWMD,		(void*)&rMenuList.AWMDEnable},
		{CFG_MENU_ALIGNLINE, 	(void*)&rMenuList.alignlineEnable},
#ifdef APP_ADAS
		{CFG_MENU_SMARTALGORITHM,   (void*)&rMenuList.smartAlgorithmEnable},
#endif

		{CFG_MENU_KEYTONE, (void*)&rMenuList.keyToneEnable},
		{CFG_MENU_STANDBY_MODE, (void*)&rMenuList.standbyModeEnable},
	};

	// --------------- init the menuItem count and current value ------------------
	int value;
	for(unsigned int i = 0; i < TABLESIZE(configTableMenuItem); i++) {
		value = getIntValueFromHandle(mCfgMenuHandle, configTableMenuItem[i].mainkey, "current");	
		if(value < 0) {
			db_error("get current %s failed\n", configTableMenuItem[i].mainkey);
			retval = -1;
			goto out;
		}
		((menuItem_t*)(configTableMenuItem[i].addr))->current = value;

		value = getIntValueFromHandle(mCfgMenuHandle, configTableMenuItem[i].mainkey, "count");	
		if(value < 0) {
			db_error("get count %s failed\n", configTableMenuItem[i].mainkey);
			retval = -1;
			goto out;
		}
#ifdef SUPPORT_4K
        if(i==1)
            ((menuItem_t*)(configTableMenuItem[i].addr))->count = value;//value;
#else
        if(i==1)
            ((menuItem_t*)(configTableMenuItem[i].addr))->count = 5;
#endif
        else
            ((menuItem_t*)(configTableMenuItem[i].addr))->count = value;
		//		db_msg("%s current is %d, count is %d\n", configTableMenuItem[i].mainkey, ((menuItem_t*)(configTableMenuItem[i].addr))->current, value);	
	}
	mCdrVolume = VolNum[rMenuList.menuDataVoiceVol.current].voice;
	// --------------- init the menuItem checkbox bool value ------------------
	for(unsigned int i = 0; i < TABLESIZE(configTableSwitch); i++) {
		value = getIntValueFromHandle(mCfgMenuHandle, "switch", configTableSwitch[i].mainkey);
		if(value < 0) {
			db_error("get switch %s failed\n", configTableSwitch[i].mainkey);
			return -1;
		}
		if(value == 0) {
			*((bool*)configTableSwitch[i].addr) = false;
		} else {
			*((bool*)configTableSwitch[i].addr) = true;
		}
		//db_error("switch %s is %d\n", configTableSwitch[i].mainkey, value);
	}
	if(rMenuList.keyToneEnable){
		setKeySoundVol(mCdrVolume);
	}else{
		switchKeyVoice(false);
	}

	value = getIntValueFromHandle(mCfgMenuHandle, "defaultlicense", "current");
	if(value < 0) {
		db_error("get switch defaultlicense %s failed\n");
		return -1;
	}
	if(value == 0) {
		menuDataLWM.LWMEnable = false;
	} else {
		menuDataLWM.LWMEnable = true;
	}
	if(GetValueFromEtcFile(configMenu, "defaultlicense", "position", menuDataLWM.lwaterMark ,16) < 0) {
		db_error("get defaultlicense failed\n");
		return -1;
		goto out;
	}else{
		db_msg(">>>>>>>>>>>>>>defaultlicense:position:%s\n",menuDataLWM.lwaterMark);
		db_msg(">>>>>>>>>>>>>>defaultlicense:enable?:%d\n",menuDataLWM.LWMEnable );
	}

out:
	return retval;
}

int ResourceManager::refreshLWMResource(void)
{
	int value;
	int retval = 0 ;
	getCfgMenuHandle();
		value = getIntValueFromHandle(mCfgMenuHandle, "defaultlicense", "current");
	if(value < 0) {
		db_error("get switch defaultlicense %s failed\n");
		return -1;
	}
	if(value == 0) {
		menuDataLWM.LWMEnable = false;
	} else {
		menuDataLWM.LWMEnable = true;
	}
	if(GetValueFromEtcFile(configMenu, "defaultlicense", "position", menuDataLWM.lwaterMark ,16) < 0) {
		db_error("get defaultlicense failed\n");
		return -1;
	}

	releaseCfgMenuHandle();
	return 1 ;
}


int ResourceManager::initOtherResource(void)
{
	int err_code;
	int retval = 0;
	configTable2 configTableRect[] = {
		{CFG_FILE_WARNING_MB,		(void*)&rWarningMB.rect},
		{CFG_FILE_CONNECT2PC,		(void*)&rConnectToPC.rect},
		{CFG_FILE_TIPLABEL,			(void*)&rTipLabel.rect}
	};

	configTable3 configTableColor[] = {
		{CFG_FILE_WARNING_MB, FGC_WIDGET,	(void*)&rWarningMB.fgcWidget },
		{CFG_FILE_WARNING_MB, BGC_WIDGET,	(void*)&rWarningMB.bgcWidget },
		{CFG_FILE_WARNING_MB, LINEC_TITLE,	(void*)&rWarningMB.linecTitle },
		{CFG_FILE_WARNING_MB, LINEC_ITEM,	(void*)&rWarningMB.linecItem },
		{CFG_FILE_CONNECT2PC, BGC_WIDGET,	(void*)&rConnectToPC.bgcWidget },
		{CFG_FILE_CONNECT2PC, BGC_ITEM_FOCUS,	(void*)&rConnectToPC.bgcItemFocus},
		{CFG_FILE_CONNECT2PC, BGC_ITEM_NORMAL,	(void*)&rConnectToPC.bgcItemNormal},
		{CFG_FILE_CONNECT2PC, MAINC_THREED_BOX,	(void*)&rConnectToPC.mainc_3dbox},
		{CFG_FILE_TIPLABEL,	  FGC_WIDGET,	(void*)&rTipLabel.fgcWidget},
		{CFG_FILE_TIPLABEL,	  BGC_WIDGET,	(void*)&rTipLabel.bgcWidget},
		{CFG_FILE_TIPLABEL,	  LINEC_TITLE,	(void*)&rTipLabel.linecTitle},
	};

	configTable3 configTableIntValue[] = {
		{CFG_FILE_CONNECT2PC, "item_width",		(void*)&rConnectToPC.itemWidth  },
		{CFG_FILE_CONNECT2PC, "item_height",	(void*)&rConnectToPC.itemHeight  },
		{CFG_FILE_TIPLABEL,	  "title_height",	(void*)&rTipLabel.titleHeight }
	};

	configTable3 configTableValue[] = {
		{CFG_FILE_OTHER_PIC, "poweroff",		(void*)&rPoweroffPic }
	};

	configTable3 configTableUSB[]= {
        {CFG_FILE_USB_CONNECT, "usb_computer",		(void*)&rUSBconnect.usb_computer },
		{CFG_FILE_USB_CONNECT, "usb_camera",		(void*)&rUSBconnect.usb_camera },
		{CFG_FILE_USB_CONNECT, "usb_working",		(void*)&rUSBconnect.usb_working }
	};
		
	configTable3 configTableFirmware[]= {
        {CFG_FIRMWARE_INFO, "product_type",			(void*)&rFirmwareconnect.product_type },
		{CFG_FIRMWARE_INFO, "software_version",		(void*)&rFirmwareconnect.software_version },
		{CFG_FIRMWARE_INFO, "updated",				(void*)&rFirmwareconnect.updated }
	};
	/*---- Rect ----*/
	for(unsigned int i = 0; i < TABLESIZE(configTableRect); i++) {
		err_code = getRectFromFileHandle(configTableRect[i].mainkey, (CDR_RECT*)configTableRect[i].addr);
		if(err_code != ETC_OK) {
			db_error("get %s rect failed: %s\n", configTableRect[i].mainkey, pEtcError(err_code));
			retval = -1;
			goto out;
		}
	}

	/*---- get color ----*/
	for(unsigned int i = 0; i < TABLESIZE(configTableColor); i++) {
		err_code = getABGRColorFromFileHandle(configTableColor[i].mainkey, configTableColor[i].subkey, *(gal_pixel*)configTableColor[i].addr);
		if(err_code != ETC_OK) {
			db_error("get %s %s failed: %s\n", configTableColor[i].mainkey, configTableColor[i].subkey, pEtcError(err_code));
			retval = -1;
			goto out;
		}
	}

	/*  init inval */
	for(unsigned int i = 0; i < TABLESIZE(configTableIntValue); i++) {
		err_code = getIntValueFromHandle(mCfgFileHandle, configTableIntValue[i].mainkey, configTableIntValue[i].subkey);
		if(err_code < 0) {
			db_error("get %s %s failed\n", configTableIntValue[i].mainkey, configTableIntValue[i].subkey);
			retval = -1;
			goto out;
		}
		*(unsigned int*)configTableIntValue[i].addr = err_code;
	}

	/*---- get the menulist picture name ----*/
	char buf[50];
	for(unsigned int i = 0; i < TABLESIZE(configTableValue); i++) {
		err_code = getValueFromFileHandle(configTableValue[i].mainkey, configTableValue[i].subkey, buf, sizeof(buf));
		if(err_code != ETC_OK) {
			db_error("get %s %s pic failed: %s\n", configTableValue[i].mainkey, configTableValue[i].subkey, pEtcError(err_code));
			retval = -1;
			goto out;
		}
		*(std::string*)configTableValue[i].addr = buf;
		//db_error("get %s %s pic is %s\n", configTableValue[i].mainkey, configTableValue[i].subkey, (*(string*)configTableValue[i].addr).c_str() );
	}
	
	char buf1[50];
	for(unsigned int i = 0; i < TABLESIZE(configTableUSB); i++) {
		err_code = getValueFromFileHandle(configTableUSB[i].mainkey, configTableUSB[i].subkey, buf1, sizeof(buf1));
		if(err_code != ETC_OK) {
			db_error("get %s %s pic failed: %s\n", configTableUSB[i].mainkey, configTableUSB[i].subkey, pEtcError(err_code));
			retval = -1;
			goto out;
		}
		*(std::string*)configTableUSB[i].addr = buf1;
	}

	char buf2[50];
	for(unsigned int i = 0; i < TABLESIZE(configTableFirmware); i++) {
		err_code = getValueFromFileHandle(configTableFirmware[i].mainkey, configTableFirmware[i].subkey, buf2, sizeof(buf2));
		if(err_code != ETC_OK) {
			db_error("get %s %s pic failed: %s\n", configTableFirmware[i].mainkey, configTableFirmware[i].subkey, pEtcError(err_code));
			retval = -1;
			goto out;
		}
		*(std::string*)configTableFirmware[i].addr = buf2;
	}	
out:
	return retval;
}

gal_pixel ResourceManager::getStatusBarColor(enum ResourceID resID, enum ColorType type)
{
	gal_pixel color = 0;
	if(resID == ID_STATUSBAR) {
		switch(type) {
		case COLOR_FGC:
			color = rStatusBarData.fgc;
			break;
		case COLOR_BGC:
			color = rStatusBarData.bgc;
			break;
		default:
			db_error("invalid ColorType: %d\n", type);
			break;
		}
	} else {
		db_error("invalid resID: %d\n", resID);
	}

	return color;
}

gal_pixel ResourceManager::getPlaybackPreviewColor(enum ResourceID resID, enum ColorType type)
{
	gal_pixel color = 0;
	if(resID == ID_PLAYBACKPREVIEW_MENU) {
		switch(type) {
		case COLOR_BGC:
			color = rPlayBackPreviewData.popupMenuBgcWidget;
			break;
		case COLOR_BGC_ITEMFOCUS:
			color = rPlayBackPreviewData.popUpMenuBgcItemFocus;
			break;
		case COLOR_BGC_ITEMNORMAL:
			color = rPlayBackPreviewData.popupMenuBgcItemNormal;
			break;
		case COLOR_MAIN3DBOX:
			color = rPlayBackPreviewData.popUpMenuMain3dBox;
			break;
		default:
			db_error("invalid ColorType: %d\n", type);
			break;
		}
	} else if(resID == ID_PLAYBACKPREVIEW_MB) {
		switch(type) {
		case COLOR_BGC:
			color = rPlayBackPreviewData.messageBoxBgcWidget;
			break;
		case COLOR_FGC:
			color = rPlayBackPreviewData.messageBoxFgcWidget;
			break;
		case COLOR_BGC_ITEMFOCUS:
			color = rPlayBackPreviewData.messageBoxBgcItemFous;
			break;
		case COLOR_BGC_ITEMNORMAL:
			color = rPlayBackPreviewData.messageBoxBgcItemNormal;
			break;
		case COLOR_MAIN3DBOX:
			color = rPlayBackPreviewData.messageBoxMain3dBox;
			break;
		default:
			db_error("invalid ColorType: %d\n", type);
			break;
		}
	} else {
		db_error("invalid resID: %d\n", resID);
	}

	return color;
}

gal_pixel ResourceManager::getPlaybackColor(enum ResourceID resID, enum ColorType type)
{
	gal_pixel color = 0;
	if(resID == ID_PLAYBACK_PGB) {
		switch(type) {
		case COLOR_BGC:
			color = rPlayBackData.PGBBgcWidget;
			break;
		case COLOR_FGC:
			color = rPlayBackData.PGBFgcWidget;
			break;
		default:
			db_error("invalid ColorType: %d\n", type);
			break;
		}
	} else {
		db_error("invalid resID: %d\n", resID);
	}

	return color;
}


gal_pixel ResourceManager::getMenuListColor(enum ResourceID resID, enum ColorType type)
{
	gal_pixel color = 0;
	if(resID == ID_MENU_LIST) {
		switch(type) {
		case COLOR_BGC:
			color = rMenuList.bgcWidget;
			break;
		case COLOR_FGC:
			color = rMenuList.fgcWidget;
			break;
		case COLOR_LINEC_ITEM:
			color = rMenuList.linecItem;
			break;
		case COLOR_STRINGC_NORMAL:
			color = rMenuList.stringcNormal;
			break;
		case COLOR_STRINGC_SELECTED:
			color = rMenuList.stringcSelected;
			break;
		case COLOR_VALUEC_NORMAL:
			color = rMenuList.valuecNormal;
			break;
		case COLOR_VALUEC_SELECTED:
			color = rMenuList.valuecSelected;
			break;
		case COLOR_SCROLLBARC:
			color = rMenuList.scrollbarc;
			break;
		default:
			db_error("invalid ColorType: %d\n", type);
			break;
		}
	} else if(resID == ID_MENU_LIST_MB) {
		switch(type) {
		case COLOR_BGC:
			color = rMenuList.messageBoxBgcWidget;
			break;
		case COLOR_FGC:
			color = rMenuList.messageBoxFgcWidget;
			break;
		case COLOR_LINEC_TITLE:
			color = rMenuList.messageBoxLinecTitle;
			break;
		case COLOR_LINEC_ITEM:
			color = rMenuList.messageBoxLinecItem;
			break;
		default:
			db_error("invalid ColorType: %d\n", type);
			break;
		}
	} else if(resID == ID_MENU_LIST_DATE) {
		switch(type) {
		case COLOR_BGC:
			color = rMenuList.dateBgcWidget;
			break;
		case COLOR_FGC_LABEL:
			color = rMenuList.dateFgc_label;
			break;
		case COLOR_FGC_NUMBER:
			color = rMenuList.dateFgc_number;
			break;
		case COLOR_LINEC_TITLE:
			color = rMenuList.dateLinecTitle;
			break;
		case COLOR_BORDERC_NORMAL:
			color = rMenuList.dateBordercNormal;
			break;
		case COLOR_BORDERC_SELECTED:
			color = rMenuList.dateBordercSelected;
			break;
		default:
			db_error("invalid ColorType: %d\n", type);
			break;
		}
	} else if(resID == ID_MENU_LIST_INDICATOR) {
		switch(type) {
		case COLOR_BGC:
			color = rMenuList.bgcIndicator;
			break;
		case COLOR_SCROLLBARC:
			color = rMenuList.scrollbarcIndicator;
		default:
			db_error("invalid ColorType: %d\n", type);
			break;
		}
	} else {
		db_error("invalid resID: %d\n", resID);
	}

	return color;
}

gal_pixel ResourceManager::getSubMenuColor(enum ResourceID resID, enum ColorType type)
{
	gal_pixel color = 0;
	if(resID == ID_SUBMENU) {
		switch(type) {
		case COLOR_BGC:
			color = rMenuList.subMenuBgcWidget;
			break;
		case COLOR_FGC:
			color = rMenuList.subMenuFgcWidget;
			break;
		case COLOR_LINEC_TITLE:
			color = rMenuList.subMenuLinecTitle;
			break;
		default:
			db_error("invalid ColorType: %d\n", type);
			break;
		}
	} else {
		db_error("invalid resID: %d\n", resID);
	}

	return color;
}

gal_pixel ResourceManager::getWarningMBColor(enum ResourceID resID, enum ColorType type)
{
	gal_pixel color = 0;
	if(resID == ID_WARNNING_MB) {
		switch(type) {
		case COLOR_BGC:
			color = rWarningMB.bgcWidget;
			break;
		case COLOR_FGC:
			color = rWarningMB.fgcWidget;
			break;
		case COLOR_LINEC_TITLE:
			color = rWarningMB.linecTitle;
			break;
		case COLOR_LINEC_ITEM:
			color = rWarningMB.linecItem;
			break;
		default:
			db_error("invalid ColorType: %d\n", type);
			break;
		}
	} else {
		db_error("invalid resID: %d\n", resID);
	}

	return color;
}

gal_pixel ResourceManager::getConnect2PCColor(enum ResourceID resID, enum ColorType type)
{
	gal_pixel color = 0;
	if(resID == ID_CONNECT2PC) {
		switch(type) {
		case COLOR_BGC:
			color = rConnectToPC.bgcWidget;
			break;
		case COLOR_BGC_ITEMFOCUS:
			color = rConnectToPC.bgcItemFocus;
			break;
		case COLOR_BGC_ITEMNORMAL:
			color = rConnectToPC.bgcItemNormal;
			break;
		case COLOR_MAIN3DBOX:
			color = rConnectToPC.mainc_3dbox;
			break;
		default:
			db_error("invalid ColorType: %d\n", type);
			break;
		}
	} else {
		db_error("invalid resID: %d\n", resID);
	}

	return color;
}

gal_pixel ResourceManager::getResColor(enum ResourceID resID, enum ColorType type)
{
	gal_pixel color = 0;

	switch(resID) {
	case ID_STATUSBAR:
		color = getStatusBarColor(resID, type);
		break;
	case ID_PLAYBACKPREVIEW_MENU:
	case ID_PLAYBACKPREVIEW_MB:
		color = getPlaybackPreviewColor(resID, type);
		break;
	case ID_PLAYBACK_PGB:
		color = getPlaybackColor(resID, type);
		break;
	case ID_MENU_LIST:
	case ID_MENU_LIST_MB:
	case ID_MENU_LIST_DATE:
	case ID_MENU_LIST_INDICATOR:
		color = getMenuListColor(resID, type);
		break;
	case ID_SUBMENU:
		color = getSubMenuColor(resID, type);
		break;
	case ID_WARNNING_MB:
		color = getWarningMBColor(resID, type);
		break;
	case ID_CONNECT2PC:
		color = getConnect2PCColor(resID, type);
		break;
	case ID_TIPLABEL:
		if(type == COLOR_BGC)
			color = rTipLabel.bgcWidget;
		else if(type == COLOR_FGC)
			color = rTipLabel.fgcWidget;
		else if(type == COLOR_LINEC_TITLE)
			color = rTipLabel.linecTitle;
		break;
	default:
		db_error("invalid resIndex for the color\n");
		break;
	}

	return color;
}

int ResourceManager::getResRect(enum ResourceID resID, CDR_RECT &rect)
{
	switch(resID) {
	case ID_SCREEN:
	case ID_POWEROFF:
		rect = rScreenRect;
		break;
	case ID_STATUSBAR:
		rect = rStatusBarData.STBRect;
		break;
	case ID_STATUSBAR_ICON_BACK:
		rect = rStatusBarData.backRect;
		break;
	case ID_STATUSBAR_ICON_REC:
		rect = rStatusBarData.recRect;
		break;
	case ID_STATUSBAR_ICON_PIC:
		rect = rStatusBarData.picRect;
		break;		
	case ID_STATUSBAR_LABEL1:
		rect = rStatusBarData.label1Rect;
		break;
	case ID_PLAYBACKPREVIEW_IMAGE:
		rect = rPlayBackPreviewData.imageRect;
		break;
	case ID_PLAYBACKPREVIEW_ICON:
		rect = rPlayBackPreviewData.iconRect;
		break;
	case ID_PLAYBACKPREVIEW_MENU:
		rect = rPlayBackPreviewData.popupMenuRect;
		break;
	case ID_PLAYBACKPREVIEW_MB:
		rect = rPlayBackPreviewData.messageBoxRect;
		break;
	case ID_PLAYBACK_ICON:
		rect = rPlayBackData.iconRect;
		break;
	case ID_PLAYBACK_ICON1:
		rect = rPlayBackData.iconRect1;
		break;
	case ID_PLAYBACK_ICON2:
		rect = rPlayBackData.iconRect2;
		break;
	case ID_PLAYBACK_ICON3:
		rect = rPlayBackData.iconRect3;
		break;
	case ID_PLAYBACK_PGB:
		rect = rPlayBackData.PGBRect;
		break;
	case ID_MENU_LIST:
		rect  = rMenuList.menuListRect;
		break;
	case ID_SUBMENU:
		rect = rMenuList.subMenuRect;
		break;
	case ID_MENU_LIST_MB:
		//rect = rMenuList.messageBoxRect;
		{
			int screenW,screenH;
			getScreenInfo(&screenW, &screenH);
			rect.x = screenW/6;
			rect.y = screenH/6;
			rect.w = 4*rect.x;
			rect.h = 4*rect.y;
		}
		break;
	case ID_WARNNING_MB:
		rect = rWarningMB.rect;
		break;
	case ID_MENU_LIST_DATE:
		rect = rMenuList.dateRect;
		break;
	case ID_MENU_LIST_INDICATOR:
		rect = rMenuList.indicatorRect;
		break;
	case ID_CONNECT2PC:
		rect = rConnectToPC.rect;
		break;
	case ID_TIPLABEL:
		rect = rTipLabel.rect;
		break;
   	case ID_MP_BJ_WINDOW:
        rect = rMainPreviewData.mainRect;
        break;		
    case ID_MP_RECORDER_WINDOW:
        rect = rMainPreviewData.recorderRect;
        break;		
    case ID_MP_HDMI_WINDOW:
        rect = rMainPreviewData.hdmiRect;
        break;
    case ID_MP_WIFI_WINDOW:
        rect = rMainPreviewData.wifiRect;
        break;
    case ID_MP_ETHERNET_WINDOW:
        rect = rMainPreviewData.ethernetRect;
        break;
    case ID_MP_WEBCAM_WINDOW:
        rect = rMainPreviewData.webcamRect;
        break;
    case ID_OP_FUNC_WINDOW:
        rect = rMainPreviewData.funcRect;
        break;
    case ID_MP_PLAYBACK_WINDOW:
        rect = rMainPreviewData.playbackRect;
        break;
    case ID_MP_USBMODE_WINDOW:
        rect = rMainPreviewData.usbmodeRect;
        break;
    case ID_MP_RS485_WINDOW:
        rect = rMainPreviewData.rs480Rect;
        break;	
    case ID_MP_TESTMODE_WINDOW:
        rect = rMainPreviewData.testmodeRect;
        break;				
	default:
		db_error("invalid resID index: %d\n", resID);
		return -1;
	}

	return 0;
}

int ResourceManager::getResBmp(enum ResourceID resID, enum BmpType type, BITMAP &bmp)
{
	int err_code;
	std::string file;

	if(resID > ID_IMAGE_ICON_START && resID < ID_IMAGE_ICON_END) {
		if(getCurrentIconFileName(resID, file) < 0) {
			db_error("get current icon pic bmp failed\n");	
			return -1;
		}
	} else if(resID >= ID_MENU_LIST_ITEM_START && resID <= ID_MENU_LIST_ITEM_END) {
        /*
		err_code = getMLPICFileName(resID, type, file);	
		if(err_code < 0) {
			db_error("get menu_list pic bmp failed\n");	
			return -1;
		}
        */
        return 0;
	} else {
		switch(resID) {		
		case ID_PLAYBACK_ICON1:
			if(getCurrentIconFileName(resID, file) < 0) {
			db_error("get current icon pic bmp failed\n");	
			return -1;
			}
			break;
		case ID_PLAYBACK_ICON2:
			if(getCurrentIconFileName(resID, file) < 0) {
			db_error("get current icon pic bmp failed\n");	
			return -1;
		}
			break;
		case ID_PLAYBACK_ICON3:
			if(getCurrentIconFileName(resID, file) < 0) {
			db_error("get current icon pic bmp failed\n");	
			return -1;
		}
			break;
			
		case ID_MENU_LIST_UNFOLD_PIC:
			if(type == BMPTYPE_SELECTED)
				file = rMenuList.unfoldPressPic;
			else
				file = rMenuList.unfoldNormalPic;
			break;
		case ID_MENU_LIST_CHECKBOX_PIC:
			if(type == BMPTYPE_SELECTED)
				file = rMenuList.checkedPressPic;
			else
				file = rMenuList.checkedNormalPic;
			break;
		case ID_MENU_LIST_UNCHECKBOX_PIC:
			if(type == BMPTYPE_SELECTED)
				file = rMenuList.uncheckedPressPic;
			else
				file = rMenuList.uncheckedNormalPic;
			break;
		case ID_CONNECT2PC:
		   err_code = getIconUsbConnect((UsbType)type,file); 
			if(err_code < 0) {
				db_error("get menu_list pic bmp failed\n"); 
				return -1;
			}
		    break;
		case ID_SUBMENU_CHOICE_PIC:
			file = rMenuList.choicePic;
			break;
		case ID_POWEROFF:
			file = rPoweroffPic;
			break;
		case ID_MENU_LIST_FIRMWARE_OK:
			file = rMenuList.firme_ok;
			break;

		default:
			db_error("invalid resID: %d\n", resID);
			return -1;
		}
	}

	db_msg("resID: %d, bmp file is %s\n", resID, file.c_str());
	err_code = LoadBitmapFromFile(HDC_SCREEN, &bmp, file.c_str());	
	if(err_code != ERR_BMP_OK) {
		db_error("load %s bitmap failed\n", file.c_str());
	}

	return 0;
}

int ResourceManager::getResBmp2(enum ResourceID resID, size_t index, BITMAP &bmp)
{
	int err_code;
	currentIcon_t* pIconVector = NULL;
	std::string file;

	switch(resID) {
	case ID_MENU_LIST_INDICATOR:
		pIconVector = &rMenuList.indicatorIcon;
		break;
	default:
		db_error("invalid resID: %d\n", resID);
		return -1;
		break;
	}

	if(pIconVector != NULL) {
		if(index >= pIconVector->icon.size()) {
			db_error("invalid current value %d\n", pIconVector->current);
			return -1;
		}
		file = pIconVector->icon[index];
	}

	db_msg("resID: %d, bmp file is %s\n", resID, file.c_str());	

	err_code = LoadBitmapFromFile(HDC_SCREEN, &bmp, file.c_str());	
	if(err_code != ERR_BMP_OK) {
		db_error("load %s bitmap failed\n", file.c_str());
	}

	return 0;
}

int ResourceManager::getResBkgroundBmp(size_t index, BITMAP &bmp)
{
	std::string file;
	int err_code;

	if(index < rBkgroundPic.icon.size()) {
		file = rBkgroundPic.icon[index];
		err_code = LoadBitmapFromFile(HDC_SCREEN, &bmp, file.c_str());	
		if(err_code != ERR_BMP_OK) {
			db_error("load %s bitmap failed\n", file.c_str());
		}
	}

	return 0;
}

int ResourceManager::getResBmpSubMenuCheckbox(enum ResourceID resID, bool isHilight, BITMAP &bmp)
{

	bool isChecked = false;
	switch(resID) {
	case ID_MENU_LIST_DRIVING_MODE:
		isChecked = rMenuList.drivingModeEnable;
		break;
	case ID_MENU_LIST_WIFI:
		isChecked = rMenuList.wifiEnable;
		break;
#ifdef APP_ADAS
	case ID_MENU_LIST_SMARTALGORITHM:
		isChecked = rMenuList.smartAlgorithmEnable;
		break;
#endif
	case ID_MENU_LIST_ALIGNLINE:
		isChecked = rMenuList.alignlineEnable;
		break;
	case ID_MENU_LIST_POR:
		isChecked = rMenuList.POREnable;
		break;
	case ID_MENU_LIST_TWM:
		isChecked = rMenuList.TWMEnable;
		break;
	case ID_MENU_LIST_PHOTO_WM:
		isChecked = rMenuList.photoWMEnable;
		break;
	case ID_MENU_LIST_AWMD:
		isChecked = rMenuList.AWMDEnable;
		break;
	case ID_MENU_LIST_RECORD_SOUND:
		isChecked = rMenuList.recordSoundEnable;
		break;
	case ID_MENU_LIST_IMAGE_ROTATION:
		isChecked = rMenuList.ImageRotation;
		break;
	case ID_MENU_LIST_LED_LIGHTS:
		isChecked = rMenuList.LEDLights;
		break;
	case ID_MENU_LIST_QUIT:
		break;	
	case ID_MENU_LIST_KEYTONE:
		isChecked = rMenuList.keyToneEnable;
		break;
	case ID_MENU_LIST_STANDBY_MODE:
		isChecked = rMenuList.standbyModeEnable;
		break;
	case ID_MENU_LIST_FIRMWARE:
		if(!FILE_EXSIST(MOUNT_PATH"full_img.fex")){
			db_msg("0:/full_img.fex not exist\n");
			rMenuList.menuDataFamireEnable = false;
			isChecked = false;
		//mPM->watchdogRun();
		}else{
			isChecked = true;
			rMenuList.menuDataFamireEnable = true;
		}
		break;
	default:
		db_error("invalid resID: %d\n", resID);
		return -1;
	}

	if(isChecked) {
		if(isHilight)
		{
			if(resID == ID_MENU_LIST_FIRMWARE)
			{
				return getResBmp(ID_MENU_LIST_FIRMWARE_OK, BMPTYPE_SELECTED, bmp);
			}
			return getResBmp(ID_MENU_LIST_CHECKBOX_PIC, BMPTYPE_SELECTED, bmp);
		}	
		else
		{
			if(resID == ID_MENU_LIST_FIRMWARE)
			{
				return getResBmp(ID_MENU_LIST_FIRMWARE_OK, BMPTYPE_UNSELECTED, bmp);
			}
			return getResBmp(ID_MENU_LIST_CHECKBOX_PIC, BMPTYPE_UNSELECTED, bmp);
		}
	} else {
		if(isHilight)
			return getResBmp(ID_MENU_LIST_UNCHECKBOX_PIC, BMPTYPE_SELECTED, bmp);
		else	
			return getResBmp(ID_MENU_LIST_UNCHECKBOX_PIC, BMPTYPE_UNSELECTED, bmp);
	}

}

int ResourceManager::getResIntValue(enum ResourceID resID, enum IntValueType type)
{
	unsigned int intValue = -1;

	if(resID >= ID_MENU_LIST_ITEM_START && resID <= ID_MENU_LIST_ITEM_END) {
		if(type == INTVAL_SUBMENU_INDEX) {
			return getSubMenuCurrentIndex(resID);//------------
		} else if(type == INTVAL_SUBMENU_COUNT) {
			return getSubMenuCounts(resID);
		} else {
			return getSubMenuIntAttr(resID, type);
		}
	} else {
		switch(resID) {
		case ID_PLAYBACKPREVIEW_MENU:
			if(type == INTVAL_ITEMWIDTH)
				intValue = rPlayBackPreviewData.popupMenuItemWidth;
			else if(type == INTVAL_ITEMHEIGHT) 
				intValue = rPlayBackPreviewData.popupMenuItemHeight;
			else
				db_error("invalid IntValueType: %d\n", type);
			break;
		case ID_PLAYBACKPREVIEW_MB:
			if(type == INTVAL_ITEMWIDTH)
				intValue = rPlayBackPreviewData.messageBoxItemWidth;
			else if(type == INTVAL_ITEMHEIGHT)
				intValue = rPlayBackPreviewData.messageBoxItemHeight;
			else
				db_error("invalid IntValueType: %d\n", type);
			break;
		case ID_CONNECT2PC:
			if(type == INTVAL_ITEMWIDTH)
				intValue = rConnectToPC.itemWidth;
			else if(type == INTVAL_ITEMHEIGHT)
				intValue = rConnectToPC.itemHeight;
			else
				db_error("invalid IntValueType: %d\n", type);
			break;
		case ID_TIPLABEL:
			if(type == INTVAL_TITLEHEIGHT)
				intValue = rTipLabel.titleHeight;
			else
				db_error("invalid IntValueType: %d\n", type);
			break;
		default:
			db_error("invalid resID: %d\n", resID);
			break;
		}
	}

	return intValue;
}

int ResourceManager::setResIntValue(enum ResourceID resID, enum IntValueType type, int value)
{
	if(resID >= ID_MENU_LIST_ITEM_START && resID <= ID_MENU_LIST_ITEM_END) {
		if(type == INTVAL_SUBMENU_INDEX)
			return setSubMenuCurrentIndex(resID, value);
		else {
			db_error("invalide IntValueType: %d\n", type);
		}
	} else {
		db_error("invalid resID %d\n", resID);
	}

	return -1;
}

bool ResourceManager::getResBoolValue(ResourceID resID)
{
	bool value = false;
	return value;
}

int ResourceManager::notifyWaterMark()
{
	int wmFlag = 0;
	if (rMenuList.TWMEnable) {
		wmFlag |= WATERMARK_TWM;
	}
	if (menuDataLWM.LWMEnable) {
		wmFlag |= WATERMARK_LWM;
	}
	SendMessage(mHwnd[WINDOWID_RECORDPREVIEW], MSG_WATER_MARK, (WPARAM)wmFlag, 0);
	return 0;
}

int ResourceManager::setResBoolValue(enum ResourceID resID, bool value)
{
	return 0;
}

const char* ResourceManager::getResMenuItemString(enum ResourceID resID)
{
	const char* ptr = NULL;

	return ptr;
}

const char* ResourceManager::getResFirmwareString(enum ResourceID resID,int index)
{	
	const char* ptr = NULL;
	return ptr;	
}

const char* ResourceManager::getResSubMenuItemString(enum ResourceID resID, int index)
{

	return NULL;	
}

const char* ResourceManager::getResSubMenuTitle(enum ResourceID resID)
{
	const char* ptr = NULL;

	return ptr;
}

LANGUAGE ResourceManager::getLanguage(void)
{

	return lang;
}

PLOGFONT ResourceManager::getLogFont(void)
{

	return mLogFont;
}

int ResourceManager::getIconUsbConnect(enum UsbType type, std::string &file)
{
	switch(type) {
	case ID_USB_COMPUTER:
		file = rUSBconnect.usb_computer;
		break;
	case ID_USB_CAMERA:
		file = rUSBconnect.usb_camera;
		break;
	case ID_USB_WORKING:
		file = rUSBconnect.usb_working;
		break;
	
	default:
		
		return -1;
	}
	return 0;
}
int ResourceManager::getCurrentIconFileName(enum ResourceID resID, std::string &file)
{
	int current;
	currentIcon_t* pIconVector = NULL;

	switch(resID) {
	case ID_STATUSBAR_ICON_BACK:	
		pIconVector = &rStatusBarData.backIcon;
		break;
	case ID_STATUSBAR_ICON_REC:	
		pIconVector = &rStatusBarData.recIcon;
		break;
	case ID_STATUSBAR_ICON_PIC:	
		pIconVector = &rStatusBarData.picIcon;
		break;	
	default:
		db_error("invalid resID %d\n", resID);
		return -1;
	}

	if(pIconVector != NULL) {
		if(pIconVector->current >= pIconVector->icon.size()) {
			db_error("invalid current value %d\n", pIconVector->current);
			return -1;
		}
		current = pIconVector->current;
		file = pIconVector->icon[current];
	}

	return 0;
}

int ResourceManager::getMLPICFileName(enum ResourceID resID, enum BmpType type, std::string &file)
{

	return 0;
}

int ResourceManager::getSubMenuCurrentIndex(enum ResourceID resID)
{
	
	return 0;
}

int ResourceManager::getSubMenuCounts(ResourceID resID)
{

	return 0;
}

int ResourceManager::getSubMenuIntAttr(enum ResourceID resID, enum IntValueType type)
{
	int intValue = -1;

	return intValue;
}

int ResourceManager::setSubMenuCurrentIndex(enum ResourceID resID, int newSel)
{
	return 0;
}

void ResourceManager::updateLangandFont(int langValue)
{
	LANGUAGE newLang;	

	if(langValue < LANG_ERR) {
		newLang = (LANGUAGE)langValue;
	} else {
		db_error("invalide langValue %d\n", langValue);
		return;
	}

	if(newLang == lang)
		return;

	if(newLang == LANG_CN || newLang == LANG_TW)
		mLogFont = mLogFont_ch;
	else
		mLogFont = mLogFont_other;

	/* init lang labels */
	if(initLabel(newLang) == 0) {
		lang = newLang;
		db_msg("new lang is %d\n", lang);
		SendMessage(mHwnd[WINDOWID_MAIN], MSG_RM_LANG_CHANGED, (WPARAM)&lang, 0);
	}
}

void ResourceManager::syncIconCurrent()
{
}

int ResourceManager::loadBmpFromConfig(const char* pSection, const char* pKey, PBITMAP bmp)
{

	int retval;
	char buf[CDR_FILE_LENGHT] = {0};

	if((retval = GetValueFromEtcFile(configFile, pSection, pKey, buf, sizeof(buf))) != ETC_OK) {
		db_error("get %s->%s failed: %s\n", pSection, pKey, pEtcError(retval));
		return -1;
	}

	if( (retval = LoadBitmapFromFile(HDC_SCREEN, bmp, buf)) != ERR_BMP_OK) {
		db_error("load %s failed: %s\n", buf, pBmpError(retval));
		return -1;
	}

	return 0;
}

void ResourceManager::unloadBitMap(BITMAP &bmp)
{
	if(bmp.bmBits != NULL) {
		UnloadBitmap(&bmp);
	}
}

static char err_buf[100];
char* ResourceManager::pEtcError(int err_code)
{
	bzero(err_buf, sizeof(err_buf));
	switch(err_code) 
	{
	case ETC_FILENOTFOUND:
		sprintf(err_buf, "The etc file not found");
		break;
	case ETC_SECTIONNOTFOUND:
		sprintf(err_buf, "The section not found");
		break;
	case ETC_KEYNOTFOUND:
		sprintf(err_buf, "The key not found");
		break;
	case ETC_TMPFILEFAILED:
		sprintf(err_buf, "Create tmpfile failed");
		break;
	case ETC_FILEIOFAILED:
		sprintf(err_buf, "IO operation failed to etc file");
		break;
	case ETC_INTCONV:
		sprintf(err_buf, "Convert the value string to an integer failed");
		break;
	case ETC_INVALIDOBJ:
		sprintf(err_buf, "Invalid object to etc file");
		break;
	case ETC_READONLYOBJ:
		sprintf(err_buf, "Read only to etc file");
		break;
	default:
		sprintf(err_buf, "Unkown error code %d", err_code);
		break;
	}

	return err_buf;
}

char* ResourceManager::pBmpError(int err_code)
{
	bzero(err_buf, sizeof(err_buf));
	switch(err_code)
	{
	case ERR_BMP_IMAGE_TYPE:
		sprintf(err_buf, "Not a valid bitmap");
		break;
	case ERR_BMP_UNKNOWN_TYPE:
		sprintf(err_buf, "Not recongnized bitmap type");
		break;
	case ERR_BMP_CANT_READ:
		sprintf(err_buf, "Read error");
		break;
	case ERR_BMP_CANT_SAVE:
		sprintf(err_buf, "Save error");
		break;
	case ERR_BMP_NOT_SUPPORTED:
		sprintf(err_buf, "Not supported bitmap type");
		break;
	case ERR_BMP_MEM:
		sprintf(err_buf, "Memory allocation error");
		break;
	case ERR_BMP_LOAD:
		sprintf(err_buf, "Loading error");
		break;
	case ERR_BMP_FILEIO:
		sprintf(err_buf, "I/O failed");
		break;
	case ERR_BMP_OTHER:
		sprintf(err_buf, "Other error");
		break;
	case ERR_BMP_ERROR_SOURCE:
		sprintf(err_buf, "A error data source");
		break;
	default:
		sprintf(err_buf, "Unkown error code %d", err_code);
		break;
	}

	return err_buf;
}



int ResourceManager::getCfgFileHandle(void)
{
	db_msg("by hero getCfgFileHandle start\n");
	if (mCfgFileHandle != 0) {
		return 0;
	}
	mCfgFileHandle = LoadEtcFile(configFile);
	//db_msg("----------the mCfgFileHandle is %d-------------\n");
	if(mCfgFileHandle == 0) {
		db_error("getCfgFileHandle failed\n");
		return -1;
	}
	return 0;
}

int ResourceManager::getCfgMenuHandle(void)
{
	if (mCfgMenuHandle != 0) {
		return 0;
	}
	mCfgMenuHandle = LoadEtcFile(configMenu);
	if(mCfgMenuHandle == 0) {
		db_error("getCfgMenuHandle failed\n");
		return -1;
	}

	return 0;
}

void ResourceManager::releaseCfgFileHandle(void)
{
	if(mCfgFileHandle == 0) {
		return;
	}
	UnloadEtcFile(mCfgFileHandle);
	mCfgFileHandle = 0;
}

void ResourceManager::releaseCfgMenuHandle(void)
{
	if(mCfgMenuHandle == 0) {
		return;
	}
	UnloadEtcFile(mCfgMenuHandle);
	mCfgMenuHandle = 0;
}

/*
 * On success return the int value
 * On fail return -1
 * */
int ResourceManager::getIntValueFromHandle(GHANDLE cfgHandle, const char* mainkey, const char* subkey)
{
	int retval = 0;
	int err_code;
	if(mainkey == NULL || subkey == NULL) {
		db_error("NULL pointer\n");	
		return -1;
	}

	if(cfgHandle == 0) {
		db_error("cfgHandle not initialized\n");
		return -1;
	}

	if((err_code = GetIntValueFromEtc(cfgHandle, mainkey, subkey, &retval)) != ETC_OK) {
		db_error("get %s->%s failed:%s\n", mainkey, subkey, pEtcError(err_code));
		return -1;
	}

	return retval;	
}

int ResourceManager::getValueFromFileHandle(const char*mainkey, const char* subkey, char* value, unsigned int len)
{
	int err_code;
	if(mainkey == NULL || subkey == NULL) {
		db_error("NULL pointer\n");	
		return ETC_INVALIDOBJ;
	}

	if(mCfgFileHandle == 0) {
		db_error("cfgHandle not initialized\n");
		return ETC_INVALIDOBJ;
	}

	if((err_code = GetValueFromEtc(mCfgFileHandle, mainkey, subkey, value, len)) != ETC_OK) {
		db_error("get %s->%s failed:%s\n", mainkey, subkey, pEtcError(err_code));
		return err_code;
	}

	return ETC_OK;
}

/* get the rect from configFile
 * pWindow the the section name in the config file
 * */
int ResourceManager::getRectFromFileHandle(const char *pWindow, CDR_RECT *rect)
{
	int err_code;

	if(mCfgFileHandle == 0) {
		db_msg("mCfgFileHandle not initialized\n");
		return ETC_INVALIDOBJ;
	}

	if((err_code = GetIntValueFromEtc(mCfgFileHandle, pWindow, "x", &rect->x)) != ETC_OK) {
		return err_code;
	}

	if((err_code = GetIntValueFromEtc(mCfgFileHandle, pWindow, "y", &rect->y)) != ETC_OK) {
		return err_code;
	}

	if((err_code = GetIntValueFromEtc(mCfgFileHandle, pWindow, "w", &rect->w)) != ETC_OK) {
		return err_code;
	}

	if((err_code = GetIntValueFromEtc(mCfgFileHandle, pWindow, "h", &rect->h)) != ETC_OK) {
		return err_code;
	}

	return ETC_OK;
}

/*
 * get the ABGR format color from the configFile
 * On success return ETC_OK
 * On fail return err code
 * */
int ResourceManager::getABGRColorFromFileHandle(const char* pWindow, const char* subkey, gal_pixel &pixel)
{
	char buf[20] = {0};
	int err_code;
	unsigned long int hex;
	unsigned char r, g, b, a;

	if(pWindow == NULL || subkey == NULL)
		return ETC_INVALIDOBJ;

	if(mCfgFileHandle == 0) {
		db_error("mCfgFileHandle not initialized\n");
		return ETC_INVALIDOBJ;
	}

	if((err_code = GetValueFromEtc(mCfgFileHandle, pWindow, subkey, buf, sizeof(buf))) != ETC_OK) {
		return err_code;
	}

	hex = strtoul(buf, NULL, 16);

	a = (hex >> 24) & 0xff;
	b = (hex >> 16) & 0xff;
	g = (hex >> 8) & 0xff;
	r = hex & 0xff;

	pixel = RGBA2Pixel(HDC_SCREEN, r, g, b, a);

	return ETC_OK;
}

/*
 * get current icon info from configFile, and fill the giving currentIcon
 * example : if current is 1, then the icon1 is used
 * [status_bar_battery]
 *	current=1
 *	icon0=/res/topbar/baterry1.png
 *	icon1=/res/topbar/baterry2.png
 *	icon2=/res/topbar/baterry3.png
 *	icon3=/res/topbar/baterry4.png
 * On success return ETC_OK
 * On fail return err code
 * */
int ResourceManager::fillCurrentIconInfoFromFileHandle(const char* pWindow, currentIcon_t &currentIcon)
{
	int current;
	char buf[10] = {0};
	char bufIcon[50] = {0};

	if(pWindow == NULL) {
		db_msg("invalid pWindow\n");
		return ETC_INVALIDOBJ;
	}

	current = getIntValueFromHandle(mCfgFileHandle, pWindow, "current");
	if(current < 0) {
		db_error("get current value from %s failed\n", pWindow);
		return ETC_KEYNOTFOUND;
	}
	currentIcon.current = current;
	//	db_msg("current is %d\n", currentIcon.current);

	current = 0;
	do
	{
		int err_code;
		sprintf(buf, "icon%d", current);
		//		db_msg("buf is %s\n", buf);
		err_code = GetValueFromEtc(mCfgFileHandle, pWindow, buf, bufIcon, sizeof(bufIcon));
		if(err_code != ETC_OK) {
			if(current == 0) {
				return err_code;
			} else {
				break;
			}
		}
		currentIcon.icon.insert(currentIcon.icon.end(), std::string(bufIcon));
		current++;
	}while(1);

	//	db_msg("%s has %u icons\n", pWindow, currentIcon.icon.size());
	for(unsigned int i = 0;  i< currentIcon.icon.size(); i++) {
		//		db_msg("icon%d is %s\n", i, currentIcon.icon.itemAt(i).c_str());	
	}

	if(currentIcon.current >= currentIcon.icon.size()) {
		db_error("invalide current value, current is %d icon count is %u\n", currentIcon.current, currentIcon.icon.size());	
		return ETC_INVALIDOBJ;
	}

	return ETC_OK;
}


/*
 * get language from config menu
 * On success return LANGUAGE
 * On faile return LANG_ERR
 * */
LANGUAGE ResourceManager::getLangFromMenuHandle(void)
{
	int retval;

	retval = getIntValueFromHandle(mCfgMenuHandle, "language", "current");
    if(retval == -1 || retval >= LANG_ERR) {
        db_error("get current language failed\n");
        return LANG_ERR;
    } else if (retval < LANG_ERR)
        return (LANGUAGE)retval;

	return LANG_ERR;
}

LANGUAGE ResourceManager::getLangFromConfigFile(void)
{
	int retval;
	if(GetIntValueFromEtcFile(configMenu, "language", "current", &retval) != ETC_OK) {
		db_error("get language failed\n");
		return LANG_ERR;
	}

	if(retval < LANG_ERR)
		return (LANGUAGE)retval;

	return LANG_ERR;
}

int ResourceManager::setRecordingStatus(bool recording)
{
	mRecording = recording;
	return 0;
}

BITMAP * ResourceManager::getNoVideoImage()
{
	if(	mNoVideoImage.bmWidth == -1){
		return NULL;
	}
	return &mNoVideoImage;
}
int ResourceManager::getConfig(__aw_cdr_net_config *config)
{	
	config->record_quality = rMenuList.menuDataVideoResolution.current;
	config->record_duration = rMenuList.menuDataVTL.current;
	config->record_delayp = rMenuList.menuDataDelayP.current;
	config->record_slowp = rMenuList.menuDataSlowP.current;
	config->record_Sound = (int)rMenuList.recordSoundEnable;

	config->photo_quality = rMenuList.menuDataPhotoResolution.current;
	config->photo_timetakephotos= rMenuList.menuDataPhotoTimeTakePhotos.current;
	config->photo_automatictakephotos= rMenuList.menuDataPhotoAutomaticTakePhotos.current;
	config->photo_motiontakephotos= rMenuList.menuDataPhotoMotionTakePhotos.current;
	
	config->exposure = rMenuList.menuDataExposure.current;
	config->white_balance = rMenuList.menuDataWB.current;
	config->image_rotation = rMenuList.ImageRotation;
	config->led_freq = rMenuList.menuDataLightFreq.current;
	config->led_on_off = rMenuList.LEDLights;
	config->screen_sleep = rMenuList.menuDataSS.current;

	config->record_switch = mRecording;
	config->slow_record_switch = mRecording;
	config->language = rMenuList.menuDataLang.current;
	config->watermark = (int)rMenuList.TWMEnable;
	strcpy(config->firmware_version, FIRMWARE_VERSION);
	return 0;
}
int ResourceManager::sessionControl(int sid, int cmd, char* buf, void *srv)
{
	int ret = 0;
	db_msg("by hero *** cmd:%x, param_in:%p\n", cmd, buf);
	CdrServer *cdrServer = (CdrServer*)srv;
	switch(cmd)
	{
		case NAT_CMD_SET_RECORDER_QUALITY://??
			{
				int param_in = ((aw_cdr_set_recorder_quality*)buf)->index;
				aw_cdr_set_recorder_quality_resp resp;
				ret = setResIntValue(ID_MENU_LIST_VIDEO_RESOLUTION, INTVAL_SUBMENU_INDEX, param_in);
				db_msg("NAT_CMD_SET_RECORDER_QUALITY param_in:%d, ret:%d\n", param_in, ret);
			
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_SET_RECORDER_QUALITY_RESP, 
					(char *)(&resp), sizeof(aw_cdr_set_recorder_quality_resp));
			}
			break;
		case NAT_CMD_SET_RECORDER_BITRATE://??
			{
				int param_in = ((aw_cdr_set_recorder_quality*)buf)->index;
				aw_cdr_set_recorder_quality_resp resp;
				ret = setResIntValue(ID_MENU_LIST_VIDEO_BITRATE, INTVAL_SUBMENU_INDEX, param_in);
				db_msg("NAT_CMD_SET_RECORDER_QUALITY param_in:%d, ret:%d\n", param_in, ret);
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_SET_RECORDER_BITRATE_RESP, 
					(char *)(&resp), sizeof(aw_cdr_set_recorder_quality_resp));
			}
			break;
		case NAT_CMD_SET_RECORDER_DELAY_TIME:
			{
				int param_in = ((aw_cdr_set_recorder_quality*)buf)->index;
				aw_cdr_set_recorder_quality_resp resp;
				ret = setResIntValue(ID_MENU_LIST_VIDEO_DELAYP, INTVAL_SUBMENU_INDEX, param_in);
				db_msg("NAT_CMD_SET_RECORDER_DELAY_TIME param_in:%d, ret:%d\n", param_in, ret);
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_SET_RECORDER_DELAY_TIME_RESP, 
					(char *)(&resp), sizeof(aw_cdr_set_recorder_quality_resp));
			}
			break;
		case NAT_CMD_SLOW_RECORD_VEDIO_SIZE:
			{
				if(mSlowRecordInit){
					mSlowRecordInit = false;
					int param_in = ((aw_cdr_set_recorder_quality*)buf)->index;
					aw_cdr_set_recorder_quality_resp resp;
					db_error("++++++invalid value: %d, count is %d 55 ++++++++\n",param_in);
					ret = setResIntValue(ID_MENU_LIST_VIDEO_SLOWP, INTVAL_SUBMENU_INDEX, param_in);
					db_error("++++++invalid value: %d, count is %d 33 ++++++++\n");
					db_msg("NAT_CMD_SET_RECORDER_DELAY_TIME param_in:%d, ret:%d\n", param_in, ret);
					resp.result = ret;
					db_error("++++++invalid value: %d, count is %d 44++++++++\n");
					cdrServer->reply(sid, NAT_CMD_SLOW_RECORD_VEDIO_SIZE_RESP, 
						(char *)(&resp), sizeof(aw_cdr_set_recorder_quality_resp));
				}
			}
			break;
		case NAT_CMD_SET_RECORDER_DURATION://??
			{
				int param_in = ((aw_cdr_set_recorder_duration*)buf)->index;
				aw_cdr_set_recorder_duration_resp resp;
				ret = setResIntValue(ID_MENU_LIST_VTL, INTVAL_SUBMENU_INDEX, param_in);
				db_msg("NAT_CMD_SET_RECORDER_DURATION param_in:%d, ret:%d\n", param_in, ret);
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_SET_RECORDER_DURATION_RESP, 
					(char *)(&resp), sizeof(aw_cdr_set_recorder_quality_resp));
			}
			break;
		case NAT_CMD_SET_PHOTO_QUALITY://??
			{
				int param_in = ((aw_cdr_set_photo_quality*)buf)->index;
				ret = setResIntValue(ID_MENU_LIST_PHOTO_RESOLUTION, INTVAL_SUBMENU_INDEX, param_in);
				db_msg("NAT_CMD_SET_PHOTO_QUALITY_RESP param_in:%d, ret:%d\n", param_in, ret);
				aw_cdr_set_photo_quality_resp resp;
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_SET_PHOTO_QUALITY_RESP, 
					(char *)(&resp), sizeof(aw_cdr_set_photo_quality_resp));
			}
			break;
		case NAT_CMD_SET_WHITE_BALANCE://??
			{
				int param_in = ((aw_cdr_set_white_balance*)buf)->index;
				ret = setResIntValue(ID_MENU_LIST_WB, INTVAL_SUBMENU_INDEX, param_in);
				db_msg("NAT_CMD_SET_WHITE_BALANCE param_in:%d, ret:%d\n", param_in, ret);
				aw_cdr_set_white_balance_resp resp;
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_SET_WHITE_BALANCE_RESP, 
					(char *)(&resp), sizeof(aw_cdr_set_photo_quality_resp));
			}
			break;
		case NAT_CMD_SET_EXPOSURE://??
			{
				int param_in = ((aw_cdr_set_exposure*)buf)->index;
				ret = setResIntValue(ID_MENU_LIST_EXPOSURE, INTVAL_SUBMENU_INDEX, param_in);
				db_msg("NAT_CMD_SET_EXPOSURE param_in:%d, ret:%d\n", param_in, ret);
				aw_cdr_set_exposure_resp resp;
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_SET_EXPOSURE_RESP, 
					(char *)(&resp), sizeof(aw_cdr_set_exposure_resp));

			}
			break;
		case NAT_CMD_SET_BOOT_RECORDER://??
			{
				int param_in = ((aw_cdr_set_boot_recorder*)buf)->index;
				ret = setResBoolValue(ID_MENU_LIST_POR, (bool)param_in);
				aw_cdr_set_boot_recorder_resp resp;
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_SET_BOOT_RECORDER_RESP, 
					(char *)(&resp), sizeof(aw_cdr_set_boot_recorder_resp));
			}
			break;
		case NAT_CMD_SET_MUTE://??
			{
				int param_in = ((aw_cdr_set_mute*)buf)->index;
				ret = setResBoolValue(ID_MENU_LIST_RECORD_SOUND, (bool)param_in);
				aw_cdr_set_mute_resp resp;
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_SET_MUTE_RESP, 
					(char *)(&resp), sizeof(aw_cdr_set_mute_resp));
			}
			break;
		case NAT_CMD_SET_REAR_VIEW_MIRROR:
			break;
		case NAT_CMD_SET_TIME: //??
			{
				struct tm *time_us = (struct tm *)buf;
				time_us->tm_year -= 1900;
				time_us->tm_mon -= 1;
				db_msg("time is (%d:%d:%d:%d:%d:%d)\n", time_us->tm_year, time_us->tm_mon, time_us->tm_mday, time_us->tm_hour, time_us->tm_min, time_us->tm_sec);
				ret = setDateTime(time_us);
				aw_cdr_set_time_resp resp;
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_SET_TIME_RESP, 
					(char *)(&resp), sizeof(aw_cdr_set_time_resp));
			}
			break;
		case NAT_CMD_SET_LANGUAGE://??
			{
				int param_in = ((aw_cdr_set_language*)buf)->index;
				ret = setResIntValue(ID_MENU_LIST_LANG, INTVAL_SUBMENU_INDEX, param_in);
				db_msg("NAT_CMD_SET_LANGUAGE param_in:%d, ret:%d\n", param_in, ret);
				aw_cdr_set_language_resp resp;
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_SET_LANGUAGE_RESP, 
					(char *)(&resp), sizeof(aw_cdr_set_language_resp));
			}
			break;
		case NAT_CMD_SET_SMART_DETECT:
			break;
		case NAT_CMD_SET_LANE_LINE_CALIBRATION:
			break;
		case NAT_CMD_SET_IMPACT_SENSITIVITY:
			break;
		case NAT_CMD_SET_MOTION_DETECT: //??
			{
				int param_in = ((aw_cdr_set_motion_detect*)buf)->index;
				ret = setResBoolValue(ID_MENU_LIST_AWMD, (bool)param_in);
				aw_cdr_set_motion_detect_resp resp;
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_SET_MOTION_DETECT_RESP, 
					(char *)(&resp), sizeof(aw_cdr_set_motion_detect_resp));
			}
			break;
		case NAT_CMD_SET_WATERMARK: //??
			{
				int param_in = ((aw_cdr_set_watermark*)buf)->index;
				ret = setResBoolValue(ID_MENU_LIST_TWM, (bool)param_in);
				aw_cdr_set_watermark_resp resp;
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_SET_WATERMARK_RESP, 
					(char *)(&resp), sizeof(aw_cdr_set_watermark_resp));
			}
			break;
		case NAT_CMD_FORMAT_TF_CARD: //??
			{
				ret = SendMessage(mHwnd[WINDOWID_MENU], MSG_FORMAT_TFCARD_NOW, 0, 0);
				db_msg("NAT_CMD_FORMAT_TF_CARD , ret:%d\n", ret);
				aw_cdr_format_tf_card_resp resp;
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_FORMAT_TF_CARD_RESP, 
					(char *)(&resp), sizeof(aw_cdr_format_tf_card_resp));
			}
			break;

		case NAT_CMD_FACTORY_RESET: //??
			{
				ret = SendMessage(mHwnd[WINDOWID_MENU], MSG_RM_FACTORY_RESET, 0, 0);
				db_msg("NAT_CMD_FACTORY_RESET , ret:%d\n", ret);
				aw_cdr_format_tf_card_resp resp;
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_FACTORY_RESET_RESP, 
					(char *)(&resp), sizeof(aw_cdr_format_tf_card_resp));
			}
			break;
			
		case NAT_CMD_PLAY_RECORDER:
			break;
		case NAT_CMD_GET_FILE_LIST: //
			{
				char *str = NULL;
				ret = SendMessage(mHwnd[WINDOWID_PLAYBACKPREVIEW], MSG_RM_GET_FILELIST, (WPARAM)&str, 0);
				char *strIndex = str;
				if (strIndex == NULL) {
					break;
				}
				aw_cdr_get_file_list_resp resp;
				resp.totalCnt = strlen(str) / MAX_PACKET_FILE_LIST_LEN + 1;
				resp.currentIndex = 0;
				while (strIndex != NULL) {
					memset(resp.filelist, 0, MAX_PACKET_FILE_LIST_LEN);
					int cpyLen = (strlen(strIndex) > MAX_PACKET_FILE_LIST_LEN)?
						MAX_PACKET_FILE_LIST_LEN:strlen(strIndex);
					memcpy(resp.filelist, strIndex, cpyLen);
					cdrServer->reply(sid, NAT_CMD_GET_FILE_LIST_RESP, 
						(char *)(&resp), sizeof(aw_cdr_get_file_list_resp));
					resp.currentIndex++;
					if (strlen(strIndex) > MAX_PACKET_FILE_LIST_LEN) {
						strIndex += MAX_PACKET_FILE_LIST_LEN;
					} else {
						strIndex = NULL;
					}
				}
				if (str != NULL) {
					free(str);
				}
			}
			break;
		case NAT_CMD_CHECK_TF_CARD://
			{
				aw_cdr_tf_capacity resp;
				ret = SendMessage(mHwnd[WINDOWID_MAIN], MSG_RM_GET_CAPACITY, (WPARAM)&resp, 0);
				db_msg("NAT_CMD_CHECK_TF_CARD , resp:(total:%llu, remain:%llu)ret:%d, size:%d\n", 
					resp.total, resp.remain, ret,sizeof(aw_cdr_tf_capacity));
				cdrServer->reply(sid, NAT_CMD_CHECK_TF_CARD_RESP, 
					(char *)(&resp), sizeof(aw_cdr_tf_capacity));
			}
			break;
		case NAT_CMD_DELETE_FILE://
			{
				char *file = ((aw_cdr_handle_file *)buf)->filename;
				aw_cdr_delete_file_resp resp = {0};
				resp.result = SendMessage(mHwnd[WINDOWID_PLAYBACKPREVIEW], MSG_RM_DEL_FILE, (WPARAM)file, 0);
				cdrServer->reply(sid, NAT_CMD_DELETE_FILE_RESP, (char *)&resp, sizeof(resp));
			}
			break;
		case NAT_CMD_RECORD_CTL:
			break;
		case NAT_CMD_SET_SYNC_FILE:
			break;
		case NAT_CMD_GET_CONFIG://
			{
				aw_cdr_net_config config;
				memset(&config, 0, sizeof(aw_cdr_net_config));
				getConfig(&config);
				cdrServer->reply(sid, NAT_CMD_GET_CONFIG_RESP, (char *)(&config), sizeof(aw_cdr_net_config));
			}
			break;
		case NAT_CMD_TAKE_PHOTO://??
			{
				ret = SendMessage(mHwnd[WINDOWID_RECORDPREVIEW], MSG_RM_TAKE_PICTURE, 0, 0);
				aw_cdr_take_photo_resp resp;
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_TAKE_PHOTO_RESP, 
					(char *)(&resp), sizeof(aw_cdr_take_photo_resp));
			}
			break;

		case NAT_CMD_LIST_IMAGE_ROTATION://??
			{
				int param_in = ((aw_cdr_set_white_balance*)buf)->index;
				if(param_in == 0)
					ret = setResBoolValue(ID_MENU_LIST_IMAGE_ROTATION, false);
				else
					ret = setResBoolValue(ID_MENU_LIST_IMAGE_ROTATION, true);
				aw_cdr_take_photo_resp resp;
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_LIST_IMAGE_ROTATION_RESP, 
					(char *)(&resp), sizeof(aw_cdr_take_photo_resp));
			}
			break;
			
		
		case NAT_CMD_TIME_TAKE_PHOTO://??
			{
				int param_in = ((aw_cdr_set_white_balance*)buf)->index;
				ret = setResIntValue(ID_MENU_LIST_PHOTO_TIMETAKEPHOTOS, INTVAL_SUBMENU_INDEX, param_in);
				db_msg("NAT_CMD_SET_WHITE_BALANCE param_in:%d, ret:%d\n", param_in, ret);
				aw_cdr_set_white_balance_resp resp;
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_TIME_TAKE_PHOTO_RESP, 
					(char *)(&resp), sizeof(aw_cdr_set_photo_quality_resp));
			}
			break;
			
		case NAT_CMD_AUTO_TIME_TAKE_PHOTO://??
			{
				int param_in = ((aw_cdr_set_white_balance*)buf)->index;
				ret = setResIntValue(ID_MENU_LIST_PHOTO_AUTOMATICTAKEPHOTOS, INTVAL_SUBMENU_INDEX, param_in);
				db_msg("NAT_CMD_SET_WHITE_BALANCE param_in:%d, ret:%d\n", param_in, ret);
				aw_cdr_set_white_balance_resp resp;
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_AUTO_TIME_TAKE_PHOTO_RESP, 
					(char *)(&resp), sizeof(aw_cdr_set_photo_quality_resp));
			}
			break;
			
		case NAT_CMD_MOTION_TAKE_PHOTO://??
			{
				int param_in = ((aw_cdr_set_white_balance*)buf)->index;
				ret = setResIntValue(ID_MENU_LIST_PHOTO_MOTIONTAKEPHOTOS, INTVAL_SUBMENU_INDEX, param_in);
				db_msg("NAT_CMD_MOTION_TAKE_PHOTO param_in:%d, ret:%d\n", param_in, ret);
				aw_cdr_set_white_balance_resp resp;
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_MOTION_TAKE_PHOTO_RESP, 
					(char *)(&resp), sizeof(aw_cdr_set_photo_quality_resp));
			}
			break;
			
		case NAT_CMD_REQUEST_FILE_FINISH:
			{
				aw_cdr_file_trans* file_name = (aw_cdr_file_trans*) buf;
				db_msg("*******filename(%s)\n", file_name->filename);
				int flag =strcmp(file_name->filename,"/mnt/extsd/full_img.fex");
				db_msg("*********************flage=%d*******\n",flag);
				if(flag==0){
				    SendMessage(mHwnd[WINDOWID_MAIN], MSG_OTA_UPDATE, 0, 0);
				}
				
			}
			break;
		case NAT_CMD_TURN_OFF_WIFI: //??
			ret = setResBoolValue(ID_MENU_LIST_WIFI, false);
			break;

		case NAT_CMD_LED_ON_OFF: 
		{//??
			int param_in = ((aw_cdr_set_white_balance*)buf)->index;
			if(param_in == 0)
				ret = setResBoolValue(ID_MENU_LIST_LED_LIGHTS, false);
			else
				ret = setResBoolValue(ID_MENU_LIST_LED_LIGHTS, true);
			aw_cdr_set_white_balance_resp resp;
			resp.result = ret;
			cdrServer->reply(sid, NAT_CMD_LED_ON_OFF_RESP, 
				(char *)(&resp), sizeof(aw_cdr_set_photo_quality_resp));
		}
			break;
		case NAT_CMD_LED_FREQ://??
			{
				int param_in = ((aw_cdr_set_white_balance*)buf)->index;
				ret = setResIntValue(ID_MENU_LIST_LIGHT_FREQ, INTVAL_SUBMENU_INDEX, param_in);
				db_msg("NAT_CMD_LED_FREQ param_in:%d, ret:%d\n", param_in, ret);
				aw_cdr_set_white_balance_resp resp;
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_LED_FREQ_RESP, 
					(char *)(&resp), sizeof(aw_cdr_set_photo_quality_resp));
			}
			break;

		case NAT_CMD_SCREEN_SLEEP://??
			{
				int param_in = ((aw_cdr_set_white_balance*)buf)->index;
				ret = setResIntValue(ID_MENU_LIST_SS, INTVAL_SUBMENU_INDEX, param_in);
				db_msg("NAT_CMD_LED_FREQ param_in:%d, ret:%d\n", param_in, ret);
				aw_cdr_set_white_balance_resp resp;
				resp.result = ret;
				cdrServer->reply(sid, NAT_CMD_SCREEN_SLEEP_RESP, 
					(char *)(&resp), sizeof(aw_cdr_set_photo_quality_resp));
			}
			break;
		default:
			ret = -1;
			break;
	}
	return ret;
}


void ResourceManager::syncConfigureToDisk(void)
{
	char buf[3] = {0};
	configTable2 configTableSwitch[] = {
		{CFG_MENU_POR, (void*)&rMenuList.POREnable},
		{CFG_MENU_RECORD_SOUND,	(void*)&rMenuList.recordSoundEnable},
		{CFG_MENU_IMAGE_ROTATION,	(void*)&rMenuList.ImageRotation},
		{CFG_MENU_LED_LIGHTS,	(void*)&rMenuList.LEDLights},
		{CFG_MENU_TWM,		(void*)&rMenuList.TWMEnable},
		{CFG_MENU_PHOTO_WM,		(void*)&rMenuList.photoWMEnable},
		{CFG_MENU_AWMD,		(void*)&rMenuList.AWMDEnable},
		{CFG_MENU_DRIVING_MODE,		(void*)&rMenuList.drivingModeEnable},
		{CFG_MENU_WIFI,		(void*)&rMenuList.wifiEnable},
#ifdef APP_ADAS
		{CFG_MENU_SMARTALGORITHM, 	(void*)&rMenuList.smartAlgorithmEnable},
#endif
		{CFG_MENU_ALIGNLINE,		(void*)&rMenuList.alignlineEnable},
		{CFG_MENU_STANDBY_MODE,		(void*)&rMenuList.standbyModeEnable},
		{CFG_MENU_KEYTONE,			(void*)&rMenuList.keyToneEnable}
	};

	configTable2 configTableMenuItem[] = {
		{CFG_MENU_LANG, (void*)&rMenuList.menuDataLang},
		{CFG_MENU_VIDEO_RESOLUTION,	(void*)&rMenuList.menuDataVideoResolution},
		{CFG_MENU_VIDEO_BITRATE,	(void*)&rMenuList.menuDataVideoBitrate},
		{CFG_MENU_PHOTO_RESOLUTION,	(void*)&rMenuList.menuDataPhotoResolution},
		{CFG_MENU_VIDEO_DELAYP,	(void*)&rMenuList.menuDataDelayP},
		{CFG_MENU_VIDEO_SLOWP,	(void*)&rMenuList.menuDataSlowP},		
		{CFG_MENU_VTL,	(void*)&rMenuList.menuDataVTL},
		{CFG_MENU_WB,	(void*)&rMenuList.menuDataWB},
		{CFG_MENU_CONTRAST, (void*)&rMenuList.menuDataContrast},
		{CFG_MENU_EXPOSURE, (void*)&rMenuList.menuDataExposure},
		{CFG_MENU_SS,		(void*)&rMenuList.menuDataSS},
		{CFG_MENU_GSENSOR,	(void*)&rMenuList.menuDataGsensor},
		{CFG_MENU_VOICEVOL,	(void*)&rMenuList.menuDataVoiceVol},
		{CFG_MENU_DELAY_SHUTDOWN, (void*)&rMenuList.menuDataShutDown},
		{CFG_MENU_PARK,		(void*)&rMenuList.menuDataPark},
		{CFG_MENU_PHOTO_COMPRESSION_QUALITY,		(void*)&rMenuList.menuDataPhotoCompressionQuality},
		{CFG_MENU_PHOTO_TIMETAKEPHOTOS,          (void*)&rMenuList.menuDataPhotoTimeTakePhotos},
		{CFG_MENU_PHOTO_AUTOMATICTAKEPHOTOS,	(void*)&rMenuList.menuDataPhotoAutomaticTakePhotos},
		{CFG_MENU_PHOTO_MOTIONTAKEPHOTOS,	   (void*)&rMenuList.menuDataPhotoMotionTakePhotos},

	};

	if(getCfgMenuHandle() < 0) {
		db_error("get config menu handle failed\n");	
		return;
	}

	db_msg("syncConfigureToDisk\n");
	for(unsigned int i = 0; i < TABLESIZE(configTableSwitch); i++) {
		if(*(bool*)configTableSwitch[i].addr == true)
			buf[0] = '1';
		else
			buf[0] = '0';
		db_msg("buf is %s\n", buf);
		SetValueToEtc(mCfgMenuHandle, "switch", configTableSwitch[i].mainkey, buf);
	}
	db_msg("syncConfigureToDisk\n");

	for(unsigned int i = 0; i < TABLESIZE(configTableMenuItem); i++) {
		sprintf(buf, "%d", ((menuItem_t*)(configTableMenuItem[i].addr))->current);
		db_msg("buf is %s\n", buf);
		SetValueToEtc(mCfgMenuHandle, configTableMenuItem[i].mainkey, "current", buf);

		sprintf(buf, "%d", ((menuItem_t*)(configTableMenuItem[i].addr))->count);
		db_msg("buf is %s\n", buf);
		SetValueToEtc(mCfgMenuHandle, configTableMenuItem[i].mainkey, "count", buf);
	}
#if 1
	sprintf(buf, "%d", (menuDataLWM.LWMEnable) % 10);
	db_msg("menuDataLWM.LWMEnable buf is %s\n", buf);
	SetValueToEtc(mCfgMenuHandle, CFG_MENU_DEFAULTLICENSE , "current", buf);

//	if(menuDataLWM.LWMEnable && (menuDataLWM.lwaterMark[9] != 0) )
	if(menuDataLWM.LWMEnable  )
		SetValueToEtc(mCfgMenuHandle, CFG_MENU_DEFAULTLICENSE , "position", menuDataLWM.lwaterMark);

	db_msg("menuDataLWM.lwaterMark  is %s\n", menuDataLWM.lwaterMark);
#endif
	SetValueToEtc(mCfgMenuHandle, CFG_VERIFICATION, "current", (char*)"1");
	db_msg("syncConfigureToDisk\n");
	SaveEtcToFile(mCfgMenuHandle, configMenu);

	releaseCfgMenuHandle();
	db_msg("syncConfigureToDisk finished\n");
}

bool ResourceManager::verifyConfig()
{
	int retval;
	int err_code;

	if((err_code = GetIntValueFromEtc(mCfgMenuHandle, CFG_VERIFICATION, "current", &retval)) != ETC_OK) {
		db_error("get %s->%s failed:%s\n", CFG_VERIFICATION, "current", pEtcError(err_code));
		db_msg("verify %s %s\n", CFG_VERIFICATION, (retval==1)?"correct":"incorrect");
		return false;
	}

	if (retval != 1) {
		db_msg("verify %s %s\n", CFG_VERIFICATION, (retval==1)?"correct":"incorrect");
		return false;
	}
	db_msg("verify %s %s\n", CFG_VERIFICATION, (retval==1)?"correct":"incorrect");
	return true;
}

void ResourceManager::resetResource(void)
{
/*
	releaseCfgFileHandle();
	if(copyFile(defaultConfigFile, configFile) < 0) {
		db_error("reset config file failed\n");	
		return;
	}
*/
	releaseCfgMenuHandle();
	if(copyFile(defaultConfigMenu, configMenu) < 0) {
		db_error("reset config menu failed\n");	
		return;
	}

	if(initStage1() < 0) {
		db_error("initStage1 failed\n");	
		return;
	}
	if(initStage2() < 0) {
		db_error("initStage2 failed\n");
		return;
	}
	/*
	SendMessage(mHwnd[WINDOWID_STATUSBAR], STBM_WIFI, 0, 0);
	SendMessage(mHwnd[WINDOWID_MAIN], MSG_WIFI_SWITCH, 0, 0);
	
	SendMessage(mHwnd[WINDOWID_RECORDPREVIEW], MSG_AWMD, 0, 0);
	SendMessage(mHwnd[WINDOWID_STATUSBAR], STBM_AWMD, 0, 0);
	*/
	SendMessage(mHwnd[WINDOWID_MAIN], MSG_RESET_WIFI_PWD, 0, 0);
	/*
	SendMessage(mHwnd[WINDOWID_RECORDPREVIEW], MSG_ADAS_SCREEN, 0, 0);		
	SendMessage(mHwnd[WINDOWID_RECORDPREVIEW], MSG_DRAW_ALIGN_LINE, 0, 0);
	SendMessage(mHwnd[WINDOWID_STATUSBAR], STBM_PARK, 0, 0);
	*/

	struct tm *tblock;
	time_t timer;
	timer = time(NULL);
	tblock = localtime(&timer);
	tblock->tm_year=2015-1900;
	tblock->tm_mon=0;
	tblock->tm_mday=1;
	tblock->tm_hour=0;
	tblock->tm_min=0;
	tblock->tm_sec=0;
	if(setDateTime(tblock)!=0){
	   db_msg("***reset cdr time failed***\n");
	}

}

void ResourceManager::notifyRM()
{
	std::unique_lock<std::mutex> lock(mLock);
	mNotifyRM.notify_one();
	mSignaled = true;
	
}
