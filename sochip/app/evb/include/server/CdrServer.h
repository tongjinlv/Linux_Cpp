#ifndef _CDR_SERVER_H_
#define _CDR_SERVER_H_

#include "windows.h"
#ifdef HTTP_SRV
#include <httpd.h>
#endif
#include <include_net/AWNetInterface.h>
#include <include_net/AWNetCmdData.h>


class CdrMain;
class CdrServer
{
public:
	CdrServer(CdrMain *cdrMain);
	~CdrServer();
	int start();
	void *getSession();
	int sessionControl(int sid, int cmd, char* buf);
	int notify(int cmd, char* buf, int len, int sid = -1);
	int reply(int sid, int cmd, char* buf, int buf_len);
	CdrMain * getCdrMain();
//	http_srv* mHttpServer;
private:
	AWNetInterface *mNetServer;
	CdrMain *mCdrMain;

	class AWNetCallbackHandler : public AWNetNotificationHandler
	{
	public:
		AWNetCallbackHandler(CdrServer *cdrsrv)
		{
			mCdrServer = cdrsrv;
		};
		int handleNetCmd(int SID, int CommdID, char* buf);
		int handleRecvVideoData(char *abFrameData, int nFrameDataMaxSize,char *abFrameInfo, 
								int nFrameInfoMaxSize, unsigned int nFrameIdx);
		int handleRecvFile(int SID, AWFileInfo* fileInfo, char* buf, unsigned int buferLen);
		int handleRecvAudioData(char *abAudioData, int nAudioDataMaxSize,char *abFrameInfo, 
								int nFrameInfoMaxSize, unsigned int nFrameIdx);
	private:
		CdrServer* mCdrServer;
	};
	AWNetCallbackHandler *mNetCallbackHandler;
	void *mHttpHandler;
};

#endif	//_CDR_SERVER_H_
