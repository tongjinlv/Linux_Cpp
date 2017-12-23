#include "PowerManager.h"
#undef LOG_TAG
#define LOG_TAG "PowerManager"
#include "debug.h"
#include "windows.h"
#include <sys/ioctl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/watchdog.h>

using namespace zeta;

PowerManager::PowerManager(CdrMain *cm)
	: mDispFd(0),
	  mSS(NULL),
  	  mWD(NULL),
  	  mOffTime(TIME_INFINITE),
  	  mState(SCREEN_ON),
      standbyFlag(false),
      mEnterStandby(false),
      mRestoreScreenOn(false)
{
	db_msg("by hero PowerManager start\n");
	mDispFd = open(DISP_DEV, O_RDWR);
	if (mDispFd < 0) {
		db_msg("fail to open %s", DISP_DEV);
	}

	mSS = new ScreenSwitchThread(this);
	mSS->startThread();

	isEnterStandby=true;
	isEnterStandby2=true;
	mCdrMain = cm;
	setBatteryLevel();
	
	db_msg("by hero PowerManager end\n");
	//watchdogRun();
}

void PowerManager::enterStandby(int pid)
{
	db_msg(" ");
	system("/usr/bin/ZetaUpdate");
	isEnterStandby=false;
}

void PowerManager::connect2StandbyService()
{

	return ;
}
int PowerManager::setBatteryLevel()
{

	int cap = EventManager::getBattery();
	if(cap<=0){
	       db_msg("****mBatteryLevel:%d****",mBatteryLevel); //-- return 1 -------
	       return mBatteryLevel;
	}else{
	       int val = cap/20;
	       db_msg("***val:%d*******",val);
	       switch(val){
	               case 0:
	                       mBatteryLevel = 0;
	                       break;
	               case 1:
	                       mBatteryLevel = 1;
	                       break;
	               case 2:
	                       mBatteryLevel = 2;
	                       break;
	               case 3:
	                       mBatteryLevel = 3;
	                       break;
	               case 4:
	                       mBatteryLevel = 4;
	                       break;
	               default:
	                       mBatteryLevel = 4;
				break;
	       }
	}
	return mBatteryLevel;

}

int PowerManager::getBatteryLevel(void)
{
	return mBatteryLevel;
}

PowerManager::~PowerManager()
{	

	delete mSS;
	delete mWD;

	close(mDispFd);
}

void PowerManager::setOffTime(unsigned int time)
{
	std::unique_lock<std::mutex> lock(mLock);
	mOffTime = time;
	pulse();
}

void PowerManager::pulse()
{
	db_msg("pulse\n");
	mCon.notify_one();
}

void PowerManager::standbyOver()
{
	std::unique_lock<std::mutex> lock(mLock);
	standbyFlag = false;
	if (mRestoreScreenOn) {
		screenOn();
	}
}

void PowerManager::screenSwitch()
{
	std::unique_lock<std::mutex> lock(mLock);
	if(!standbyFlag){
		if (mState == SCREEN_ON) {
			screenOff();
		} else {
			screenOn();
		}
	}
	pulse();
}

void PowerManager::isScreenOnOff()
{	
    int retval = 0;
	unsigned long args[4]={0};
	std::unique_lock<std::mutex> lock(mLock);
	if(!standbyFlag){
		if (mState == SCREEN_ON) {
			screenOff();	
			pulse();
			screenOn();
			pulse();
		//	args[1] = DISP_OUTPUT_TYPE_NONE;
		//	retval = ioctl(mDispFd, DISP_DEVICE_SWITCH, args);
		//	retval = ioctl(mDispFd, DISP_DEVICE_SWITCH, args);
		//	usleep(100 * 1000);
		//	args[1] = DISP_OUTPUT_TYPE_LCD;
		//	retval = ioctl(mDispFd, DISP_DEVICE_SWITCH, args);
		}
	}
	
}

bool PowerManager::isScreenOn()
{
	std::unique_lock<std::mutex> lock(mLock);
	return (mState == SCREEN_ON);
}

void PowerManager::powerOff()
{
	mPO = new poweroffThread(this);
	mPO->start();
}

int PowerManager::poweroff()
{
	db_msg("power off");
//	screenOff(); //-----------��ע��
	//android_reboot(ANDROID_RB_POWEROFF, 0, 0);
	db_error("power off!");

	return 0;
}

