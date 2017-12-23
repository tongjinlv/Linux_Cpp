
#include "wifi_tip.h"
#undef LOG_TAG
#define LOG_TAG "wifi_tip.cpp"
#include "debug.h"
#include "ResourceManager.h"


#define IDC_TITLE	102
#define IDC_TEXT	103
#define ONE_SHOT_TIMER	202
#define TIMEOUT_TIMER	203

#define FULL_BLACK 0xFF000000

CTRLDATA ctrlData1[2] = {
	{
		CTRL_STATIC,
		WS_VISIBLE | SS_CENTER | SS_VCENTER,
		0, 0, 0, 0,
		IDC_TITLE,
		"",
		0,
		WS_EX_NONE | WS_EX_TRANSPARENT,
		NULL, NULL
	},

	{
		CTRL_STATIC,
		WS_VISIBLE | SS_LEFT,
		0, 0, 0, 0,
		IDC_TEXT,
		"",
		0,
		WS_EX_NONE | WS_EX_TRANSPARENT,
		NULL, NULL
	}
};

static BOOL timerCallback(HWND hDlg, int id, DWORD data)
{
	tipLabelData_t* tipLabelData;

	tipLabelData = (tipLabelData_t*)GetWindowAdditionalData(hDlg);

	db_msg("timerCallback\n");
	if(id == ONE_SHOT_TIMER) {
		db_msg("one shot timer timerCallback\n");
		if(tipLabelData->callback != NULL)
			(*tipLabelData->callback)(hDlg, tipLabelData->callbackData);
		db_msg("one shot timer timerCallback\n");
	} else if(id == TIMEOUT_TIMER) {
		db_msg("timeout timer timerCallback\n");
		SendNotifyMessage(hDlg, MSG_CLOSE_TIP_LABEL, 0, 0);
	}

	return FALSE;
}


static int DialogProc(HWND hDlg, int message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
	case MSG_INITDIALOG:
		{
			tipLabelData_t* tipLabelData;

			tipLabelData = (tipLabelData_t*)lParam;
			if(!tipLabelData) {
				db_error("invalid tipLabelData\n");
				return -1;
			}
			SetWindowAdditionalData(hDlg, (DWORD)tipLabelData);

			if(tipLabelData->pLogFont) {
				SetWindowFont(hDlg, tipLabelData->pLogFont);
			}
			if(tipLabelData->full_screen == 1){
				SetWindowBkColor(hDlg,FULL_BLACK);			
			}else{
				SetWindowBkColor(hDlg, tipLabelData->bgc_widget);			
			}
			if(tipLabelData->full_screen == 1){
				SetWindowElementAttr(GetDlgItem(hDlg, IDC_TITLE), WE_FGC_WINDOW, PIXEL2DWORD(HDC_SCREEN, FULL_BLACK) );
				SetWindowElementAttr(GetDlgItem(hDlg, IDC_TEXT), WE_FGC_WINDOW,  PIXEL2DWORD(HDC_SCREEN, FULL_BLACK) );
			}else{
				SetWindowElementAttr(GetDlgItem(hDlg, IDC_TITLE), WE_FGC_WINDOW, PIXEL2DWORD(HDC_SCREEN, tipLabelData->fgc_widget) );
				SetWindowElementAttr(GetDlgItem(hDlg, IDC_TEXT), WE_FGC_WINDOW,  PIXEL2DWORD(HDC_SCREEN, tipLabelData->fgc_widget) );
			}		
			SetTimerEx(hDlg, TIMEOUT_TIMER, tipLabelData->timeoutMs / 10, timerCallback);
		}
		break;
	case MSG_PAINT:
		{
			RECT rect;
			HDC hdc; 
			tipLabelData_t* tipLabelData;

			hdc = BeginPaint(hDlg);

			tipLabelData = (tipLabelData_t*)GetWindowAdditionalData(hDlg);
			GetClientRect(GetDlgItem(hDlg, IDC_TITLE), &rect);
			if(tipLabelData->full_screen == 1){
				SetPenColor(hdc, FULL_BLACK);
			}else{
				SetPenColor(hdc, tipLabelData->linec_title );	
			}
			Line2(hdc, 0, RECTH(rect) + 2, RECTW(rect), RECTH(rect) + 2);
			#if 0
			char *filepath=(char *)"/etc/res/others/wifi.png";
			LoadBitmapFromFile(HDC_SCREEN, &tipLabelData->bitImage, filepath);
			FillBoxWithBitmap(hdc,0,0,320,240,&tipLabelData->bitImage);		
			#endif
			//ShowWindow(hDlg,SW_SHOWNORMAL);
			EndPaint(hDlg, hdc);
		}
		break;
	case MSG_FONTCHANGED:
		{
			PLOGFONT pLogFont;		
			pLogFont = GetWindowFont(hDlg);
			if(pLogFont) {
				SetWindowFont(GetDlgItem(hDlg, IDC_TITLE), pLogFont);
				SetWindowFont(GetDlgItem(hDlg, IDC_TEXT), pLogFont);
			}
		}
		break;
	case MSG_KEYUP:
		{	
			switch(wParam) {
			case CDR_KEY_RIGHT:
				EndDialog(hDlg, 0);
				break;
			case CDR_KEY_LEFT:
			case CDR_KEY_OK:
			case CDR_KEY_MODE:
				break;
			}			
		}
		break;
	case MSG_KEYDOWN:
		{	
		
			//EndDialog(hDlg, 0);
		}
		break;
	case MSG_CLOSE_TIP_LABEL:	
		tipLabelData_t* tipLabelData;

		tipLabelData = (tipLabelData_t*)GetWindowAdditionalData(hDlg);
		db_info("MSG_CLOSE_LOWPOWER_DIALOG\n");
		if(IsTimerInstalled(hDlg, ONE_SHOT_TIMER) == TRUE) {
			KillTimer(hDlg, ONE_SHOT_TIMER);
		}
		if(IsTimerInstalled(hDlg, TIMEOUT_TIMER) == TRUE) {
			KillTimer(hDlg, TIMEOUT_TIMER);
		}
		if (tipLabelData->bitImage.bmBits != NULL){
			UnloadBitmap(&tipLabelData->bitImage);
		}
		EndDialog(hDlg, 0);
		break;
	}
	return DefaultDialogProc(hDlg, message, wParam, lParam);
}

