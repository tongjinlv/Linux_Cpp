#include "EventManager.h"
#include "StorageManager.h"
#include "ResourceManager.h"
#include <sys/mman.h> 
#include "debug.h"
#include "windows.h"
#include<linux/input.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "EventManager"

#define KEY_DEV "/dev/input/event0"
#define POWER_DEV "/dev/input/event1"

using namespace zeta;
void get_oneline(const char *pathname, std::string &val);
void set_usbconfig(const char *string);

#define SHARE_VOL 1

enum PC_CONNECT_STATE {
	DISCONNECT = 0, // no pccam:DISCONNECTED pccam:ADB_RESTART && DISCONNECTED
	CONNECTED,  // CONFIGURE
	ADB_RESTART_R, // pccam:DISCONNECTED
	ADB_RESTART_O, // pccam:CONFIGURE PCCAM_CONNECTED
};

static int gState = 0;



EventManager::EventManager()
	: mListener(NULL)
	, mUT(NULL)
	, mPowerKeyDetectThread(NULL)
	, mKeyDetectThread(NULL)
	, mBatteryOnline(false)
	, mUSBDisconnect(true)
	, tf_flag(false)
	, connect_pc(false)
	, mACOnline(false)
{
	db_msg("by hero EventManager start\n");
	
	if(access(CD_TFCARD_PATH, R_OK) != 0) {
		db_msg("/sys/devices/platform/sunxi_usb_udc/gadget/lun0/file can not read\n");        
	}else{
		if(!tf_flag){
			system("mount -t vfat /dev/mmcblk0 /mnt/extsd");
			tf_flag = true;
		}
	}
	set_usbconfig("mass_storage");
	db_msg("by hero EventManager end\n");
}

EventManager::~EventManager()
{
	exit();
}

int EventManager::init()
{
	uevent_init();
	if (mUT == NULL) {
		mUT = new UeventThread(this);
		mUT->startThread();
		db_msg("-----test Eventmanager: Uevent thread start-----\n");
	}
	startDetectPowerKey();
	startDetectKey();
	return 0;
}

int EventManager::exit(void)
{
	if (mUT != NULL) {
		mUT->stopThread();
		delete mUT;
		mUT = NULL;
	}
	if (mPowerKeyDetectThread != NULL) {
		mPowerKeyDetectThread->stopThread();
		delete mPowerKeyDetectThread;
		mPowerKeyDetectThread = NULL;
	}
	if (mKeyDetectThread != NULL) {
		mKeyDetectThread->stopThread();
		delete mKeyDetectThread;
		mKeyDetectThread = NULL;
	}
	return 0;
}

void EventManager::hdmiAudioOn(bool open)
{

}

void EventManager::setEventListener(EventListener *pListener)
{
	mListener = pListener;
	int level;
	checkBattery(level);
	mListener->notify(EVENT_BATTERY, level);
}

bool EventManager::isBatteryOnline()
{
	return mBatteryOnline;
}

void set_usbconfig(const char *string)
{
	if(!strcmp(string, "mass_storage")){
		system("echo 0 > /sys/class/android_usb/android0/enable");
		system("echo 1f3a > /sys/class/android_usb/android0/idVendor");
		system("echo 1002 > /sys/class/android_usb/android0/idProduct");
		system("echo mass_storage > /sys/class/android_usb/android0/functions");
		system("echo 1 > /sys/class/android_usb/android0/enable");
	}
}


void get_oneline(const char *pathname, std::string &val)
{
	FILE * fp;
	char linebuf[1024];

	if(access(pathname, F_OK) != 0) {
		//db_msg("%s not exsist\n", pathname);	
	}

	if(access(pathname, R_OK) != 0) {
		//db_msg("%s can not read\n", pathname);	
	}

	fp = fopen(pathname, "r");

	if (fp == NULL) {
		return;
	}

	if(fgets(linebuf, sizeof(linebuf), fp) != NULL) {
		fclose(fp);
		val = linebuf;
		return ;
	}

	fclose(fp);
}

