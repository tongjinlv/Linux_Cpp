#include <stdio.h> 
#include <string.h> 
#include <unistd.h>
#include "qisr.h"
#include "msp_types.h"
#include "msp_errors.h"

#define BLOCK_LEN 5 * 1024 

int main(int argc, char* argv[]) 
{	
	const char *session_id;
	const char* configs = "server_url=dev.voicecloud.cn/index.htm,appid=545bb5b5,vad_enable=true,max_audio_size=262144"; 
	//max_audio_size 最大音频长度，单位为 Byte，最大值为 1MB，缺省值为262144（256KB）
	const char* param= "rst=json,rse=utf8,sub=iat,ssm=1,audio/L16;rate=16000,ent=sms16k" ;
	
	const char* filename= "4.wav";
	FILE *f_pcm = NULL;
	char *pPCM = NULL;
	
	char buff_data[ BLOCK_LEN ]; //音频数据缓冲区
	unsigned int audio_len =0; //音频数据长度
	int audio_status = 2; //本次识别的音频是否发送完毕
	int ep_status = 0;//端点检测（End-point)状态
	int rslt_status = 0;//识别器所处的状态
	const char* rec_result = 0;
	
	
	//读取文件
	f_pcm = fopen(filename, "rb");
	
	//初始化ISR
	int errorcode = 0 ;
	errorcode = QISRInit( configs );
	if(MSP_SUCCESS != errorcode ) 
	{ 
	  printf( "QISRSessionBegin failed, error code is: %d", errorcode ); 
	}
	
	//开始会话
	session_id = QISRSessionBegin(NULL, param, &errorcode); //
	if(MSP_SUCCESS != errorcode ) 
	{ 
	  printf( "QISRSessionBegin failed, error code is: %d", errorcode ); 
	}
	

	while( MSP_AUDIO_SAMPLE_LAST != audio_status ) //当发送最后一个区块后停止ISR_AUDIO_SAMPLE_LAST
	{ 
		// 每次读取BLOCK_LEN长度的数据 BLOCK_LEN 为5KB 大小的 16KPCM
		audio_len = fread( buff_data, 1, BLOCK_LEN, f_pcm ); 
		
		// 如果缓冲区充满则后续还有数据MSP_AUDIO_SAMPLE_CONTINUE     否则为最后数据MSP_AUDIO_SAMPLE_LAST
		audio_status = ( audio_len == BLOCK_LEN ) ?   MSP_AUDIO_SAMPLE_CONTINUE :MSP_AUDIO_SAMPLE_LAST; 
		// 发送数据 BLOCK_LEN 
		errorcode = QISRAudioWrite( session_id, buff_data, audio_len , audio_status, &ep_status, &rslt_status ); 
		if(MSP_SUCCESS != errorcode ) 
		{ 
		  printf( "QISRSessionBegin failed, error code is %d\n", errorcode ); 
		  rslt_status = MSP_REC_STATUS_COMPLETE; 
		  break; 
		} 
		
		printf( "write audio data ok! len=%d, status=%d\n", audio_len, audio_status );
		
		/* 已经有结果缓存在 MSC 中了，可以获取了  */ 

		if(MSP_REC_STATUS_COMPLETE == rslt_status ) 
		{ 
			printf("rslt_status:%d 获取...\n", rslt_status );
			rec_result = QISRGetResult( session_id, &rslt_status, 5000, &errorcode ); 
			if( MSP_SUCCESS != errorcode ) 
			{ 
			printf( "QISRGetResult failed, error code is %d\n", errorcode ); 
			rslt_status = MSP_REC_STATUS_COMPLETE; 
			break; 
			} 
 
			if( NULL != rec_result ) 
			{ 
				printf( "got a result: %s\n", rec_result ); 
			} 
		 
			/* 全部结果已经取完了  */ 
			if( MSP_REC_STATUS_COMPLETE == rslt_status ) 
			{ 
				printf( "the result has been got completely!\n" ); 
				break; 
			} 
		}
		
		if(MSP_EP_AFTER_SPEECH == ep_status )  /* 检测到音频后端点，停止发送音频  */ 
		{ 
			printf( "end point of speech has been detected!" );		
			break;
		}
		
		/* 如果是实时采集音频，可以省略此操作 5KB 大小的 16KPCM持续的时间是 160 毫秒  */  
		//等待下一个160ms音频数据冲入
		usleep( 160 );
	}
	fclose( f_pcm );
	

	/* 获取余下的识别结果  */ 
	while( MSP_REC_STATUS_COMPLETE != rslt_status ) 
	{ 	
		rec_result = QISRGetResult( session_id, &rslt_status, 5000, &errorcode );	
		if(MSP_SUCCESS != errorcode ) 
		{ 
			printf( "QISRGetResult failed, error code is: %d\n", errorcode ); 
			break; 
		} 
		
		if(MSP_REC_STATUS_SUCCESS == rslt_status){
			printf("识别成功,开始获取结果...\n");
		}
		if( NULL != rec_result ) 
		{ 
			printf( "got a result: %s\n", rec_result ); 
		} 
		if(MSP_REC_STATUS_COMPLETE == rslt_status){
			printf("识别完毕...\n");
		}
		/* sleep 一下很有必要，防止MSC 端无缓存的识别结果时浪费 CPU 资源  */ 
		usleep( 200 ); 
	} 
 

	/* 结束会话，释放资源  */ 
	errorcode = QISRSessionEnd( session_id, "normal end" ); 
	// if( NULL == f_speech ) 
	// { 
		// printf( "QISRSessionEnd failed, error code is %d\n", errorcode ); 
	// } 
	session_id = NULL; 
	errorcode = QISRFini(); //逆初始化
	if(MSP_SUCCESS != errorcode ) 
	{ 
		printf( "QISRFini failed, error code is %d\n", errorcode ); 
	} 
	 
	return 0; 
}