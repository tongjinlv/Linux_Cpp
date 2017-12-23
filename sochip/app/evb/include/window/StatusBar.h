
#ifndef __STATUSBAR_H__
#define __STATUSBAR_H__

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "cdr_res.h"
#include "misc.h"
#include "windows.h"
#include "cdr_message.h"
#include "ResourceManager.h"


// ------------------ statusBarTop -----------------------
static CdrIcon::CdrIcon_t s_IconSetsTop[] = {
	{ID_STATUSBAR_ICON_BACK,		"Back"},
};

static CdrLabel::CdrLabel_t s_LabelSetsTop[] = {
	{ID_STATUSBAR_LABEL1,			"StatusBarLabel1"},
};
// ++++++++++++++++++ end of statusBarTop ++++++++++++++++++++


// ------------------ statusBarBottom -----------------------
static CdrIcon::CdrIcon_t s_IconSetsBottom[] = {
	{ID_STATUSBAR_ICON_REC,		"REC"},		
	{ID_STATUSBAR_ICON_PIC,		"PIC"},
	//{ID_STATUSBAR_ICON_RECORD_SOUND,		"RecordSound"},
};
static CdrLabel::CdrLabel_t s_LabelSetsBottom[] = {
	//{ID_STATUSBAR_LABEL_RECORD_TIME,			"RecordTime"},
};
// ++++++++++++++++++ end of statusBarBottom ++++++++++++++++++++

#endif