bool isUVCStr(const char *str)
{
	char buf[128]={""};
	const char *num;
	int idx;
	getBaseName(str, buf);
//	db_msg("isUVCstr :%s", buf);
	if (strncmp(buf, UVC_NODE, strlen(UVC_NODE))) {
		return false;
	}
	num = buf+strlen(UVC_NODE);
	db_msg("isUVCstr num :%s\n", num);
	idx = std::atoi(num);
	if (0<idx && idx<64) {
		return true;
	}
	return false;
}

int checkUVC(char *udata)
{
	int ret = 0;
	const char *tmp_start;
	tmp_start = udata;

	if (isUVCStr(udata)) {
		if (!strncmp(tmp_start, UVC_ADD, strlen(UVC_ADD))) {
			db_msg("\nUVC Plug In\n");
			ret = EVENT_UVC_PLUG_IN;
			//mListener->notify(EVENT_UVC_PLUG_IN);	
		} else if (!strncmp(tmp_start, UVC_REMOVE, strlen(UVC_REMOVE))) {
			db_msg("\nUVC Plug Out\n");
			ret = EVENT_UVC_PLUG_OUT;
			//mListener->notify(EVENT_UVC_PLUG_OUT);
		} else {
			db_msg("unkown message for UVC\n");	
			db_msg("tmp_start is %s\n", tmp_start);
		}
	}
	return ret;
}

static inline int capacity2Level(int capacity)
{
	return ((capacity - 1) / 20 + 1);
}

int EventManager::checkACPlug()
{
	int ret = 0;
	std::string val_new("");
	
	get_oneline(BATTERY_ONLINE_FILE, val_new);
	bool battery_online = (std::atoi(val_new.c_str()) == 1) ? true : false;
	db_msg("mBatteryOnline:%d, battery_online:%d mUSBDisconnect:%d\n", mBatteryOnline, battery_online, mUSBDisconnect);
	get_oneline(AC_ONLINE_FILE, val_new);
	bool ac_online = (std::atoi(val_new.c_str()) == 1) ? true : false;
	mACOnline = ac_online;
	//db_msg("mAConline:%d", mACOnline);
	if(((CdrMain*)mListener)->getPCCamMode() == true && mACOnline) {
		gState = DISCONNECT;
		mUSBDisconnect = true;
		mListener->notify(EVENT_DISCONNECFROMPC, 0);
	}
	if (mBatteryOnline==false && battery_online==true) {
		mBatteryOnline = battery_online;
		if (mUSBDisconnect == false && mACOnline) {
			return ret;
		}
		return EVENT_DELAY_SHUTDOWN;
	} else if(mBatteryOnline==true && battery_online==false) {
		mBatteryOnline = battery_online;
	return EVENT_CANCEL_SHUTDOWN;
	}
	
	return ret;
}

static bool isBatteryEvent(char *udata)
{
	const char *tmp_start;
	const char *tmp_end;
	tmp_start = udata;
	tmp_end = udata + strlen(udata) - strlen(POWER_BATTERY);
	
	if (strcmp(tmp_end, POWER_BATTERY)) { //battery
		return false;
	}
	if (strncmp(tmp_start, UEVENT_CHANGE, strlen(UEVENT_CHANGE))) { //change
		return false;
	}
	return true;
}

int EventManager::checkBattery(int &level)
{
	int ret = 0;
	std::string val("");
	int battery_capacity = 0;
	
	get_oneline(BATTERY_CAPACITY, val);
	battery_capacity = std::atoi(val.c_str());

	level = capacity2Level(battery_capacity);

	ret = EVENT_BATTERY;
	return ret;
}

