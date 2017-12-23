

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ResourceManager.h"
#include "misc.h"

#include "licensePlateWM.h"
#include "cdr_widgets.h"
#include "keyEvent.h"
#include "cdr_message.h"



#undef LOG_TAG
#define LOG_TAG "licensePlateWM"
#include "debug.h"

enum FocusArea {
	FOCUS_AREA_SWITCH = 1,		// can close and open licensePlateWM
	FOCUS_AREA_NUMBER,			// can select licensePlate number
};
typedef struct _license_plate_ctrl_info{
	char licensePlateNumber[LICENSE_PLATE_NUMBER_COUNT][10];
	int curSel;		
	BOOL enableEdit;
	enum FocusArea focusArea;
	const char* enableText;
	const char* disableText;
	PBITMAP pBmpEnableChioce;
	PBITMAP pBmpDisableChioce;
	PBITMAP pBmpArrow;
	int curNumberPos;
	int platePosData[LICENSE_PLATE_NUMBER_COUNT];
	char candidateEnNumbers[CANDICATE_BUFFER_LEN + 1];
	char candidateCnNumbers[CANDICATE_BUFFER_LEN + 1];
	int enNumberCnt;
	int cnNumberCnt;
	int enNumberPositions[CANDICATE_BUFFER_LEN + 1];
	int cnNumberPositions[CANDICATE_BUFFER_LEN + 1];
	LANGUAGE language;
}LicensePlate_CtrlInfo_t;


char wmString[10]={0};

menuPlate_t menuPlate;
GHANDLE lCfgMenuHandle;



enum localWidgetsId {
	ID_TITLE_IMAGE = 100,
	ID_TITLE_TEXT,
	ID_ENABLE_SWITCH_TEXT,
	ID_ENABLE_SWITCH_IMAGE,
	ID_NUMBER_START,
	ID_NUMBER_END = ID_NUMBER_START + LICENSE_PLATE_NUMBER_COUNT - 1,
};
#define ID2Index(id) (id - 100)

static CTRLDATA ctrlData[4 + 8] = {
	// title
	{
		CTRL_STATIC,
		WS_CHILD | WS_VISIBLE | SS_BITMAP |SS_CENTERIMAGE,
		8, 4, 32, 28,
		ID_TITLE_IMAGE,
		"",
		0,
		WS_EX_TRANSPARENT,
		NULL, NULL
	},
	{
		CTRL_STATIC,
		WS_CHILD | WS_VISIBLE | SS_SIMPLE,
		48, 8, 100, 16,
		ID_TITLE_TEXT,
		"",
		0,
		WS_EX_TRANSPARENT | WS_EX_USEPARENTFONT,
		NULL, NULL
	},

	// enable choice
	{
		CTRL_STATIC,
		WS_CHILD | WS_VISIBLE | SS_SIMPLE,
		48, 44, 100, 16,
		ID_ENABLE_SWITCH_TEXT,
		"",
		0,
		WS_EX_TRANSPARENT | WS_EX_USEPARENTFONT,
		NULL, NULL
	},

	{
		CTRL_STATIC,
		WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTERIMAGE,
		240, 40, 32, 28,
		ID_ENABLE_SWITCH_IMAGE,
		"",
		0,
		WS_EX_TRANSPARENT,
		NULL, NULL
	},
};


void getMenuCfgValue(menuPlate_t* MenuValue)
{
	ResourceManager* rm;
	rm = ResourceManager::getInstance();

	MenuValue->current = rm->menuDataLWM.LWMEnable ;
	strncpy(MenuValue->NumStr , rm->menuDataLWM.lwaterMark ,10 );

}

