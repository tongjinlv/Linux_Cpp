#include <cutils/log.h>
#include <CdrServer.h>

#include "debug.h"
//#include <hardware_legacy/wifi_api.h>
//#include <netutils/ifc.h>
//#include <netutils/dhcp.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CdrServer"
#ifdef HTTP_SRV
typedef struct __httpHandler {
	FILE *fp;
} httpHandler;

int httpdStatCallback(int stat, void *argv, int argvLen, void *handler)
{
	httpHandler *httpHdr = (httpHandler*)handler;
	ALOGE("httpdStatCallback stat[%d], argvLen[%d]", stat, argvLen);
	switch (stat)
	{
	case POST_FILE_START:
		{
			char absfilename[256] = "";
			snprintf(absfilename, 256, "/mnt/extsd/%s", (char*)argv);
			ALOGE("httpdStatCallback filename[%s]", (char*)argv);
			httpHdr->fp = fopen(absfilename, "wb");
			if (httpHdr->fp == NULL) {
				ALOGE("httpdStatCallback open file error[%d:%s]", errno, strerror(errno));
			}
		}
		break;
	case POST_FILE_ERROR:
		if (httpHdr->fp != NULL) {
			fclose(httpHdr->fp);
			httpHdr->fp = NULL;
		}
		break;		
	case POST_FILE_END:
		if (httpHdr->fp != NULL) {
			fclose(httpHdr->fp);
			httpHdr->fp = NULL;
		}
		break;
	default:
		break;
	}
	return 0;
}

int httpdDataCallback(void *argv, int argvLen, void *handler)
{
	ALOGE("httpdDataCallback  argvLen[%d]",  argvLen);
	httpHandler *httpHdr = (httpHandler*)handler;
	fwrite(argv, 1, argvLen, httpHdr->fp);
	return 0;
}
#endif
static void qrDecCallback(void* data, int data_len, void* caller)
{

}
CdrServer::CdrServer(CdrMain *cdrMain)
{
	db_msg("CdrSever!!!\n");
	mNetServer = AWNetInterface::getInstance(1);
	if (!mNetServer) {
		db_msg("net init failed\n");
	}

	//mNetServer->init((char*)"F3J9A9NE4E6RUH6PSFZ1", (char*)"12345", 0x00000);
	mNetServer->init((char*)"F3J9A9NE4E6RUH6PSFZ1", (char*)"12345", 0x10000, (char*)"127.0.0.1", 0);
	mNetCallbackHandler = new AWNetCallbackHandler(this);
	mNetServer->registerNotificationHandler(mNetCallbackHandler);
	
	mCdrMain = cdrMain;
#ifdef HTTP_SRV
	mHttpHandler = malloc(sizeof(httpHandler));
	httpdCallback httpdCb = {httpdStatCallback, httpdDataCallback, mHttpHandler};
	startHttpd(&httpdCb);
//	mHttpServer = http_srv_init();
#endif
	db_msg("end!!!\n");
}

CdrServer::~CdrServer()
{
#ifdef HTTP_SRV
	stopHttpd();
//	http_srv_close(mHttpServer);
//	http_srv_release(mHttpServer);
#endif
	mNetServer->deInit();
}

CdrMain* CdrServer::getCdrMain()
{
	return mCdrMain;
}

int CdrServer::AWNetCallbackHandler::handleNetCmd(int sid, int cmd, char* buf)
{	
	ALOGD("~~~~~cmd :%#X", cmd);
	return mCdrServer->sessionControl(sid, cmd, buf);
}

int CdrServer::AWNetCallbackHandler::handleRecvVideoData(char *abFrameData, 
	int nFrameDataMaxSize,char *abFrameInfo, 
	int nFrameInfoMaxSize, unsigned int nFrameIdx)
{
	return 0;
}
int CdrServer::AWNetCallbackHandler::handleRecvFile(int SID, AWFileInfo* fileInfo, 
	char* buf, unsigned int buferLen)
{
	return 0;
}
int CdrServer::AWNetCallbackHandler::handleRecvAudioData(char *abAudioData, 
	int nAudioDataMaxSize,char *abFrameInfo, 
	int nFrameInfoMaxSize, unsigned int nFrameIdx)
{
	return 0;
}


int CdrServer::start()
{
#ifdef HTTP_SRV
//	registerGetDataOrEnd(mHttpServer, (void *)this, handleHttpServerMsg);
	int ret = 0;

//	pthread_t mBroadThread, mTcpSrvThread, iThreadId, iHttpThreadId;
//	int err = pthread_create(&iHttpThreadId, NULL, httpServerThread, this);
	return ret;
#endif
	return 0;
}

