
#include "include/wifi_info.h"
#undef LOG_TAG
#define LOG_TAG "wifi_info.cpp"
#include "debug.h"
#include "ResourceManager.h"

static void SetBorderColor(HWND hDlg, unsigned int IDC, unsigned int select_flg)
{
	RECT rect;
	wifi_info_t* configData;
	HWND hCtrl;

	configData = (wifi_info_t*)GetWindowAdditionalData(hDlg);
	hCtrl = GetDlgItem(hDlg, IDC);

	HDC hdc = GetDC(hCtrl);

	GetClientRect(hCtrl, &rect);
	if (select_flg == SELECT_BORDER){
		SetPenColor(hdc, configData->borderc_selected );
	}
	else if (select_flg == UNSELECT_BORDER){
		SetPenColor(hdc,configData->borderc_normal);				
	}
	Rectangle (hdc, rect.left, rect.top, rect.right-1, rect.bottom-1);
	EndPaint(hCtrl, hdc);
}


static int WifiInfoProc(HWND hDlg, int message, WPARAM wParam, LPARAM lParam)  
{
	static unsigned int IDCSelected = IDC_WIFI_SSID;
	static unsigned long long tickCnt;
	wifi_info_t* configData;
	
	db_msg("message is %s,keycode is %d\n", Message2Str(message),wParam);
	switch(message) {
	case MSG_INITDIALOG:
		{
			ResourceManager * rm = ResourceManager::getInstance();
			configData = (wifi_info_t*)lParam;
			if(configData == NULL) {
				db_error("configData is NULL\n");	
				EndDialog(hDlg, -1);
			}
			SetWindowAdditionalData(hDlg, (DWORD)configData);
			//ResourceManager * rm = ResourceManager::getInstance();
			gal_pixel gp_normal_fgc =  rm->getResColor(ID_MENU_LIST, COLOR_FGC);		
			for (unsigned int i = 0; i < 6; i++){
				SetWindowElementAttr(GetDlgItem (hDlg, IDC_LABEL_START + i ), WE_FGC_WINDOW , PIXEL2DWORD(HDC_SCREEN,gp_normal_fgc) ); //0xFFDCDCDC
				if(i == INDEX_LABEL_STR_WIFI_SSID || i == INDEX_LABEL_STR_WIFI_PASSWORD){
				SetWindowElementAttr(GetDlgItem (hDlg, IDC_LABEL_START + i ), WE_FGC_WINDOW , PIXEL2DWORD(HDC_SCREEN,0xFF1E1E1E) ); 
				}
			}
			
			for (unsigned int i = 0; i < 5; i++){
				SetWindowElementAttr(GetDlgItem (hDlg, IDC_START + i), WE_FGC_WINDOW, PIXEL2DWORD(HDC_SCREEN, gp_normal_fgc) );//0xFF919191
			}
			
			SetWindowBkColor(hDlg, rm->getResColor(ID_MENU_LIST, COLOR_BGC));
			// SetWindowFont(hDlg, configData->pLogFont);
			for(unsigned int i = 0; i < 11; i++) {
				SetWindowFont(GetDlgItem(hDlg, IDC_LABEL_START + i), configData->pLogFont);
			}
		}
		break;
#if 0
	case MSG_FONTCHANGED:
		{
			PLOGFONT pLogFont;
			pLogFont = GetWindowFont(hDlg);
			for(unsigned int i = 0; i < 11; i++) {
				SetWindowFont(GetDlgItem(hDlg, IDC_LABEL_START + i), pLogFont);
			}
		}
		break;
#endif
	case MSG_PAINT:
		{
			ResourceManager * rm = ResourceManager::getInstance();
			RECT rect;
			HDC hdc = BeginPaint(hDlg);
			configData = (wifi_info_t*)GetWindowAdditionalData(hDlg);
			GetClientRect(GetDlgItem(hDlg, IDC_LABEL_TITLE), &rect);
			SetPenColor(hdc, configData->linec_title);

			Line2(hdc, 0, RECTH(rect), RECTW(rect), RECTH(rect));
			SetBorderColor(hDlg, IDCSelected, SELECT_BORDER);
			
			for(unsigned int i = 0; i < 5; i++) {
				if(IDC_START + i != IDCSelected)	
					SetBorderColor(hDlg, IDC_START + i, UNSELECT_BORDER);
			}			
			EndPaint(hDlg, hdc);
			//SendMessage(GetParent(hDlg),MSG_GET_STATE,hDlg,0);
			SendMessage(rm->getHwnd(WINDOWID_MAIN), MSG_GET_STATE,hDlg, 0);
		}
		break;
	case MSG_KEYDOWN:
		{
			if(wParam == CDR_KEY_MODE || wParam == CDR_KEY_MENU) {
				tickCnt = GetTickCount();
			}
		}
		break;
	case MSG_KEYUP:
		{	
			db_msg("++++[debug_jason]:keycode is %d+++++\n", wParam);
			ResourceManager * rm = ResourceManager::getInstance();
			switch(wParam) {
			case CDR_KEY_LEFT:
				db_msg("++++[debug_jason]:keycode  2222  is %d+++++\n", wParam);
				SendMessage(rm->getHwnd(WINDOWID_RECORDPREVIEW), MSG_CLOSE_CDR_WIFI,0, 0);
				break;
			case CDR_KEY_RIGHT:
			
				break;
			case CDR_KEY_OK:
				//EndDialog(hDlg, 0);						
				break;
			case CDR_KEY_MENU:
				break;
			case CDR_KEY_MODE:				
				//EndDialog(hDlg, 0);
				break;
			}
		}
		break;
	case MSG_WIFI_SHUT_DOWN:
		EndDialog(hDlg, IDCANCEL);
		break;
	}
	return DefaultDialogProc (hDlg, message, wParam, lParam);
}