void setLocation(int screenW,int screenH)
{
	getScreenInfo(&screenW, &screenH);
	PCTRLDATA location = ctrlData + 0;
	location->x = screenW/40;
	location->y = screenH/40;
	location->w = screenW/10;
	location->h = screenH*0.075;
	db_msg("x:%d,y:%d,w:%d,h:%d",location->x,location->y,location->w,location->h);
	
	location = ctrlData + 1;
	location->x = screenW/5;
	location->y = ctrlData[0].y*2;
	location->w = screenW/2;
	location->h = screenH/20;
	db_msg("x:%d,y:%d,w:%d,h:%d",location->x,location->y,location->w,location->h);

	location = ctrlData + 2;
	location->x = ctrlData[1].x;
	location->y = screenH*0.1875;
	location->w = screenW/2;
	location->h = screenH/20;
	db_msg("x:%d,y:%d,w:%d,h:%d",location->x,location->y,location->w,location->h);
	
	location = ctrlData + 3;
	location->x = screenW*0.75;
	location->y = screenH/6;
	location->w = screenW/10;
	location->h = screenH*0.075;
	db_msg("x:%d,y:%d,w:%d,h:%d",location->x,location->y,location->w,location->h);

}



PCTRLDATA getCtrlData(int ctrlId)
{
	if(ID2Index(ctrlId) < TABLESIZE(ctrlData) ) {
		//return ctrlData + ID2Index(ctrlId);
		return &ctrlData[ID2Index(ctrlId)];
	}

	return NULL;
}

static void draw_frame_lines(HWND hDlg, HDC hdc)
{
	RECT rect1, rect2;
	gal_pixel lineColor;

	GetClientRect(hDlg, &rect1);
	GetWindowRect(GetDlgItem(hDlg, ID_TITLE_IMAGE), &rect2);

	lineColor = RGBA2Pixel(hdc, 0x42, 0xCE, 0xFF, 0xFF);
	SetPenColor(hdc, lineColor);

	MoveTo(hdc, 0, rect2.bottom + 16);
	LineTo(hdc, RECTW(rect1), rect2.bottom + 16);


	GetWindowRect(GetDlgItem(hDlg, ID_ENABLE_SWITCH_IMAGE), &rect2);
	MoveTo(hdc, 0, rect2.bottom + 16);
	LineTo(hdc, RECTW(rect1), rect2.bottom + 16);

	GetWindowRect(GetDlgItem(hDlg, ID_NUMBER_START), &rect2);
	MoveTo(hdc, 0, rect2.bottom + 28);
	LineTo(hdc, RECTW(rect1), rect2.bottom + 28);
}

static void draw_hilighted_item(HWND hDlg, HDC hdc)
{
	LicensePlate_CtrlInfo_t* ctrlInfo;
	HWND hwndWidget;
	int prevSel;
	RECT rect;
	int x, y, w, h;
	gal_pixel hilightColor, normalColor;

	hilightColor = RGBA2Pixel(HDC_SCREEN, 0xDC, 0xDC, 0xDC, 0xFF);

//	hilightColor = RGBA2Pixel(HDC_SCREEN, 0x00, 0xFF, 0x99, 0xFF);
	normalColor = RGBA2Pixel(HDC_SCREEN, 0x99, 0x99, 0x99, 0xFF);
	ctrlInfo = (LicensePlate_CtrlInfo_t*)GetWindowAdditionalData(hDlg);

	if(ctrlInfo != NULL) {
		if(ctrlInfo->curSel >= 0) {
			hwndWidget = GetDlgItem(hDlg, ID_NUMBER_START + ctrlInfo->curSel);
			GetWindowRect(hwndWidget, &rect);
			w = ctrlInfo->pBmpArrow->bmWidth;
			h = ctrlInfo->pBmpArrow->bmHeight;
			x = rect.left;
			y = rect.top - h - 4;
			if(w < RECTW(rect)) {
				x = rect.left + ( (RECTW(rect) - w) >> 1 );
			}
			FillBoxWithBitmap(hdc, x, y, w, h, ctrlInfo->pBmpArrow);
			if(hwndWidget && hwndWidget != HWND_INVALID) {
				SetWindowBkColor(hwndWidget, hilightColor );
			}

			prevSel = ctrlInfo->curSel - 1;
			if(prevSel >= 0) {
				hwndWidget = GetDlgItem(hDlg, ID_NUMBER_START + prevSel );
				if(hwndWidget && hwndWidget != HWND_INVALID) {
					SetWindowBkColor(hwndWidget, normalColor );
				}
			}
		} else {
			hwndWidget = GetDlgItem(hDlg, ID_NUMBER_END );
			if(hwndWidget && hwndWidget != HWND_INVALID) {
				SetWindowBkColor(hwndWidget, normalColor );
			}
		}
	}
}
static void draw_Selected_item(HWND hDlg, HDC hdc)
{
	LicensePlate_CtrlInfo_t* ctrlInfo;
	gal_pixel lineColor;
	ctrlInfo = (LicensePlate_CtrlInfo_t*)GetWindowAdditionalData(hDlg);
	if(ctrlInfo->focusArea == FOCUS_AREA_SWITCH)
		lineColor = RGBA2Pixel(hdc, 0x42, 0xCE, 0xFF, 0xFF);
	else
		lineColor = RGBA2Pixel(hdc, 0x39, 0x39, 0x39, 0xFF);
	SetPenColor(hdc, lineColor);
	for(int i = 0 ; i < 62 ; i++){
		Line2(hdc , 5 , 36+i+30, 275 , 36+i+30);
	}
#if 0
	if(ctrlInfo->focusArea == FOCUS_AREA_NUMBER)
		lineColor = RGBA2Pixel(hdc, 0x42, 0xCE, 0xFF, 0xFF);
	else
		lineColor = RGBA2Pixel(hdc, 0x39, 0x39, 0x39, 0xFF);
	SetPenColor(hdc, lineColor);
	for(int i = 0 ; i < 72 ; i++){		
		Line2(hdc , 5 , 72+i , 275 , 72+i );
	}
#endif
}