void PowerManager::reboot()
{
	//android_reboot(ANDROID_RB_RESTART, 0, 0);
}

int PowerManager::getStandby_status()
{
	return standbyFlag;//isEnterStandby2;
}

void PowerManager::setStandby_status(bool flag)
{
	isEnterStandby2 = flag;
}

void PowerManager::enterStandbyModem()
{
	standbyFlag = true;
	standbySuspend();
	isEnterStandby2  = false;
	db_msg(" standbySuspend!!!!!!!");
	system("echo mem > /sys/power/state");
//	usleep(1000*1000);
//	standbyResume();
//	db_msg(" standbyResume!!!!!!!");
}

int PowerManager::standbySuspend()
{
	if (mEnterStandby) {
		mRestoreScreenOn = true;
	}
	int ret,args[4]={0};
	args[0] = 0;
	args[1] = 1;
	ret = ioctl(mDispFd, DISP_BLANK, args);
	mEnterStandby = true;
	return ret;
}

int PowerManager::standbyResume()
{
	int ret,args[4]={0};
	args[0] = 0;
	args[1] = 0;
	ret = ioctl(mDispFd, DISP_BLANK, args);
	mEnterStandby = false;
	return ret;
}

int PowerManager::screenOff()
{
	db_msg("screenOff");
#ifdef HERO_OLEDUI
	mState = SCREEN_OFF;
	return 0;
#else
	int retval = 0;
	unsigned long args[4]={0};
	#ifdef DE2
		#ifdef NORMAL_STANDBY
			standbySuspend();
			db_msg("standbySuspend```");
		#else
			args[1] = DISP_OUTPUT_TYPE_NONE;
			retval = ioctl(mDispFd, DISP_DEVICE_SWITCH, args);
		#endif
	#else
		retval = ioctl(mDispFd, DISP_CMD_LCD_OFF, args);
	#endif
		if (retval < 0) {
			db_error("fail to set screen on\n");
			return -1;
		}

	mState = SCREEN_OFF;
	return 0;
#endif
}

int PowerManager::screenOn()
{
	db_msg("screenOn");
#ifdef HERO_OLEDUI
	mState = SCREEN_ON;
	return 0;
#else
	//screen_on_off_flag = true;
	int retval = 0;
	unsigned long args[4]={0};
	#ifdef DE2
			#ifdef NORMAL_STANDBY
				standbyResume();
				db_msg("standbyResume```");
			#else
				args[1] = DISP_OUTPUT_TYPE_LCD;
				retval = ioctl(mDispFd, DISP_DEVICE_SWITCH, args);
			#endif
	#else
		retval = ioctl(mDispFd, DISP_CMD_LCD_ON, args);
	#endif
		if (retval < 0) {
			db_error("fail to set screen on\n");
			return -1;
		}

	mState = SCREEN_ON;
	return 0;
#endif
}

int PowerManager::readyToOffScreen()
{
	#if 0
	status_t result;
	unsigned int time = 0;
	std::unique_lock<std::mutex> lck(mLock);
	//db_msg("waitRelative 0x%x", mOffTime);
	time = (mState == SCREEN_ON) ? mOffTime : TIME_INFINITE;
	if (mCon.wait_for(lck, std::chrono::seconds(time)) == std::cv_status::timeout) { //timeout
		db_msg("timeout\n");
		if (mState == SCREEN_ON) {
			if (!mCdrMain->isPlaying()) {
        		screenOff();
			} else {
				db_msg("isPlaying, not to close screen\n");
			}
		}
    } else {
		db_msg("signaled\n");
	}
	#endif
	return 0;
}

