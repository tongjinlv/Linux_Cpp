#include <string.h>
#include <stdlib.h>

#include <minigui/mgconfig.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include <minigui/ctrl/listbox.h>

#include "keyEvent.h"
#include "misc.h"
#include "cdr_message.h"
#include "cdr_widgets.h"

#include "include/ctrlclass.h"


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "MainList.cpp"
#include "debug.h"

int MainListCallback(HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
	static BITMAP bitImage;
	switch(message) {
	case MSG_CREATE:
		{
			#if 0
			char *filepath=(char *)"/etc/res/main/main.png";
			LoadBitmapFromFile(HDC_SCREEN, &bitImage, filepath);
			#endif
		}
		break;
	case MSG_PAINT:
		{
			#if 0
			HDC		hdc;
			RECT	rc;
			hdc = BeginPaint (hwnd);
			if(1){
				int screenW,screenH;
				if(bitImage.bmBits == NULL)
				{
					char *filepath=(char *)"/etc/res/main/main.png";
					LoadBitmapFromFile(HDC_SCREEN, &bitImage, filepath);
				}
				getScreenInfo(&screenW, &screenH);
				FillBoxWithBitmap(hdc,0,0,screenW,screenH,&bitImage); 
			}
			EndPaint (hwnd, hdc);
			return 0;
			#endif
		}
		break;
	case MSG_SIZECHANGED:   
		break;
	case MSG_DESTROY:
		#if 0
		{
			UnloadBitmap(&bitImage);
		}
		#endif
		break;
	case MSG_MOUSEMOVE:
	case MSG_LBUTTONDOWN:
		break;
	case MSG_LBUTTONUP:
		break;
	case MSG_KEYDOWN:
		if(wParam == CDR_KEY_OK) {
			db_msg("press key ok\n");
		}else{
			db_msg("press key down\n");
		}
		break;
	default:
		break;
	} 

	return DefaultControlProc (hwnd, message, wParam, lParam);
}