static int getCharactorCount(HWND hDlg, const char* spText, int len)
{
	return GetTextMCharInfo( GetWindowFont(hDlg), spText, len, NULL);
}

static void myTextOut(HWND hDlg, HDC hdc, int x, int y, int hilight_index)
{
	int count;
	int *position;
	int startx = x;
	const char* spText;
	int curSel;
	RECT rect;
	POINT point;
	gal_pixel hilightColor, normalColor;
	LicensePlate_CtrlInfo_t* ctrlInfo;

	ctrlInfo = (LicensePlate_CtrlInfo_t*)GetWindowAdditionalData(hDlg);
	if(ctrlInfo == NULL)
		return;

	curSel = ctrlInfo->curSel;
	if(ctrlInfo->language == LANG_CN || ctrlInfo->language == LANG_TW) {
		if(curSel == 0) {
			count = ctrlInfo->cnNumberCnt;
			position = ctrlInfo->cnNumberPositions;
			spText = ctrlInfo->candidateCnNumbers;
		} else {
			count = ctrlInfo->enNumberCnt;
			position = ctrlInfo->enNumberPositions;
			spText = ctrlInfo->candidateEnNumbers;
		}
	} else {
		count = ctrlInfo->enNumberCnt;
		position = ctrlInfo->enNumberPositions;
		spText = ctrlInfo->candidateEnNumbers;
	}
//	db_msg("charactor count is %d, hilight_index is %d\n", count, hilight_index);

	if(hilight_index >= count) {
		return;
	}

	GetClientRect(hDlg, &rect);
	hilightColor = RGBA2Pixel(HDC_SCREEN, 0xFF, 0xDC, 0xDC, 0xDC);
	normalColor = RGBA2Pixel(HDC_SCREEN, 0x00, 0x00, 0x00, 0xFF);

	int len;
	for(int index = 0; index < count; index++) {
		if(index < count - 1) {
			len = position[index + 1] - position[index];
		} else {
			len = strlen(spText) - position[index];
		}
//		db_msg("x: %d, y: %d, positon[%d]=%d, len=%d\n", x, y, index, position[index], len);

		if(index == hilight_index) {
			SetTextColor(hdc, hilightColor );
			if(curSel >= 0 && curSel < LICENSE_PLATE_NUMBER_COUNT) {
				strncpy(ctrlInfo->licensePlateNumber[curSel], spText + position[index], len);
				//show licensePlate number
				SetWindowText(GetDlgItem(hDlg, ID_NUMBER_START + curSel), ctrlInfo->licensePlateNumber[curSel] );

			}
		} else {
			SetTextColor(hdc, normalColor );
		}

		TextOutLen(hdc, x, y, spText + position[index], len);

		GetLastTextOutPos(hdc, &point);
		x = point.x;

		if(x >= (RECTW(rect)-10) - startx) {
			x = startx;
			y = point.y + 5;
		}
	}
}