void* CdrServer::getSession()
{
	db_msg("cdrserver: mNetServer %p\n", mNetServer);
	return mNetServer;
}
static int encoder_start_flag = 0;
int CdrServer::sessionControl(int sid, int cmd, char* buf)
{
	db_msg("by hero *** cmd:%x, param_in:%p\n", cmd, buf);
	switch (cmd)
	{
		case NAT_CMD_TURN_OFF_WIFI:
			mCdrMain->wifiEnable(NET_CONNECT_MANAGER_DISABLE_CONN_WIFI);
			break;
		case NAT_CMD_SET_WIFI_PWD:
			{
				aw_cdr_wifi_setting *wifi_info = (aw_cdr_wifi_setting*)buf;
				mCdrMain->wifiEnable(NET_CONNECT_MANAGER_DISABLE_SOFTAP);
				db_msg("********wifi_info->ssid is %s   wifi_info->new_pwd is %s\n",wifi_info->ssid,wifi_info->new_pwd);
				return mCdrMain->wifiEnable(NET_CONNECT_MANAGER_ENABLE_SOFTAP,wifi_info->ssid,wifi_info->new_pwd,NULL);
				//return mCdrMain->wifiSetPwd(wifi_info->old_pwd, wifi_info->new_pwd);
			}
		
			break;
		case NAT_CMD_REQUEST_FILE:
#ifdef HTTP_SRV
//			{
//				aw_cdr_file_trans* in = (aw_cdr_file_trans*)buf;
//				int ret = 0;
//				//in->local_port = 31601;
//				//strcpy(in->local_ip, "192.168.100.1");
//				//if (in->from_to == NAT_CMD_FILE_TRANS_CMD_APK_TO_CDR)
//				//{
//					//db_msg("somebody ")
//				//	return 0;
//				//}
//				//if (in->from_to == NAT_CMD_FILE_TRANS_CMD_CDR_TO_APK)
//				{
//					ret = http_srv_getURL(mHttpServer, in->filename, "192.168.100.1", in->url);
//					db_msg("in->filename(%s), url(%s)", in->filename, in->url);
//					if (access(in->filename, R_OK) != 0)
//					{
//						ret = -1;
//						reply(sid, NAT_CMD_REQUEST_FILE_RESP, 0, 0);
//						return ret;
//					}
//					reply(sid, NAT_CMD_REQUEST_FILE_RESP, (int)in, sizeof(aw_cdr_file_trans));
//					return 0;
//				}
//			}
#endif
			break;
		case AW_IOTYPE_USER_IPCAM_START_ENCODER:
			{
				//db_msg("jjjjjjjjj 111\n");
				if (encoder_start_flag == 1)
				{
					break;
				}
				encoder_start_flag = 1;
				AWSMsg_AVIOCTRLStartStream *str = (AWSMsg_AVIOCTRLStartStream *)buf;
                		db_msg("str->stream_type=%d\n",str->stream_type);		
				if(mCdrMain->isDownLoadFlag == true)
				{
					db_msg("[debug_jason]:this is down loading!\n");
					break;
				}
				mCdrMain->sessionChange(true);	
			}
			break;
		case AW_IOTYPE_USER_IPCAM_STOP_ENCODER:
			//db_msg("jjjjjjjjj\n");
			if (encoder_start_flag == 0)
			{
				break;
			}	
			encoder_start_flag = 0;
			if(mCdrMain->isDownLoadFlag == true)
			{
				db_msg("[debug_jason]:this is down loading!\n");
				break;
			}	
			mCdrMain->sessionChange(false);
			break;
		case AW_IOTYPE_USER_IPCAM_START:
			if (encoder_start_flag == 1)
			{
				break;
			}
			mCdrMain->sessionChange(true);
			encoder_start_flag = 1;
			break;
		case AW_IOTYPE_USER_IPCAM_STOP:
			break;
		case AW_IOTYPE_USER_IPCAM_OFF_LINE:
			if (encoder_start_flag == 1)
			{
				mCdrMain->sessionChange(false);
				encoder_start_flag = 0;
			}
		default:
			break;
	}
	return mCdrMain->sessionControl(sid, cmd, buf);
}

int CdrServer::notify(int cmd,  char *buf, int len, int sid)
{
	int ret = 0;
	switch(cmd)
	{
		case NAT_CMD_EVENT_SOS_FILE:
		case NAT_CMD_INSERT_TF_CARD:
		case NAT_CMD_REMOVE_TF_CARD:
		case NAT_CMD_MOTION_TAKE_PHOTO_FINISH:
		case NAT_CMD_RECORD_TO_FULL_DISK:
			db_msg("++++++[debug_jaosn]:this is notify cmd 22 ++++++++++\n");
			ret = reply(NAT_BROADCAST_SID, cmd, buf, len);
		break;
	}

	return ret;

}

int CdrServer::reply(int sid, int cmd, char *buf, int buf_len)
{
	db_msg(" CdrServer::reply(sid(%d), cmd(%d), buf(%x), buf_len(%d))\n", sid, cmd, buf, buf_len);
	return mNetServer->awSendIOCtrl(sid, cmd,(const char *)buf, buf_len);
}
