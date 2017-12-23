#ifndef _EVENT_MANAGER_H
#define _EVENT_MANAGER_H

#include <ThreadDefs.h>
#include <string>
#include <utils/Thread.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "uevent.h"
#include "ConfigData.h"


using namespace zeta;

enum event_msg {
	EVENT_UVC_PLUG_IN	= 1,
	EVENT_UVC_PLUG_OUT,
	EVENT_CONNECT2PC,
	EVENT_DISCONNECFROMPC,
	EVENT_IMPACT,
	EVENT_BATTERY,
	EVENT_DELAY_SHUTDOWN,
	EVENT_CANCEL_SHUTDOWN,
	EVENT_MOUNT,
	EVENT_REMOVESD,
	EVENT_INSERTSD,
	EVENT_HDMI_PLUGIN,
	EVENT_HDMI_PLUGOUT,
	EVENT_TVD_PLUG_IN,
	EVENT_TVD_PLUG_OUT,
#ifdef APP_TVOUT
	EVENT_TVOUT_PLUG_IN,
	EVENT_TVOUT_PLUG_OUT,
#endif
};

enum {
	GSENSOR_CLOSE = 0,
	GSNESOR_LOW ,
	GSNESOR_MIDDLE ,
	GSNESOR_HIGH
};

enum{
	BATTERY_CHARGING = 1,
	BATTERY_DISCHARGING,
	BATTERY_NOTCHARGING,
	BATTERY_FULL
};

static int mHdmiInOutFlag;

#define THRESHOLD_VALUE 18

#define HDMI_NODE "hdmi"
#ifdef A20
#define HDMI_ONLINE_FILE	"sys/devices/virtual/switch/hdmi/state"
#else
#define HDMI_ONLINE_FILE "/sys/class/switch/hdmi/state"
#endif

#define UVC_NODE	"video"

#ifdef PLATFORM_0
	#define UVC_ADD		"add@/devices/platform/sw-ehci."
	#define UVC_REMOVE	"remove@/devices/platform/sw-ehci."
	#define USB_NODE				"usb"
	#define USB_ONLINE_FILE			"/sys/class/power_supply/usb/online"
#else
	#define UVC_ADD		"add@/devices/platform/sunxi-"
	#define UVC_REMOVE	"remove@/devices/platform/sunxi-"
	#define USB_NODE				"android0"
	#define USB_ONLINE_FILE			"/sys/class/android_usb/android0/state"
	#define USB_CONNECTED  "CONNECTED"
	#define USB_CONFIGURED "CONFIGURED"
	#define USB_DISCONNECT "DISCONNECTED"
	#define USB_FILE "/sys/devices/platform/sunxi_usb_udc/gadget/lun0/file"
#endif
#define BATTERY_NODE			"battery"
#define POWER_BATTERY			"power_supply/battery"
#define UEVENT_CHANGE			"change"

#define BATTERY_ONLINE_FILE		"/sys/class/power_supply/battery/online"
#define BATTERY_CAPACITY			"/sys/class/power_supply/battery/capacity"
#define BATTERY_DETECT				"/sys/class/power_supply/battery/present"
#define AC_ONLINE_FILE			"/sys/class/power_supply/ac/online"
#define USB_ONLINE              "/sys/class/power_supply/usb/online"
#define TVD_FILE_ONLINE			"/sys/devices/tvd/tvd"

#define CD_TFCARD_PATH "/dev/block/179:0"
#define CD_TFCARD_LUN_PATH "/sys/class/android_usb/android0/f_mass_storage/lun/file"

#define BATTERY_STATUS_FILE                    "/sys/class/power_supply/battery/status"


#define TFCARD_ADD_HEAD			"add@/devices/platform/sunxi-mmc."
#define TFCARD_END				"mmcblk0"
#define TFCARD_REMOVE_HEAD		"remove@/devices/platform/sunxi-mmc."


class EventManager;

class EventListener
{
public:
		EventListener(){};
		virtual ~EventListener(){};
		virtual int notify(int message, int val) = 0;
};


class EventManager
{
public:
	EventManager();
	~EventManager();
	int init(void);
	int exit(void);
	void connect2PC();
	void disconnectFromPC();
	int checkUSB(char *udata);
	bool isBatteryOnline();
	static int getBattery();
	static int getCurBattery();
	static int batteryDetct();
	static int getBatteryStatus();
	bool getHdmiStatus();
	int checkHDMI(const char *str);
	int getHdmistateValue(const char *str);
	void detectTVD();
	void startDetectTVD();
	void hdmiAudioOn(bool open);
	int usbConnect();

	void detectKey();
	int startDetectKey();
	void detectPowerKey();
	int startDetectPowerKey();
	class UeventThread : public Thread
	{
	public:
		UeventThread(EventManager *em)
			:mEM(em)
		{
		}
		void startThread()
		{
			run("UeventThread", PRIORITY_NORMAL);
		}
		void stopThread()
		{
			requestExitAndWait();
		}
		bool threadLoop()
		{
			mEM->ueventLoop();
			return true;
		}
	private:
		EventManager *mEM;
	};

	class KeyDetectThread : public Thread
	{
		public:
			KeyDetectThread(EventManager *em)
			: mEM(em)
			{}
			void startThread()
			{run("KeyDetectThread", PRIORITY_NORMAL);}
			void stopThread()
			{requestExitAndWait();}
			bool threadLoop()
			{
				mEM->detectKey();
				return true;
			}
		private:
			EventManager *mEM;
			bool mNeedStop;
			int mFD;
	};

	class PowerKeyDetectThread : public Thread
	{
		public:
			PowerKeyDetectThread(EventManager *em)
			:mEM(em)
			{}
			void startThread()
			{run("PowerKeyDetectThread", PRIORITY_NORMAL);}
			void stopThread()
			{requestExitAndWait();}
			bool threadLoop()
			{
				mEM->detectPowerKey();
				return true;
			}
			private:
				EventManager *mEM;
				bool mNeedStop;
				int mFD;
	};
	void setEventListener(EventListener *pListener);
	int	checkTFCard(char *udata);
    bool tf_flag;
private:
	UeventThread *mUT;
	KeyDetectThread *mKeyDetectThread;
	PowerKeyDetectThread *mPowerKeyDetectThread;
	EventListener *mListener;
	int ueventLoop(void);
	int checkBattery(int &level);
	int checkACPlug();
	bool mBatteryOnline;
	bool mUSBDisconnect;
	bool mTVDOnline;
	bool connect_pc;
	bool mACOnline;
};

#endif