void draw_default_plate(HWND hDlg, HDC hdc )
{
	db_msg(">>>>>>draw default plate\n");
	if(menuPlate.current){
		char tmpStr[10][10]={0};
		strncpy( tmpStr[0]  , menuPlate.NumStr , 3);
		SetWindowText(GetDlgItem(hDlg, ID_NUMBER_START ), tmpStr[0] );

		for(int i=0; i<7 ; i++)
		{
			strncpy( tmpStr[i+1] , &(menuPlate.NumStr[i+3]) , 1);
			SetWindowText(GetDlgItem(hDlg, ID_NUMBER_START + i + 1), tmpStr[i+1] );
		}
	}else{
		for(int i=0; i<8 ; i++)
		{
			SetWindowText(GetDlgItem(hDlg, ID_NUMBER_START + i ), NULL );
		}
	}
}



static void draw_candicate_numbers(HWND hDlg, HDC hdc)
{
	LicensePlate_CtrlInfo_t* ctrlInfo;
	RECT rect;
	int x, y;
	int hilight_pos;

	ctrlInfo = (LicensePlate_CtrlInfo_t*)GetWindowAdditionalData(hDlg);
	if (ctrlInfo)
		hilight_pos = ctrlInfo->curNumberPos;
	GetClientRect(hDlg, &rect);

	if(ctrlInfo != NULL && ctrlInfo->curSel >= 0) {
		x = 5;
		y = RECTH(rect) - 50;

		db_msg("hilight position is %d\n", hilight_pos);
		if(hilight_pos > 0) {
			myTextOut(hDlg, hdc, x, y, hilight_pos);
		} else {
			myTextOut(hDlg, hdc, x, y, 0);
		}
	}
}




void syncLWM2Res(HWND hDlg, HDC hdc)
{
	LicensePlate_CtrlInfo_t* ctrlInfo;
	char buf[3]={0};

	ResourceManager* rm;
	rm = ResourceManager::getInstance();
	ctrlInfo = (LicensePlate_CtrlInfo_t*)GetWindowAdditionalData(hDlg);

	rm->menuDataLWM.LWMEnable = menuPlate.current ;
	char LWM[10]={0};
	int tmp=strlen(ctrlInfo->licensePlateNumber[0]);
	strncpy(LWM,(ctrlInfo->licensePlateNumber[0]),tmp);
	for(int j= 0 ; j< 7 ; j++){
		strncpy(&LWM[j+tmp] , (ctrlInfo->licensePlateNumber[j+1]) , 1 );
	}

//	if(menuPlate.current && (LWM[9] != 0) )
	if(menuPlate.current ){
		strncpy(menuPlate.NumStr , LWM ,10);
		strncpy(rm->menuDataLWM.lwaterMark, LWM ,10);
	}
	db_msg(">>>>>>>cpy 2 rm->menuDataLWM>>>>>>LWM = %s",LWM);

}



static void syncAndExit(HWND hDlg)
{
	ResourceManager* rm;
	rm = ResourceManager::getInstance();

	syncLWM2Res(hDlg,0);
	if(rm->setResBoolValue(ID_MENU_LIST_LWM,rm->menuDataLWM.LWMEnable) < 0) {
		db_error("set ID_MENU_LIST_LWM / menuDataLWM.LWMEnable failed\n");
	}
	EndDialog(hDlg, 0);
}