int EventManager::getBattery()
{
	int ret = 0;
	std::string val("");
	int battery_capacity = 0;
	get_oneline(BATTERY_ONLINE_FILE, val);
	if (std::atoi(val.c_str()) != 1) {	   //not use battery
		db_msg("has not use battery\n");
		return -1;
	}
	get_oneline(BATTERY_CAPACITY, val);
	battery_capacity = std::atoi(val.c_str());
	db_msg("++++++++++[debug_jaosn]:the battery_capacity is %d +++++++++\n",battery_capacity);
	return battery_capacity;
}

int EventManager::getCurBattery()
{
	std::string val("");
	int battery_capacity = 0;
	get_oneline(BATTERY_CAPACITY, val);
	battery_capacity = std::atoi(val.c_str());
	db_msg("++++++++++[debug_jaosn]:the battery_capacity is %d +++++++++\n",battery_capacity);
	return battery_capacity;
}

int EventManager::batteryDetct()
{
	
	int ret = 0;
	std::string val("");
	int battery_capacity = 0;
	get_oneline(BATTERY_DETECT, val);
	ret = std::atoi(val.c_str());

	if (ret != 1) {	//not use battery
		db_msg(" not have battery\n");
		return -1;
	}
	return ret;
}
int EventManager::usbConnect()
{
	std::string val_usb;
	int ret = -1;
	get_oneline(USB_ONLINE_FILE, val_usb);
	if (strncmp(val_usb.c_str(),USB_CONNECTED, 9) == 0 || strncmp(val_usb.c_str(), USB_CONFIGURED, 10) == 0) {
		ret =  1;
	} else if (strncmp(val_usb.c_str(),USB_DISCONNECT, 12) == 0) {
		if(!connect_pc)
			ret =  0;
	}
	return ret;
}

int EventManager::getBatteryStatus()
{
	char status[512]={0};
	FILE * fp;      
	if(access(BATTERY_STATUS_FILE, F_OK) != 0) {
	       db_msg("/sys/class/power_supply/battery/status not exsist\n");  
	}
	if(access(BATTERY_STATUS_FILE, R_OK) != 0) {
	       db_msg("/sys/class/power_supply/battery/status can not read\n");        
	}
	fp = fopen(BATTERY_STATUS_FILE, "r");
	if (fp == NULL) {
	       return 0;
	}       
	if(fgets(status, sizeof(status), fp) == NULL) {
	       db_msg("/sys/class/power_supply/battery/status read failed\n");
	       fclose(fp);
	       return 0;
	}
	fclose(fp);
	status[strlen(status)-1]='\0';
	if(strcmp(status,"Charging")==0){
	       return BATTERY_CHARGING;
	       
	}else if(strcmp(status,"Discharging")==0){
	       return BATTERY_DISCHARGING;     
	}else if(strcmp(status,"Not charging")==0){
	       return BATTERY_NOTCHARGING;             
	}else if(strcmp(status,"Full")==0){
	       return BATTERY_FULL;
	}
	return 0;

}