int WifiInfoDialog(HWND hParent, wifi_info_t* configData)
{
	CTRLDATA ctrlData[11];
	DLGTEMPLATE dlgWifiInfo;
	unsigned int intval;
	memset(ctrlData, 0, sizeof(ctrlData));
	
	//db_msg("intval is %d\n", intval);	
	for(unsigned int i = 0; i < 11; i++) {		
		ctrlData[i].class_name = CTRL_STATIC;
		if(0 == i || i==5 ){
			ctrlData[i].dwStyle = SS_CENTER | SS_VCENTER | WS_VISIBLE;	
		}else{
			ctrlData[i].dwStyle = SS_LEFT | SS_VCENTER | WS_VISIBLE;
		}		
		ctrlData[i].caption = "";
		ctrlData[i].dwExStyle = WS_EX_TRANSPARENT | WS_EX_USEPARENTFONT;
		ctrlData[i].werdr_name = NULL;
		ctrlData[i].we_attrs = NULL;
		ctrlData[i].id	= IDC_LABEL_START + i;
			
		ctrlData[i].y = intval;	
		ctrlData[i].h = configData->boxH;		
		
	}	
	
	ctrlData[INDEX_LABEL_TITLE].caption = configData->title;
	ctrlData[INDEX_LABEL_TITLE].x = 0;
	ctrlData[INDEX_LABEL_TITLE].y = 0;
	ctrlData[INDEX_LABEL_TITLE].w = configData->rect.w;
	ctrlData[INDEX_LABEL_TITLE].h = configData->titleRectH-5;

	
	ctrlData[INDEX_LABEL_WIFI_SSID].caption = configData->ProductType;
	ctrlData[INDEX_LABEL_WIFI_SSID].x = 20;
	ctrlData[INDEX_LABEL_WIFI_SSID].y = 40+60;
	ctrlData[INDEX_LABEL_WIFI_SSID].w = strlen(configData->ProductType)*8/*130*/;
	ctrlData[INDEX_LABEL_WIFI_SSID].h = configData->titleRectH-5;

	ctrlData[INDEX_LABEL_STR_WIFI_SSID].caption = configData->StrProductType;
	ctrlData[INDEX_LABEL_STR_WIFI_SSID].x = ctrlData[INDEX_LABEL_WIFI_SSID].x+ctrlData[INDEX_LABEL_WIFI_SSID].w+10;
	ctrlData[INDEX_LABEL_STR_WIFI_SSID].y = 40+60;
	ctrlData[INDEX_LABEL_STR_WIFI_SSID].w = configData->rect.w-ctrlData[INDEX_LABEL_STR_WIFI_SSID].x;
	ctrlData[INDEX_LABEL_STR_WIFI_SSID].h = configData->titleRectH-5;

	ctrlData[INDEX_LABEL_WIFI_PASSWORD].caption = configData->SoftwareVersion;
	ctrlData[INDEX_LABEL_WIFI_PASSWORD].x = 20;
	ctrlData[INDEX_LABEL_WIFI_PASSWORD].y = 32+configData->titleRectH-5+60;
	ctrlData[INDEX_LABEL_WIFI_PASSWORD].w = strlen(configData->SoftwareVersion)*8/*130*/;
	ctrlData[INDEX_LABEL_WIFI_PASSWORD].h = configData->titleRectH-5;

	ctrlData[INDEX_LABEL_STR_WIFI_PASSWORD].caption = configData->StrSoftwareVersion;
	ctrlData[INDEX_LABEL_STR_WIFI_PASSWORD].x = ctrlData[INDEX_LABEL_WIFI_PASSWORD].x+ctrlData[INDEX_LABEL_WIFI_PASSWORD].w+10;
	ctrlData[INDEX_LABEL_STR_WIFI_PASSWORD].y = 32+configData->titleRectH-5+60;
	ctrlData[INDEX_LABEL_STR_WIFI_PASSWORD].w = configData->rect.w-ctrlData[INDEX_LABEL_STR_WIFI_PASSWORD].x;
	ctrlData[INDEX_LABEL_STR_WIFI_PASSWORD].h = configData->titleRectH-5;

	ctrlData[INDEX_LABEL_STR_WIFI_PROMPT].caption = configData->StrWifiPrompt;
	ctrlData[INDEX_LABEL_STR_WIFI_PROMPT].x = 0;
	ctrlData[INDEX_LABEL_STR_WIFI_PROMPT].y = 180;
	ctrlData[INDEX_LABEL_STR_WIFI_PROMPT].w = configData->rect.w;
	ctrlData[INDEX_LABEL_STR_WIFI_PROMPT].h = configData->titleRectH;
	//db_msg("luo ProductType===========%d SoftwareVersion==============%d ",strlen(configData->ProductType),strlen(configData->SoftwareVersion));	
	//db_msg("luo ***********%d %d %d %d\n",configData->rect.x,configData->rect.y,configData->rect.w,configData->rect.h);
	memset(&dlgWifiInfo, 0, sizeof(dlgWifiInfo));
	dlgWifiInfo.dwStyle = WS_VISIBLE;
	dlgWifiInfo.dwExStyle = WS_EX_NONE;
	dlgWifiInfo.x = configData->rect.x;
	dlgWifiInfo.y = configData->rect.y;
	dlgWifiInfo.w = configData->rect.w;
	dlgWifiInfo.h = configData->rect.h;
	dlgWifiInfo.caption = "";
	dlgWifiInfo.controlnr = TABLESIZE(ctrlData);
	dlgWifiInfo.controls = ctrlData;
	//dlgWifiInfo.dwAddData = &a;

	return DialogBoxIndirectParam (&dlgWifiInfo, hParent, WifiInfoProc, (LPARAM)configData);
}