int set_scaling_governor(int mode)
{
	int fd = 0;
	int ret = -1;
    char scale_mode[20];
	
	fd =  open(SCALING_GOVERNOR, O_RDWR);
    if(fd < 0) {
		db_msg("open SCALING_GOVERNOR fail\n");  
		return -1;  
    }
	memset(scale_mode,0,sizeof(scale_mode));
	if(mode == PERFORMANCE)
	{
		strncpy(scale_mode,"performance",strlen("performance"));
	}
	else if(mode == USERSPACE) {
	    strncpy(scale_mode,"userspace",strlen("userspace"));
//		db_msg("scale_mode=%s\n",scale_mode);
	}
	else {
		strncpy(scale_mode,"userspace",strlen("userspace"));
	}
	ret = write(fd, scale_mode, strlen(scale_mode)+1);
//	ret = write(fd, "userspace", strlen("userspace"));

	if(ret > 0)
		db_msg("write SCALING_GOVERNOR success!\n");
	else {
		db_msg("write SCALING_GOVERNOR fail\n");
		db_msg("Error (%s),ret=%d,fd=%d",strerror(errno),ret,fd);
	}
    close(fd);  
    return (ret > 0) ? 0 : -1;  
}

int set_scaling_speed(const char *freq)
{
	int fd = 0;
	int ret = -1;
//	char freq[10]={"640000"};
	fd =  open(SCALING_SETSPEED, O_RDWR);
	if(fd < 0) {
		db_error("open SCALING_SETSPEED fail\n");  
		return -1;  
    }
	ret = write(fd, freq, strlen(freq));
	if(ret > 0)
		db_msg("write cpu_freq_value success!\n");
	else {
		db_error(" write Error (%s),ret=%d,fd=%d \n",strerror(errno),ret,fd);
	}	
    close(fd);  
    return (ret > 0) ? 0 : -1;  
}

unsigned int get_scaling_speed( )
{
	int fd = 0;
	int ret = -1;
	char cpu_freq_value[10] ="1007000";;
	fd =  open(CPUINFO_CUR_FREQ, O_RDONLY);
	if(fd < 0) {
		db_msg("open CPUINFO_CUR_FREQ fail\n");  
		return -1;  
    }
	ret = read(fd, cpu_freq_value, strlen(cpu_freq_value));
	if(ret > 0)
		db_msg("read cpu_freq_value success!\n");
	else {
		db_msg(" read Error (%s),ret=%d,fd=%d\n",strerror(errno),ret,fd);
    }	
	db_msg("cpu_freq_value =%s\n",cpu_freq_value);	
    close(fd);  
    return (ret > 0) ? 0 : -1;  
}

int _adjustCpuFreq()
{
	//get_scaling_speed();
	
	set_scaling_governor(USERSPACE);
	db_msg("set new freq %s", CPU_FREQ);
	set_scaling_speed(CPU_FREQ);
	get_scaling_speed();
    return 0;
}

void PowerManager::adjustCpuFreq()
{
	_adjustCpuFreq();
}

void PowerManager::watchdogThread()
{
	int fd = open("/dev/watchdog", O_RDWR);
    if (fd < 0)     {
            db_error("Watch dog device open error.");
            return;
    }

    db_msg("Before WDIOC_SETOPTIONS disable...............");
    int watchdog_enable = WDIOS_DISABLECARD;
    int io_ret = ioctl(fd,WDIOC_SETOPTIONS, &watchdog_enable);
    db_msg("After WDIOC_SETOPTIONS disable, ret = %d", io_ret);
    int time_out = 10;      //8s, min:1, max:16
    io_ret = ioctl(fd, WDIOC_SETTIMEOUT, &time_out);
    db_msg("WDIOC_SETTIMEOUT, ret = %d, time_out = %d", io_ret, time_out);
    io_ret = ioctl(fd,WDIOC_GETTIMEOUT, &time_out);
    db_msg("WDIOC_GETTIMEOUT, ret = %d, time_out = %d", io_ret, time_out);

	watchdog_enable = WDIOS_ENABLECARD;
    io_ret = ioctl(fd, WDIOC_SETOPTIONS, &watchdog_enable);
    db_msg("WDIOC_SETOPTIONS enable, ret = %d", io_ret);

    while(1) {
            ioctl(fd, WDIOC_KEEPALIVE, NULL);
            usleep(3000000);
			if(!isEnterStandby) {
				    watchdog_enable = WDIOS_DISABLECARD;
   					io_ret = ioctl(fd,WDIOC_SETOPTIONS, &watchdog_enable);
    				db_msg("After WDIOC_SETOPTIONS disable, ret = %d", io_ret);
				break;
			}
    }
	close(fd);
	return;
}

void PowerManager::watchdogRun()
{
	mWD = new WatchDogThread(this);
	mWD->startThread();
}