int EventManager::checkUSB(char *udata)
{
	const char *tmp_start;
	const char *tmp_end;
	tmp_start = udata;
	tmp_end = udata + strlen(udata) - strlen(USB_NODE);
	std::string val_usb;
	int ret = -1;
	if (strcmp(tmp_end, USB_NODE)) { 
		return -1;
	}

	if (strncmp(tmp_start, UEVENT_CHANGE, strlen(UEVENT_CHANGE))) { 
		return -1;
	}
	bool usb_online;

	get_oneline(USB_ONLINE_FILE, val_usb);

	db_msg("current state:%d\n", gState);
	switch(gState) {
		case DISCONNECT: {
				if (usbConnect() == 1) {
					gState = CONNECTED;
					mUSBDisconnect = false;
					ret = EVENT_CONNECT2PC;
				}
			}
			break;
		case CONNECTED: {
				if(usbConnect() == 0) {
					if(((CdrMain*)mListener)->getPCCamMode() == true) {
						// adb release
						gState = ADB_RESTART_R;
						ret = -1;
					} else {
						gState = DISCONNECT;
						mUSBDisconnect = true;
						ret = EVENT_DISCONNECFROMPC;
					}
				}
			}
			break;
		case ADB_RESTART_R: {
				if(((CdrMain*)mListener)->getPCCamMode() == true) {
					if(usbConnect() == 1) {
						// adb reopen
						gState = ADB_RESTART_O;
						ret = -1;
					} else {
						gState = DISCONNECT;
						mUSBDisconnect = true;
						ret = EVENT_DISCONNECFROMPC;
					}
				}
			}
			break;
		case ADB_RESTART_O: {
				if(usbConnect() == 0 && ((CdrMain*)mListener)->getPCCamMode() == true) {
					// webcam close
					gState = DISCONNECT;
					mUSBDisconnect = true;
					ret = EVENT_DISCONNECFROMPC;
				}
			}
			break;
		default:
			break;
	}
	db_msg("next state:%d, val_usb %s\n", gState, val_usb.c_str());

	return ret;
}
int EventManager::checkTFCard(char *udata)
{
	const char *tmp_start;
	const char *tmp_end;
	tmp_start = udata;
	int ret = -1;
	tmp_end = udata + strlen(udata) - strlen(TFCARD_END);
	db_msg("tmp_start:%s",tmp_start);
	db_msg("tmp_end:%s",tmp_end);
	if (strcmp(tmp_end, TFCARD_END)) {
		return -1;
	}
	if (strncmp(tmp_start, TFCARD_ADD_HEAD, strlen(TFCARD_ADD_HEAD))==0) {
		return EVENT_INSERTSD;
	}
	if (strncmp(tmp_start, TFCARD_REMOVE_HEAD, strlen(TFCARD_REMOVE_HEAD))==0) {
		return EVENT_REMOVESD;
	}
	return -1;

}
static bool getValue(const char *str)
{
	std::string val;
	get_oneline(str, val);
	return (std::atoi(val.c_str())==1)?true:false;
}

int EventManager::getHdmistateValue(const char *str)
{
	std::string val;
	bool flag;
	get_oneline(str, val);
	flag = (std::atoi(val.c_str())==1)?true:false;
	if(flag == true){
		mHdmiInOutFlag = true;
	}else if(flag == false){
		mHdmiInOutFlag = false;
	}
	return flag;
	
}

int EventManager::checkHDMI(const char *str)
{
	char buf[32];
	getBaseName(str, buf);
	bool val;
	if (strncmp(buf, HDMI_NODE, strlen(HDMI_NODE))) {
		return 0;
	}
	if (strncmp(str, UEVENT_CHANGE, strlen(UEVENT_CHANGE))) { //change
		return 0;
	}
	val = getValue(HDMI_ONLINE_FILE);
	if (val) {
		mHdmiInOutFlag = true;
		return EVENT_HDMI_PLUGIN;
	}else{
		mHdmiInOutFlag = false;
	}	
	return EVENT_HDMI_PLUGOUT;
}


bool EventManager::getHdmiStatus()
{
	return mHdmiInOutFlag;
}

int EventManager::ueventLoop(void)
{
	char udata[4096] = {0};
	uevent_next_event(udata, sizeof(udata) - 2);
	
	db_msg("uevent_loop %s\n", udata);
	if(mListener) {
		int ret = 0;
		int val = 4;

		ret = checkUSB(udata);
		if (ret > 0) {
			mListener->notify(ret, 0);
			return ret;
		}
		ret = checkTFCard(udata);
		if (ret > 0) {
			mListener->notify(ret, 0);
			return ret;
		}
		if(isBatteryEvent(udata)) {
			ret = checkACPlug();
			if (ret > 0) {
				mListener->notify(ret, val);
				return ret;
			}
		ret = checkBattery(val);
			if (ret > 0) {
				mListener->notify(ret, val);
				return ret;
			}
		}

		ret = checkHDMI(udata);
		if (ret > 0) {
			mListener->notify(ret, 0);
			return ret;
		}

	}

	return 0;
}
void EventManager::connect2PC()
{	
	if(access(CD_TFCARD_PATH, R_OK) != 0) {
		db_msg("/sys/devices/platform/sunxi_usb_udc/gadget/lun0/file can not read\n");   
		return;
	}else{
		if(tf_flag){
			system("umount /mnt/extsd");
			tf_flag = false;
		}
	}
	connect_pc = true;

	set_usbconfig("mass_storage");
	
	{
		int fd = open(USB_FILE, O_RDWR);
		write(fd, CD_TFCARD_PATH, strlen(CD_TFCARD_PATH));
		close(fd);		
	}

	sleep(1);
	connect_pc = false;
}