static int licensePlateWM_Proc(HWND hDlg, int message, WPARAM wParam, LPARAM lParam)  
{
	LicensePlate_CtrlInfo_t* ctrlInfo;
	switch(message) {
	case MSG_INITDIALOG:
		{
			int len;
			HWND hwndWidget;
			LicensePlateWM_data_t* dialogData;

			db_msg("MSG_INITDIALOG\n");
			dialogData = (LicensePlateWM_data_t*)lParam;
			SetWindowFont( hDlg, dialogData->pLogFont);
			SetWindowFont( GetDlgItem(hDlg, ID_TITLE_TEXT), dialogData->pLogFont);
			SetWindowFont( GetDlgItem(hDlg, ID_ENABLE_SWITCH_TEXT), dialogData->pLogFont);
			SetWindowBkColor(hDlg, RGBA2Pixel(HDC_SCREEN, 0x39, 0x39, 0x39, 0xFF));

			for( int i = 0, ctrlId = ID_NUMBER_START; i < LICENSE_PLATE_NUMBER_COUNT; i++ ) {
				hwndWidget = GetDlgItem(hDlg, ctrlId + i);
				if(hwndWidget && hwndWidget != HWND_INVALID) {
					SetWindowFont(hwndWidget, dialogData->pLogFont);
					SetWindowBkColor(hwndWidget, RGBA2Pixel(HDC_SCREEN, 0x99, 0x99, 0x99, 0xFF));
				}
			}
			ctrlInfo = (LicensePlate_CtrlInfo_t*)malloc(sizeof(LicensePlate_CtrlInfo_t));
			memset(ctrlInfo, 0, sizeof(LicensePlate_CtrlInfo_t));
			ctrlInfo->curSel = -1;
			ctrlInfo->curNumberPos = 0;
			if(menuPlate.current)
				ctrlInfo->enableEdit = FALSE ;
			else
				ctrlInfo->enableEdit = TRUE ;
			ctrlInfo->focusArea = FOCUS_AREA_SWITCH;
			if(dialogData != NULL) {
				ctrlInfo->enableText = dialogData->enableText;
				ctrlInfo->disableText = dialogData->disableText;
				ctrlInfo->pBmpEnableChioce = dialogData->pBmpEnableChioce;
				ctrlInfo->pBmpDisableChioce = dialogData->pBmpDisableChioce;
				ctrlInfo->pBmpArrow = dialogData->pBmpArrow;

				len = strnlen(dialogData->candidateEnNumbers, CANDICATE_BUFFER_LEN);
				strncpy(ctrlInfo->candidateEnNumbers, dialogData->candidateEnNumbers, len );
				ctrlInfo->enNumberCnt = getCharactorCount(hDlg, ctrlInfo->candidateEnNumbers, len );
				GetTextMCharInfo( dialogData->pLogFont, ctrlInfo->candidateEnNumbers, len, ctrlInfo->enNumberPositions);
				db_msg("enNumberCnt is %d\n", ctrlInfo->enNumberCnt);

				len = strnlen(dialogData->candidateCnNumbers, CANDICATE_BUFFER_LEN);
				strncpy(ctrlInfo->candidateCnNumbers, dialogData->candidateCnNumbers, len );
				ctrlInfo->cnNumberCnt = getCharactorCount(hDlg, ctrlInfo->candidateCnNumbers, len );
				GetTextMCharInfo( dialogData->pLogFont, ctrlInfo->candidateCnNumbers, len, ctrlInfo->cnNumberPositions);
				db_msg("cnNumberCnt is %d\n", ctrlInfo->cnNumberCnt);

				ctrlInfo->language = dialogData->language;
				strncpy((ctrlInfo->licensePlateNumber[0]) , menuPlate.NumStr , 3);
				for(int j= 0 ; j< 7 ; j++){
					strncpy((ctrlInfo->licensePlateNumber[j+1]) , &menuPlate.NumStr[3+j] , 1 );		
				}
			}

			SetWindowAdditionalData(hDlg, (DWORD)ctrlInfo);

			//my_test

		}
		break;
	case MSG_PAINT:
		{
#if 1
			ctrlInfo = (LicensePlate_CtrlInfo_t*)GetWindowAdditionalData(hDlg);

			db_msg("MSG_PAINT\n");
			HDC hdc;

			hdc = BeginPaint(hDlg);
		
			draw_frame_lines(hDlg, hdc);
			draw_Selected_item(hDlg, hdc);

			draw_hilighted_item(hDlg, hdc);

			draw_candicate_numbers(hDlg, hdc);

			if(ctrlInfo->curSel < 0){
				syncLWM2Res(hDlg,0);
				draw_default_plate(hDlg, hdc);
			}
			EndPaint(hDlg, hdc);
#endif
		}	
		break;

	case MSG_KEYDOWN:
		{
			ctrlInfo = (LicensePlate_CtrlInfo_t*)GetWindowAdditionalData(hDlg);

			db_msg("keycode is %d\n", wParam);
			switch(wParam) {
			case CDR_KEY_OK:
				{
					if( ctrlInfo->focusArea == FOCUS_AREA_SWITCH ){
						db_msg("in FOCUS_AREA_SWITCH\n");
						if(ctrlInfo->enableEdit == TRUE){
							menuPlate.current = 1 ;
							ctrlInfo->enableEdit = FALSE;
							db_msg("set enable text\n");
							SetWindowText(GetDlgItem(hDlg, ID_ENABLE_SWITCH_TEXT), ctrlInfo->enableText);
							SendMessage(GetDlgItem(hDlg, ID_ENABLE_SWITCH_IMAGE), STM_SETIMAGE, (WPARAM)ctrlInfo->pBmpEnableChioce, 0 );
						}else {
							menuPlate.current = 0 ;
							db_msg("set disable text\n");
							ctrlInfo->enableEdit = TRUE;
							SetWindowText(GetDlgItem(hDlg, ID_ENABLE_SWITCH_TEXT), ctrlInfo->disableText);
							SendMessage(GetDlgItem(hDlg, ID_ENABLE_SWITCH_IMAGE), STM_SETIMAGE, (WPARAM)ctrlInfo->pBmpDisableChioce, 0 );
						}
					}else if(ctrlInfo->focusArea == FOCUS_AREA_NUMBER) {
						if(ctrlInfo->curSel < 0) {
							ctrlInfo->curSel = 0;
						} else {
							ctrlInfo->curSel++;
							if(ctrlInfo->curSel >= LICENSE_PLATE_NUMBER_COUNT) {
								ctrlInfo->curSel = -1;
								ctrlInfo->focusArea = FOCUS_AREA_SWITCH;
								syncAndExit(hDlg);
							}
						}
						db_msg("in FOCUS_AREA_NUMBER, curSel is %d\n", ctrlInfo->curSel);
						InvalidateRect(hDlg, NULL, TRUE);
					}

					ctrlInfo->platePosData[ctrlInfo->curSel] = ctrlInfo->curNumberPos ;
				}
				break;
			case CDR_KEY_LEFT:
				{
					int count;
					db_msg(">>>>>>>>CDR_KEY_LEFT");
					if(ctrlInfo->focusArea == FOCUS_AREA_NUMBER) {
						if(ctrlInfo->language == LANG_CN || ctrlInfo->language == LANG_TW) {
							if(ctrlInfo->curSel == 0) {
								count = ctrlInfo->cnNumberCnt;
							} else {
								count = ctrlInfo->enNumberCnt;
							}
						} else {
							count = ctrlInfo->enNumberCnt;
						}

						ctrlInfo->curNumberPos--;
						if(ctrlInfo->curNumberPos <  0 ) {
							ctrlInfo->curNumberPos = count - 1;
						}
						db_msg("count is %d, curNumberPos is %d\n", count, ctrlInfo->curNumberPos);
						InvalidateRect(hDlg, NULL, TRUE);
					//------------------
					ctrlInfo->platePosData[ctrlInfo->curSel] = ctrlInfo->curNumberPos ;
					//------------------
					}
					else if((ctrlInfo->focusArea == FOCUS_AREA_SWITCH) && menuPlate.current){
						db_msg(">>>>>>>>ctrlInfo->focusArea == FOCUS_AREA_NUMBER  ;");
						if(ctrlInfo->curSel < 0) {
							ctrlInfo->curSel = 0;
						}
						ctrlInfo->focusArea = FOCUS_AREA_NUMBER ;
						InvalidateRect(hDlg, NULL, TRUE);
					}
				}
				break;
			case CDR_KEY_RIGHT:
				{
					int count;
					db_msg(">>>>>>>>CDR_KEY_RIGHT");
					if(ctrlInfo->focusArea == FOCUS_AREA_NUMBER) {
						if(ctrlInfo->language == LANG_CN || ctrlInfo->language == LANG_TW) {
							if(ctrlInfo->curSel == 0) {
								count = ctrlInfo->cnNumberCnt;
							} else {
								count = ctrlInfo->enNumberCnt;
							}
						} else {
							count = ctrlInfo->enNumberCnt;
						}

						ctrlInfo->curNumberPos++;
						if(ctrlInfo->curNumberPos >= count ) {
							ctrlInfo->curNumberPos = 0;
						}
						InvalidateRect(hDlg, NULL, TRUE);
						//------------------
						ctrlInfo->platePosData[ctrlInfo->curSel] = ctrlInfo->curNumberPos ;
						//------------------
					}
					else if((ctrlInfo->focusArea == FOCUS_AREA_SWITCH) && menuPlate.current){
						db_msg(">>>>>>>>ctrlInfo->focusArea == FOCUS_AREA_NUMBER ;");
						if(ctrlInfo->curSel < 0) {
							ctrlInfo->curSel = 0;
						}
						ctrlInfo->focusArea = FOCUS_AREA_NUMBER ;
						InvalidateRect(hDlg, NULL, TRUE);
					}
				}
				break;
			case CDR_KEY_MENU:
			case CDR_KEY_MODE:
				{
					syncAndExit(hDlg);
				}
				break;
			case MSG_CLOSE_TIP_LABEL: {
					syncAndExit(hDlg);
				}
			break;
			}
		}
		break;
	case MSG_DESTROY:
		{
			db_msg("msg destroy");
			ctrlInfo = (LicensePlate_CtrlInfo_t *)GetWindowAdditionalData(hDlg);
			if(ctrlInfo)
			free(ctrlInfo);
		}
		break;
	default:
		break;
	}

	return DefaultDialogProc (hDlg, message, wParam, lParam);
}