int show_wifi_tip(HWND hParent)
{
	tipLabelData_t tipLabelData;
	ResourceManager* rm;
	int retval;
	rm = ResourceManager::getInstance();
	memset(&tipLabelData, 0, sizeof(tipLabelData));
	tipLabelData.title= "";
	tipLabelData.text="";	
	tipLabelData.timeoutMs= 0;
	retval = ShowWifiTip(hParent,&tipLabelData,1);	
	return retval;
}


int ShowWifiTip(HWND hParent, tipLabelData_t* info, int full_screen)
{
	DLGTEMPLATE dlg;
	CDR_RECT rect;

	if(info == NULL) {
		db_error("invalid info\n");
		return -1;
	}
	memset(&dlg, 0, sizeof(dlg));

	rect = info->rect;
	info->full_screen = full_screen;
	//if(full_screen) 
	{
		rect.x = 0;
		rect.y = 0;
		getScreenInfo(&rect.w,&rect.h);
		db_msg(" rect(%d %d %d %d)", rect.x, rect.y, rect.w, rect.h);
	}
	ctrlData1[0].x = 0;
	ctrlData1[0].y = 0;
	ctrlData1[0].w = rect.w;
	ctrlData1[0].h = info->titleHeight;
	ctrlData1[0].caption = info->title;

	//if (info->hasicon == true)
	{

		ctrlData1[1].x = 60;
		ctrlData1[1].w = rect.w - 60;
		ctrlData1[1].h = rect.h - 10 - ctrlData1[0].h;
		ctrlData1[1].y = ctrlData1[0].h +(ctrlData1[1].h/4);
		ctrlData1[1].caption = info->text;
		ctrlData1[1].dwStyle = WS_VISIBLE | SS_LEFT;
	}	
	ResourceManager	*rm = ResourceManager::getInstance();

	dlg.dwStyle = WS_VISIBLE;
	dlg.dwExStyle = WS_EX_NONE;
	dlg.x = rect.x;
	dlg.y = rect.y;
	dlg.w = rect.w;
	dlg.h =	rect.h;
	dlg.caption = "";
	dlg.hIcon = 0;
	dlg.hMenu = 0;
	dlg.controlnr = 2;
	dlg.controls = ctrlData1;
	dlg.dwAddData = 0;

	return DialogBoxIndirectParam(&dlg, hParent, DialogProc, (LPARAM)info);
}
