#ifndef _RECORD_PREVIEW_H
#define _RECORD_PREVIEW_H

#include <time.h>
#include "windows.h"
#include "keyEvent.h"
#include "cdr_message.h"
#include "ResourceManager.h"
#include "Dialog.h"
#include "wifi_tip.h"

#define BIG_RECT_W 854
#define BIG_RECT_H 480
#define BIG_RECT_X 0
#define BIG_RECT_Y 0
static int index_sdv = 0;


#define SML_RECT_W (BIG_RECT_W*2/5)
#define SML_RECT_H (SML_RECT_W*BIG_RECT_H/BIG_RECT_W)
#define SML_RECT_X (BIG_RECT_W - SML_RECT_W)
#define SML_RECT_Y (30)


#define LOCAL2SESSION(idx) (idx+CAM_CNT)
#define SESSION2LOCAL(idx) (idx-CAM_CNT)
#define CAM_OTHER(idx) (1-idx)

#define Max_Capacity_Support			(4*1024)		//MB 	

#define MAX_FILE_SIZE			3375	//MB
#define MIN_AVILIABLE_SPCE		50		//MB

#define ID_TIME_TAKE_PHOTO_KEY 102

static int motion_photo_conts = 0;



enum eRecordTimeLength {
	RTL_0 = 2,
	RTL_1 = 3,
	RTL_2 = 5,
	RTL_AVAILABLE=0xff,  
};

enum autoPhotoTimeLength {
	APTL_OFF   = 0,
	APTL_3     = 3,
	APTL_10   = 10,
	APTL_15   = 15, 
	APTL_20   = 20,
	APTL_30   = 30,
};

enum PhotoTimeLength {
	PTL_OFF   = 0,
	PTL_3       = 3,
	PTL_5       = 5,
	PTL_10     = 10, 
	PTL_20     = 20,
	
};

enum motionPhotoTimeLength {
	mPTL_OFF   = 0,
	mPTL_3       = 3,
	mPTL_5       = 5,
	mPTL_10     = 10,
};





#endif
