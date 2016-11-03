
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "qisr.h"
#include "msp_types.h"
#include "msp_errors.h"



#define MAX_KEYWORD_LEN 4096
char GrammarID[128];

const char*  get_audio_file(void)
{
	return "03.wav";
}


void release_grammar(const char** grammar)
{
	if (*grammar)
	{
		free((void*)*grammar);
		*grammar = NULL;
	}	
}

int run_asr(const char* asrfile ,  const char* param , const char* grammar)
{
	char rec_result[1024*4] = {0};
	const char *sessionID;
	FILE *f_pcm = NULL;
	char *pPCM = NULL;
	int lastAudio = 0 ;
	int audStat = 2 ;
	int epStatus = 0;
	int recStatus = 0 ;
	long pcmCount = 0;
	long pcmSize = 0;
	int ret = 0 ;
	char    audio_data[ 5120 ] = “”; 
unsigned int  audio_len = 0; 
int      audio_status = 2; 
int      ep_status = 0; 
int      rec_status = 0;
	
	
	char* configs = "server_url=dev.voicecloud.cn/index.htm,appid=545bb5b5,vad_enable=true"; 
	ret = QISRInit( configs );
	sessionID = QISRSessionBegin(NULL, param, &ret); //asr
	if(ret !=0)
	{
		printf("QISRSessionBegin Failed,ret=%d\n",ret);
	}

    f_pcm = fopen(asrfile, "rb");
	if (NULL != f_pcm) {
		fseek(f_pcm, 0, SEEK_END);
		pcmSize = ftell(f_pcm);
		fseek(f_pcm, 0, SEEK_SET);
		pPCM = (char *)malloc(pcmSize);
		fread((void *)pPCM, pcmSize, 1, f_pcm);
		fclose(f_pcm);
		f_pcm = NULL;
	}
	while (1) {
		unsigned int len = 6400;
        unsigned int audio_len = 6400;
		if (pcmSize < 12800) {
			len = pcmSize;
			lastAudio = 1;
		}
		audStat = 2;//MSP_AUDIO_SAMPLE_CONTINUE有后继音频
		if (pcmCount == 0)
			audStat = 1;//MSP_AUDIO_SAMPLE_FIRST 音频开始

		if (len<=0)
		{
			break;
		}
            
		printf("\ncsid=%s,count=%d,aus=%d,",sessionID,pcmCount/audio_len,audStat);
		ret = QISRAudioWrite(sessionID, (const void *)&pPCM[pcmCount], len, audStat, &epStatus, &recStatus);
		printf("eps=%d,rss=%d,ret=%d",epStatus,recStatus,ret);
		if (ret != 0)
			break;
		pcmCount += (long)len;
		pcmSize -= (long)len;
		if (recStatus == 0) {
			const char *rslt = QISRGetResult(sessionID, &recStatus, 0, &ret);
			if(ret !=0)
			{
				printf("QISRGetResult Failed,ret=%d\n",ret);
				break;
			}
			if (NULL != rslt)
				strcat(rec_result,rslt);
		}
		if (epStatus == MSP_EP_AFTER_SPEECH)
			break;
		usleep(150000);
	}
	ret=QISRAudioWrite(sessionID, (const void *)NULL, 0, 4, &epStatus, &recStatus);
	if (ret !=0)
	{
		printf("QISRAudioWrite Failed,ret=%d\n",ret);
	}
	free(pPCM);
	pPCM = NULL;
	while (recStatus != 5 && ret == 0) {
		const char *rslt = QISRGetResult(sessionID, &recStatus, 0, &ret);
		if (NULL != rslt)
		{
			strcat(rec_result,rslt);
		}
		usleep(150000);
	}
    ret=QISRSessionEnd(sessionID, NULL);
	if(ret !=MSP_SUCCESS)
	{
		printf("QISRSessionEnd Failed, ret=%d\n",ret);
	}	
	printf("\n=============================================================\n");
	printf("The result is: %s\n",rec_result);
	printf("=============================================================\n");
	usleep(100000);
}

int main(int argc, char* argv[])
{
	const char* login_config = "appid = 545bb5b5, work_dir =   .  ";
	const char* param = "rst=plain,rse=utf8,sub=asr,ssm=1,aue=speex-wb,auf=audio/L16;rate=16000,ent=sms16k";//注意sub=asr,16k音频aue=speex-wb，8k音频识别aue=speex，
	int ret = 0 ;
	char key = 0 ;
	const char* asrfile = get_audio_file();
	ret = run_asr(asrfile, param, NULL);
	if(ret != MSP_SUCCESS)
	{
		printf("run_asr with errorCode: %d \n", ret);
		return 0;
	}
	return 0;
}

