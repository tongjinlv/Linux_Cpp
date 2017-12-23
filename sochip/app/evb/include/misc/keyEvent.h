#ifndef _KEY_EVENT_H
#define _KEY_EVENT_H

#include <minigui/window.h>

#define MSG_CDR_KEY (MSG_USER + 500)

#define CDR_KEY_RIGHT	SCANCODE_SUNXIRIGHT
#define CDR_KEY_MODE	SCANCODE_SUNXIMODE
#define CDR_KEY_POWER	SCANCODE_SUNXIPOWER
#define CDR_KEY_OK	    SCANCODE_SUNXIOK
#define CDR_KEY_LEFT	SCANCODE_SUNXILEFT
#define CDR_KEY_MENU	SCANCODE_SUNXIMENU

#define LONG_PRESS_TIME 200 /* unit:10ms */
#define LONG_PRESS_TIME_1 80 /* unit:10ms --use for set time (long press wifi is subtraction 1)*/

#define LOWPOWER_SHUTDOWN_TIME (10) /* unit: s */
#define NOWORK_SHUTDOWN_TIME (20)
#define NOWORK_SHUTDOWN_TIME2 (5)
#define KEY_CNT 5


#define ID_TIMER1_KEY 110

/* KEY MACRO */
enum {
	SHORT_PRESS = 0,
	LONG_PRESS
};

#endif


