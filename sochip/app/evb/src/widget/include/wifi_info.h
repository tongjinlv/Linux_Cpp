
#ifndef __WIFI_INFO_H__
#define __WIFI_INFO_H__

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

#include "cdr_widgets.h"
#include "cdr_message.h"
#include "keyEvent.h"
#include "Rtc.h"


#define IDC_LABEL_START					200
#define IDC_LABEL_TITLE					200
#define IDC_LABEL_WIFI_SSID				201
#define IDC_LABEL_WIFI_PASSWORD			202
#define IDC_LABEL_STR_WIFI_SSID			203
#define IDC_LABEL_STR_WIFI_PASSWORD		204
#define IDC_LABEL_STR_WIFI_PROMPT		205
#define IDC_START						206
#define IDC_WIFI_SSID					206
#define IDC_WIFI_PASSWORD				207
#define IDC_STR_WIFI_SSID				208
#define IDC_STR_WIFI_PASSWORD			209
#define IDC_STR_WIFI_PROMPT				210


#define INDEX_LABEL_TITLE				 0
#define INDEX_LABEL_WIFI_SSID		     1
#define INDEX_LABEL_WIFI_PASSWORD	     2
#define INDEX_LABEL_STR_WIFI_SSID	     3
#define INDEX_LABEL_STR_WIFI_PASSWORD 	 4
#define INDEX_LABEL_STR_WIFI_PROMPT 	 5



#define SELECT_BORDER	1
#define UNSELECT_BORDER 0


#endif