void EventManager::disconnectFromPC()
{
	int fd = open(USB_FILE, O_RDWR);
	write(fd, " ", 1);
	close(fd);
	if(access(CD_TFCARD_PATH, R_OK) != 0) {
		db_msg("/sys/devices/platform/sunxi_usb_udc/gadget/lun0/file can not read\n");        
	}else{
		if(!tf_flag){
			system("mount -t vfat /dev/mmcblk0 /mnt/extsd");
			tf_flag = true;
		}
	}
	
}

//static int ts_fd = -1;
//static int pw_fd = -1;
#define USER_KEY_POWER 	116
#define USER_KEY_OK		28
#define USER_KEY_LEFT	115
#define USER_KEY_RIGHT	114
#define USER_KEY_MODE    139

static struct input_event data;
static struct input_event data2;

static int init_device(char *TS_DEV)
{
	int fd;
	db_msg(" init_device ===============\n");
    if((fd = open(TS_DEV, O_RDONLY)) < 0)
    {
        db_msg("Error open %s\n\n", TS_DEV);
        return -1;
    }
    return fd;
}

void EventManager::detectKey()
{
	int downup;
	int msg;
	int fd = init_device(KEY_DEV);
    if(fd < 0)
        return ;
    while(1)
    {
		db_msg(" detectKey ===============\n");
        read(fd, &data, sizeof(data));
        if (data.type == EV_KEY){
            db_msg(" type: EV_KEY, event = %d, value = %d\n",data.code, data.value);
			switch(data.code){
				case USER_KEY_OK:
					msg = CDR_KEY_OK;
					break;
				case USER_KEY_LEFT:
					msg = CDR_KEY_LEFT;
					break;
				case USER_KEY_RIGHT:
					msg = CDR_KEY_RIGHT;
					break;
				case USER_KEY_MODE:
					msg = CDR_KEY_MODE;
					break;
				default:
					msg = 0;
					break;
			}
			downup= data.value ? MSG_KEYDOWN : MSG_KEYUP;
			mListener->notify(msg, downup);
        }

		usleep(200*1000);
	}
	close(fd);
    //return 0;
}

int EventManager::startDetectKey()
{
	db_msg("----mKeyDetectThread start================\n");
	if (mKeyDetectThread == NULL) {
		mKeyDetectThread = new KeyDetectThread(this);
		mKeyDetectThread->startThread();
	}
	return 0;
}

void EventManager::detectPowerKey()
{
	int downup;
	int fd = init_device(POWER_DEV);
    if(fd < 0)
        return ;
    while(1)
    {
		db_msg(" detectPowerKey ===============\n");
        read(fd, &data2, sizeof(data2));
        if (data2.type == EV_KEY && data2.code == USER_KEY_POWER){
            db_msg(" type: EV_KEY, event = %d, value = %d\n",data2.code, data2.value);
			downup= data2.value ? MSG_KEYDOWN : MSG_KEYUP;
			mListener->notify(CDR_KEY_POWER, downup);
        }

		usleep(200*1000);
	}
	close(fd);
    //return 0;
}

int EventManager::startDetectPowerKey()
{
	db_msg("----mKeyDetectThread start================\n");
	if (mPowerKeyDetectThread == NULL) {
		mPowerKeyDetectThread = new PowerKeyDetectThread(this);
		mPowerKeyDetectThread->startThread();
	}
	return 0;
}

