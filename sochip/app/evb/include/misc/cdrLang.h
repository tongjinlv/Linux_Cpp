  
#ifndef __CDRLANG_H__
#define __CDRLANG_H__

#include <vector>
#include <map>
#include <string>

#include <minigui/common.h>
#include <minigui/gdi.h>

#include "cdr.h"







enum LANG_STRINGS {
	LANG_LABEL_RECORDPREVIEW_MENU = 0, 	 //line 1
	LANG_LABEL_PHOTO_MENU, 	 //line 2
	LANG_LABEL_PLAYBACKPREVIEW_MENU, 	 //line 3
	LANG_LABEL_SYSTEM_MENU, 	 //line 4
	LANG_LABEL_MENU_PARK, 	 //line 5
	LANG_LABEL_MENU_GSENSOR, 	 //line 6
	LANG_LABEL_MENU_RECORD_SOUND, 	 //line 7
	LANG_LABEL_MENU_VOICEVOL, 	 //line 8
	LANG_LABEL_MENU_KEYTONE, 	 //line 9
	LANG_LABEL_MENU_LIGHT_FREQ, 	 //line 10
	LANG_LABEL_MENU_TFCARD_INFO, 	 //line 11
	LANG_LABEL_MENU_DELAY_SHUTDOWN, 	 //line 12
	LANG_LABEL_MENU_FORMAT, 	 //line 13
	LANG_LABEL_MENU_LANG, 	 //line 14
	LANG_LABEL_MENU_DATE, 	 //line 15
	LANG_LABEL_MENU_SS, 	 //line 16
	LANG_LABEL_MENU_STANDBY_MODE, 	 //line 17
	LANG_LABEL_MENU_FIRMWARE, 	 //line 18
	LANG_LABEL_MENU_FRESET, 	 //line 19
	LANG_LABEL_MENU_ALIGNLINE, 	 //line 20
	LANG_LABEL_MENU_DRIVING_MODE, 	 //line 21
	LANG_LABEL_MENU_WIFI, 	 //line 21
	LANG_LABEL_MENU_IMAGE_ROTATION,
	LANG_LABEL_MENU_LED_LIGHTS,
	LANG_LABEL_MENU_QUIT,
	LANG_LABEL_MENU_VIDEO_RESOLUTION, 	 //line 22
	LANG_LABEL_MENU_VIDEO_BITRATE, 	 //line 23
	LANG_LABEL_MENU_PHOTO_RESOLUTION, 	 //line 24
	LANG_LABEL_MENU_PHOTO_COMPRESSION_QUALITY, 	 //line 25
	LANG_LABEL_MENU_VTL, 	 //line 26
	LANG_LABEL_MENU_VEDIO_DELAY,
	LANG_LABEL_MENU_VEDIO_SLOW_RECORD,
	LANG_LABEL_MENU_WB, 	 //line 27
	LANG_LABEL_MENU_CONTRAST, 	 //line 28
	LANG_LABEL_MENU_EXPOSURE, 	 //line 29
	LANG_LABEL_MENU_POR, 	 //line 30
	LANG_LABEL_MENU_PHOTO_WM, 	 //line 31
	LANG_LABEL_MENU_TWM, 	 //line 32
	LANG_LABEL_MENU_AWMD, 	 //line 33
	LANG_LABEL_MENU_LICENSE_PLATE_WM, 	 //line 34
	LANG_LABEL_MENU_SMARTALGORITHM, 	 //line 35
	LANG_LABEL_SUBMENU_PARK_TITLE, 	 //line 36
	LANG_LABEL_SUBMENU_PARK_CONTENT0, 	 //line 37
	LANG_LABEL_SUBMENU_PARK_CONTENT1, 	 //line 38
	LANG_LABEL_SUBMENU_PARK_CONTENT2, 	 //line 39
	LANG_LABEL_SUBMENU_PARK_CONTENT3, 	 //line 40
	LANG_LABEL_SUBMENU_GSENSOR_TITLE, 	 //line 41
	LANG_LABEL_SUBMENU_GSENSOR_CONTENT0, 	 //line 42
	LANG_LABEL_SUBMENU_GSENSOR_CONTENT1, 	 //line 43
	LANG_LABEL_SUBMENU_GSENSOR_CONTENT2, 	 //line 44
	LANG_LABEL_SUBMENU_GSENSOR_CONTENT3, 	 //line 45
	LANG_LABEL_SUBMENU_VOICEVOL_TITLE, 	 //line 46
	LANG_LABEL_SUBMENU_VOICEVOL_CONTENT0, 	 //line 47
	LANG_LABEL_SUBMENU_VOICEVOL_CONTENT1, 	 //line 48
	LANG_LABEL_SUBMENU_VOICEVOL_CONTENT2, 	 //line 49
	LANG_LABEL_SUBMENU_LIGHT_FREQ_TITLE, 	 //line 50
	LANG_LABEL_SUBMENU_LIGHT_FREQ_CONTENT0, 
	LANG_LABEL_SUBMENU_LIGHT_FREQ_CONTENT1, 	 //line 51
	LANG_LABEL_SUBMENU_LIGHT_FREQ_CONTENT3, 	 //line 52
	LANG_LABEL_SUBMENU_TF_CARD_TITLE,
	LANG_LABEL_SUBMENU_TF_CARD_CONTENT0,
	LANG_LABEL_SUBMENU_TF_CARD_CONTENT1,
	LANG_LABEL_SUBMENU_TF_CARD_CONTENT2,
	LANG_LABEL_SUBMENU_LANG_TITLE, 	 //line 53
	LANG_LABEL_SUBMENU_LANG_CONTENT0, 	 //line 54
	LANG_LABEL_SUBMENU_LANG_CONTENT1, 	 //line 55
	LANG_LABEL_SUBMENU_LANG_CONTENT2, 	 //line 56
	LANG_LABEL_SUBMENU_LANG_CONTENT3, 	 //line 57
	LANG_LABEL_SUBMENU_LANG_CONTENT4, 	 //line 58
	LANG_LABEL_SUBMENU_LANG_CONTENT5, 	 //line 59
	LANG_LABEL_SUBMENU_LANG_CONTENT6, 	 //line 59
	LANG_LABEL_SUBMENU_LANG_CONTENT7, 	 //line 59
	LANG_LABEL_SUBMENU_LANG_CONTENT8, 	 //line 59
	LANG_LABEL_SUBMENU_LANG_CONTENT9, 	 //line 59
	LANG_LABEL_SUBMENU_LANG_CONTENT10, 	 //line 59
	LANG_LABEL_SUBMENU_SS_TITLE, 	 //line 60
	LANG_LABEL_SUBMENU_SS_CONTENT0, 	 //line 61
	LANG_LABEL_SUBMENU_SS_CONTENT1, 	 //line 62
	LANG_LABEL_SUBMENU_SS_CONTENT2, 	 //line 63
	LANG_LABEL_SUBMENU_SS_CONTENT3, 	 //line 64
	LANG_LABEL_SUBMENU_VIDEO_RESOLUTION_TITLE, 	 //line 65
	LANG_LABEL_SUBMENU_VIDEO_RESOLUTION_CONTENT0, 	 //line 66
	LANG_LABEL_SUBMENU_VIDEO_RESOLUTION_CONTENT1, 	 //line 67
	LANG_LABEL_SUBMENU_VIDEO_RESOLUTION_CONTENT2,
	LANG_LABEL_SUBMENU_VIDEO_RESOLUTION_CONTENT3,
	LANG_LABEL_SUBMENU_VIDEO_RESOLUTION_CONTENT4,
	LANG_LABEL_SUBMENU_VIDEO_RESOLUTION_CONTENT5,
	LANG_LABEL_SUBMENU_VIDEO_RESOLUTION_CONTENT6,
	LANG_LABEL_SUBMENU_VIDEO_DELAYP_TITLE,
	LANG_LABEL_SUBMENU_VIDEO_DELAYP_CONTENT0,
	LANG_LABEL_SUBMENU_VIDEO_DELAYP_CONTENT1,
	LANG_LABEL_SUBMENU_VIDEO_DELAYP_CONTENT2,
	LANG_LABEL_SUBMENU_VIDEO_DELAYP_CONTENT3,
	LANG_LABEL_SUBMENU_VIDEO_DELAYP_CONTENT4,
	LANG_LABEL_SUBMENU_VIDEO_DELAYP_CONTENT5,
	LANG_LABEL_SUBMENU_VIDEO_DELAYP_CONTENT6,
	LANG_LABEL_SUBMENU_VIDEO_DELAYP_CONTENT7,	
	LANG_LABEL_SUBMENU_VIDEO_SLOWP_TITLE,
	LANG_LABEL_SUBMENU_VIDEO_SLOWP_CONTENT0,
	LANG_LABEL_SUBMENU_VIDEO_SLOWP_CONTENT1,	
	LANG_LABEL_SUBMENU_VIDEO_BITRATE_TITLE, 	
	LANG_LABEL_SUBMENU_VIDEO_BITRATE_CONTENT0, 	
	LANG_LABEL_SUBMENU_VIDEO_BITRATE_CONTENT1, 	 
	LANG_LABEL_SUBMENU_VIDEO_BITRATE_CONTENT2, 	
	LANG_LABEL_SUBMENU_PHOTO_RESOLUTION_TITLE,
	LANG_LABEL_SUBMENU_PHOTO_RESOLUTION_CONTENT0, 	
	LANG_LABEL_SUBMENU_PHOTO_RESOLUTION_CONTENT1, 	
	LANG_LABEL_SUBMENU_PHOTO_RESOLUTION_CONTENT2, 	
	LANG_LABEL_SUBMENU_PHOTO_RESOLUTION_CONTENT3, 	
	LANG_LABEL_SUBMENU_PHOTO_RESOLUTION_CONTENT4,
	LANG_LABEL_SUBMENU_PHOTO_COMPRESSION_QUALITY_TITLE, 	 //line 77
	LANG_LABEL_SUBMENU_PHOTO_COMPRESSION_QUALITY_CONTENT0, 	 //line 78
	LANG_LABEL_SUBMENU_PHOTO_COMPRESSION_QUALITY_CONTENT1, 	 //line 79
	LANG_LABEL_SUBMENU_PHOTO_COMPRESSION_QUALITY_CONTENT2, 	 //line 80
	LANG_LABEL_SUBMENU_PHOTO_TIME_TAKEPHOTOS_TITLE,  //line 89
	LANG_LABEL_SUBMENU_PHOTO_TIME_TAKEPHOTOS_CONTENT0,
	LANG_LABEL_SUBMENU_PHOTO_TIME_TAKEPHOTOS_CONTENT1,
	LANG_LABEL_SUBMENU_PHOTO_TIME_TAKEPHOTOS_CONTENT2,
	LANG_LABEL_SUBMENU_PHOTO_TIME_TAKEPHOTOS_CONTENT3,
	LANG_LABEL_SUBMENU_PHOTO_TIME_TAKEPHOTOS_CONTENT4, //94
	LANG_LABEL_SUBMENU_PHOTO_AUTOMATIC_TAKEPHOTOS_TITLE,  //line 95
	LANG_LABEL_SUBMENU_PHOTO_AUTOMATIC_TAKEPHOTOS_CONTENT0,
	LANG_LABEL_SUBMENU_PHOTO_AUTOMATIC_TAKEPHOTOS_CONTENT1,
	LANG_LABEL_SUBMENU_PHOTO_AUTOMATIC_TAKEPHOTOS_CONTENT2,
	LANG_LABEL_SUBMENU_PHOTO_AUTOMATIC_TAKEPHOTOS_CONTENT3,
	LANG_LABEL_SUBMENU_PHOTO_AUTOMATIC_TAKEPHOTOS_CONTENT4, 
	LANG_LABEL_SUBMENU_PHOTO_AUTOMATIC_TAKEPHOTOS_CONTENT5, //101
	LANG_LABEL_SUBMENU_PHOTO_MOTION_TAKEPHOTOS_TITLE,  //line 102
	LANG_LABEL_SUBMENU_PHOTO_MOTION_TAKEPHOTOS_CONTENT0,
	LANG_LABEL_SUBMENU_PHOTO_MOTION_TAKEPHOTOS_CONTENT1,
	LANG_LABEL_SUBMENU_PHOTO_MOTION_TAKEPHOTOS_CONTENT2,
	LANG_LABEL_SUBMENU_PHOTO_MOTION_TAKEPHOTOS_CONTENT3,
	LANG_LABEL_SUBMENU_VTL_TITLE, 	 //line 81
	LANG_LABEL_SUBMENU_VTL_CONTENT0, 	 //line 82
	LANG_LABEL_SUBMENU_VTL_CONTENT1, 	 //line 83
	LANG_LABEL_SUBMENU_VTL_CONTENT2, 	 //line 84
	LANG_LABEL_SUBMENU_VTL_CONTENT3, 	 //line 84
	LANG_LABEL_SUBMENU_WB_TITLE, 	 //line 85
	LANG_LABEL_SUBMENU_WB_CONTENT0, 	 //line 86
	LANG_LABEL_SUBMENU_WB_CONTENT1, 	 //line 87
	LANG_LABEL_SUBMENU_WB_CONTENT2, 	 //line 88
	LANG_LABEL_SUBMENU_WB_CONTENT3, 	 //line 89
	LANG_LABEL_SUBMENU_WB_CONTENT4, 	 //line 90
	LANG_LABEL_SUBMENU_SHUTDOWN_TITLE, 	 //line 85
	LANG_LABEL_SUBMENU_SHUTDOWN_CONTENT0, 	 //line 86
	LANG_LABEL_SUBMENU_SHUTDOWN_CONTENT1, 	 //line 87
	LANG_LABEL_SUBMENU_SHUTDOWN_CONTENT2, 	 //line 88
	LANG_LABEL_SUBMENU_SHUTDOWN_CONTENT3, 	 //line 89
	LANG_LABEL_SUBMENU_CONTRAST_TITLE, 	 //line 91
	LANG_LABEL_SUBMENU_CONTRAST_CONTENT0, 	 //line 92
	LANG_LABEL_SUBMENU_CONTRAST_CONTENT1, 	 //line 93
	LANG_LABEL_SUBMENU_CONTRAST_CONTENT2, 	 //line 94
	LANG_LABEL_SUBMENU_CONTRAST_CONTENT3, 	 //line 95
	LANG_LABEL_SUBMENU_CONTRAST_CONTENT4, 	 //line 96
	LANG_LABEL_SUBMENU_CONTRAST_CONTENT5, 	 //line 97
	LANG_LABEL_SUBMENU_EXPOSURE_TITLE, 	 //line 98
	LANG_LABEL_SUBMENU_EXPOSURE_CONTENT0, 	 //line 99
	LANG_LABEL_SUBMENU_EXPOSURE_CONTENT1, 	 //line 100
	LANG_LABEL_SUBMENU_EXPOSURE_CONTENT2, 	 //line 101
	LANG_LABEL_SUBMENU_EXPOSURE_CONTENT3, 	 //line 102
	LANG_LABEL_SUBMENU_EXPOSURE_CONTENT4, 	 //line 103
	LANG_LABEL_SUBMENU_EXPOSURE_CONTENT5, 	 //line 104
	LANG_LABEL_SUBMENU_EXPOSURE_CONTENT6, 	 //line 105
	LANG_LABEL_SUBMENU_OK, 	 //line 106
	LANG_LABEL_SUBMENU_CANCEL, 	 //line 107
	LANG_LABEL_SUBMENU_FORMAT_TITLE, 	 //line 108
	LANG_LABEL_SUBMENU_FORMAT_TEXT, 	 //line 109
	LANG_LABEL_SUBMENU_FORMATTING_TEXT, 	 //line 110
	LANG_LABEL_SUBMENU_FRESET_TITLE, 	 //line 111
	LANG_LABEL_SUBMENU_FRESET_TEXT, 	 //line 112
	LANG_LABEL_SCREEN_SHOT, 	 //line 113
	LANG_LABEL_WARNING, 	 //line 114
	LANG_LABEL_TIPS, 	 //line 115
	LANG_LABEL_NO_TFCARD, 	 //line 116
	LANG_LABEL_TFCARD_FULL, 	 //line 117
	LANG_LABEL_FORMATTING, 	 //line 118
	LANG_LABEL_DATE_TITLE, 	 //line 119
	LANG_LABEL_DATE_YEAR, 	 //line 120
	LANG_LABEL_DATE_MONTH, 	 //line 121
	LANG_LABEL_DATE_DAY, 	 //line 122
	LANG_LABEL_DATE_HOUR, 	 //line 123
	LANG_LABEL_DATE_MINUTE, 	 //line 124
	LANG_LABEL_DATE_SECOND, 	 //line 125
	LANG_LABEL_DELETEFILE_TITLE, 	 //line 126
	LANG_LABEL_DELETEFILE_TEXT, 	 //line 127
	LANG_LABEL_SHUTDOWN_TITLE, 	 //line 128
	LANG_LABEL_SHUTDOWN_TEXT, 	 //line 129
	LANG_LABEL_OPEN_USB_STORAGE_DEVICE, 	 //line 130
	LANG_LABEL_CLOSE_USB_STORAGE_DEVICE, 	 //line 131
	LANG_LABEL_CHARGE_MODE, 	 //line 132
	LANG_LABEL_POWER_CHARGING_FULL,
	LANG_LABEL_LOW_POWER_SHUTDOWN, 	 //line 133
	LANG_LABEL_10S_SHUTDOWN, 	 //line 134
	LANG_LABEL_SHUTDOWN_NOW, 	 //line 135
	LANG_LABEL_30S_NOWORK_SHUTDOWN, 	 //line 136
	LANG_LABEL_NOWORK_ENTER_STANDBY,
	LANG_LABEL_PLAYBACK_FAIL, 	 //line 137
	LANG_LABEL_FILELIST_EMPTY, 	 //line 138
	LANG_LABEL_SOS_WARNING, 	 //line 139
	LANG_LABEL_WIFI_CONNECT, 	 //line 140
	LANG_LABEL_WIFI_DISCONNECT, 	 
	LANG_LABEL_WIFI_CONNECTING, 	 
	LANG_LABEL_WIFI_INFORMATION, 	 
	LANG_LABEL_WIFI_SSID, 	 
	LANG_LABEL_WIFI_PASSWORD, 
	LANG_LABEL_WIFI_PROMPT, 
	LANG_LABEL_IMPACT_NUM, 	 //line 141
	LANG_LABEL_OTA_UPDATE, 	 //line 142
	LANG_LABEL_TAKE_PICTURE, 	 //line 143
	LANG_LABEL_FORMAT_SDCARD, 	 //line 144
	LANG_LABEL_FILE_LOCKED, 	 //line 145
	LANG_LABEL_LOCKED_FILE, 	 //line 146
	LANG_LABEL_UNLOCK_FILE, 	 //line 147
	LANG_LABEL_LICENSE_PLATE_NUMBERS_CN, 	 //line 148
	LANG_LABEL_LICENSE_PLATE_NUMBERS_EN, 	 //line 149
	LANG_LABEL_OPEN_PCCAM, 	 //line 150
	LANG_LABEL_INSERTED_SDCARD, 	 //line 151
	LANG_LABEL_EXTRACT_SDCARD, 	 //line 152
	LANG_LABEL_CARD_SPEED_TEST,
	LANG_LABEL_CARD_READ_WRITE,
	LANG_LABEL_15S_SHUTDOWN, 	 //line 158
	LANG_LABEL_30S_SHUTDOWN, 	 //line 159
	LANG_LABEL_60S_SHUTDOWN, 	 //line 160
	LANG_LABEL_CANCEL_SHUTDOWN, 	 //line 161
	LANG_LABEL_FORMAT_TFCARD_FINISH,
	LANG_LABEL_FORMATING_TFCARD,
	LANG_LABEL_SUBMENU_DELAY_SHUTDOWN_CONTENT0,
	LANG_LABEL_FIRMWARE_VERSION,
	LANG_LABEL_PRODUCT_TYPE,
	LANG_LABEL_SOFTWARE_VERSION,
	LANG_LABEL_UPDATED,
	LANG_LABEL_MANUFACTURER,
	LANG_LABEL_AITEWEI
};

class cdrLang
{
public:
	cdrLang();
	virtual ~cdrLang(){}
	virtual int initLangAndFont(LANGUAGE language) = 0;

	int initLabel(LANGUAGE language);
	const char* getLabel(unsigned int labelIndex);

	LANGUAGE lang;
	PLOGFONT mLogFont;
	PLOGFONT mLogFont_other;
	PLOGFONT mLogFont_ch;

private:
	std::vector <std::string> label;
	std::map <LANGUAGE, std::string> mLangFileTable;
	int loadLabelFromLangFile(std::string langFile);
};

#endif
