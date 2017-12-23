#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#include "windows.h"
#include "cdr_message.h"
#include "keyEvent.h"

#include "PowerManager.h"
#include "EventManager.h"

/* MainWindow */
#define ID_TIMER_KEY 100
#define ID_WIFI_LED_KEY 101
#define ID_WIFI_CLOSE_SCREEN 102
#define ID_DIGHT_ZOOM 103
#define ID_FIEWORD_REWIND 104
#define ID_TIMER_KEY_1 105




static bool wifi_led_flag ;
static bool screen_on_off_flag ;
static bool battery_is_full ;
static bool first_hdmi_plugin;


#define CDRMain \
MiniGUIAppMain (int args, const char* argv[], CdrMain*); \
int main_entry (int args, const char* argv[]) \
{ \
	printf("main entry\n"); \
    int iRet = 0; \
	CdrMain *cdrMain = new CdrMain(); \
	cdrMain->initPreview(NULL); \
    if (InitGUI (args, argv) != 0) { \
        return 1; \
    } \
    iRet = MiniGUIAppMain (args, argv, cdrMain); \
    TerminateGUI (iRet); \
    return iRet; \
} \
int MiniGUIAppMain
#endif