int licensePlateWM_Dialog(HWND hParent, LicensePlateWM_data_t* dialogData)
{
	DLGTEMPLATE dlg;
	PCTRLDATA pCtrlData;
	int i;

	getMenuCfgValue(&menuPlate);
	int screenW,screenH;
	getScreenInfo(&screenW, &screenH);
	setLocation(screenW,screenH);

	pCtrlData = getCtrlData(ID_TITLE_TEXT);
	if(pCtrlData != NULL) {
		pCtrlData->caption = dialogData->titleText;
	}
	pCtrlData = getCtrlData(ID_ENABLE_SWITCH_TEXT);
	if(pCtrlData != NULL) {
		if(menuPlate.current)
			pCtrlData->caption = dialogData->enableText;
		else
			pCtrlData->caption = dialogData->disableText;
	}

	pCtrlData = getCtrlData(ID_TITLE_IMAGE);
	if(pCtrlData != NULL) {
		pCtrlData->dwAddData = (DWORD)dialogData->pBmpTitle;
	}
	pCtrlData = getCtrlData(ID_ENABLE_SWITCH_IMAGE);
	if(pCtrlData != NULL) {
		if(menuPlate.current)
			pCtrlData->dwAddData = (DWORD)dialogData->pBmpEnableChioce;
		else
			pCtrlData->dwAddData = (DWORD)dialogData->pBmpDisableChioce;
	}

	int ctrlId = ID_NUMBER_START;
	for( i = 0; i < LICENSE_PLATE_NUMBER_COUNT; i++) {
		pCtrlData = getCtrlData(ctrlId + i);
		if(pCtrlData != NULL) {
			pCtrlData->class_name = CTRL_STATIC;
			pCtrlData->dwStyle = WS_VISIBLE | SS_CENTER | SS_VCENTER | SS_GRAYFRAME;
			pCtrlData->x = 22 + (22 + 8) * i;
			pCtrlData->y = 154;
			pCtrlData->w = 24;
			pCtrlData->h = 50;
			pCtrlData->id	= ctrlId + i;
			pCtrlData->caption = "";
			pCtrlData->dwAddData = 0;
			pCtrlData->dwExStyle = WS_EX_USEPARENTFONT;
			pCtrlData->werdr_name = NULL;
			pCtrlData->we_attrs = NULL;
		}
	}

	memset(&dlg, 0, sizeof(DLGTEMPLATE));
	dlg.dwStyle = WS_VISIBLE;
	dlg.dwExStyle = WS_EX_USEPARENTFONT;
	dlg.caption = "xxx";
	dlg.controlnr = TABLESIZE(ctrlData);
	dlg.controls = ctrlData;
	db_msg("contrlnr is %d\n", dlg.controlnr);

	dlg.x = screenW/16;
	dlg.y = screenH/12;
	dlg.w = screenW*0.875;
	dlg.h = screenH*0.8;

	return DialogBoxIndirectParam (&dlg, hParent, licensePlateWM_Proc, (LPARAM)dialogData);
}
