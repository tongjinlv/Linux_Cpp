
#ifndef __WIFI_TIP_H__
#define __WIFI_TIP_H__

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

#define SELECT_BORDER	1
#define UNSELECT_BORDER 0

extern int show_wifi_tip(HWND hParent);
#endif
