// ttsdemo.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "qtts.h"

#define TRUE 1
#define FALSE 0

typedef int SR_DWORD;
typedef short int SR_WORD ;

//音频头部格式
struct wave_pcm_hdr
{
	char            riff[4];                        // = "RIFF"
	SR_DWORD        size_8;                         // = FileSize - 8
	char            wave[4];                        // = "WAVE"
	char            fmt[4];                         // = "fmt "
	SR_DWORD        dwFmtSize;                      // = 下一个结构体的大小 : 16

	SR_WORD         format_tag;              // = PCM : 1
	SR_WORD         channels;                       // = 通道数 : 1
	SR_DWORD        samples_per_sec;        // = 采样率 : 8000 | 6000 | 11025 | 16000
	SR_DWORD        avg_bytes_per_sec;      // = 每秒字节数 : dwSamplesPerSec * wBitsPerSample / 8
	SR_WORD         block_align;            // = 每采样点字节数 : wBitsPerSample / 8
	SR_WORD         bits_per_sample;          // = 量化比特数: 8 | 16

	char            data[4];                        // = "data";
	SR_DWORD        data_size;                // = 纯数据长度 : FileSize - 44 
} ;

//默认音频头部数据
struct wave_pcm_hdr default_pcmwavhdr = 
{
	{ 'R', 'I', 'F', 'F' },
	0,
	{'W', 'A', 'V', 'E'},
	{'f', 'm', 't', ' '},
	16,
	1,
	1,
	16000,
	32000,
	2,
	16,
	{'d', 'a', 't', 'a'},
	0  
};

int text_to_speech(const char* src_text ,const char* des_path ,const char* params)
{
	struct wave_pcm_hdr pcmwavhdr = default_pcmwavhdr;
	const char* sess_id = NULL;
	int ret = 0;
	unsigned int text_len = 0;
	char* audio_data;
	unsigned int audio_len = 0;
	int synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;
	FILE* fp = NULL;

	int pos = 0;			//用于标记上一次已经合成到的位置
	int loop_count = 0;		//用于标记，取了几次结果
	int upload_flow = 0,download_flow = 0;//上传流量和下载流量
	char param_value[32] = "";//参数值的字符串形式
	unsigned int value_len = 32;	//字符串长度或buffer长度

	printf("begin to synth...\n");
	if (NULL == src_text || NULL == des_path)
	{
		printf("params is null!\n");
		return -1;
	}
	text_len = (unsigned int)strlen(src_text);
	fp = fopen(des_path,"wb");
	if (NULL == fp)
	{
		printf("open file %s error\n",des_path);
		return -1;
	}
	sess_id = QTTSSessionBegin(params, &ret);
	if ( ret != MSP_SUCCESS )
	{
		printf("QTTSSessionBegin: qtts begin session failed Error code %d.\n",ret);
		return ret;
	}

	ret = QTTSTextPut(sess_id, src_text, text_len, NULL );
	if ( ret != MSP_SUCCESS )
	{
		printf("QTTSTextPut: qtts put text failed Error code %d.\n",ret);
		QTTSSessionEnd(sess_id, "TextPutError");
		return ret;
	}

	fwrite(&pcmwavhdr, 1, sizeof(pcmwavhdr), fp);

	while ( true )
	{
		audio_data = (char*)QTTSAudioGet( sess_id ,&audio_len , &synth_status , &ret );
		/****************获取合成位置*****************///如果不需要该功能，本段可以删除
		const char* audio_info = QTTSAudioInfo(sess_id);	//获取已经得到的音频的位置
		int ced = atoi(audio_info+4);						//位置信息的格式是ced=xxx，所以前四个字符被截断抛弃
		char text_complete[255];							//声明字符串，“已经合成完毕的文本”
		strcpy(text_complete ,src_text + pos);					//将原字符串复制到新字符串中，并对其前部进行截断，把之前合成的，非本次合成的文本抛弃
		text_complete[ced-pos]='\0';						//将新字符串进行后部截断，截断到ced提示的位置，这样这个字符串就是本次获取到的字符串了
		printf("[%d]:get result[err==%d / status==%d]:%s\n", (loop_count), ret, synth_status,text_complete);
		loop_count++;										//取结果次数自增
		pos = ced;											////合成位置更新
		/**************获取合成位置结束***************/
		if ( ret != MSP_SUCCESS )
		{
			printf("QTTSAudioGet: qtts get audio failed Error code %d.\n",ret);
			break;
		}
		/*********获取当前会话此时的流量信息**********///如果不需要该功能，本段可以删除
		value_len = 32;//value_len既是传入参数，又是传出参数，每次调用QTTSGetParam时要调整为buffer长度
		ret = QTTSGetParam(sess_id,"upflow",param_value,&value_len);//获取上行流量信息
		if ( ret != MSP_SUCCESS )
		{
			printf("QTTSGetParam: qtts get param failed Error code %d.\n",ret);
			break;
		}
		upload_flow = atoi(param_value);
		printf("upflow== %d Byte\n",upload_flow);
		value_len = 32;//value_len既是传入参数，又是传出参数，每次调用QTTSGetParam时要调整为buffer长度
		ret = QTTSGetParam(sess_id,"downflow",param_value,&value_len);//获取下行流量信息
		if ( ret != MSP_SUCCESS )
		{
			printf("QTTSGetParam: qtts get param failed Error code %d.\n",ret);
			break;
		}
		download_flow = atoi(param_value);
		printf("downflow== %d Byte\n",download_flow);
		/**************获取流量信息结束***************/
		fwrite(audio_data, 1, audio_len, fp);
		pcmwavhdr.data_size += audio_len;//修正pcm数据的大小
		if ( MSP_TTS_FLAG_DATA_END == synth_status )
		{
			printf("QTTSAudioGet: get end of data.\n");
			break;
		}
	}
	//修正pcm文件头数据的大小
	pcmwavhdr.size_8 += pcmwavhdr.data_size + 36;

	//将修正过的数据写回文件头部
	fseek(fp, 4, 0);
	fwrite(&pcmwavhdr.size_8,sizeof(pcmwavhdr.size_8), 1, fp);
	fseek(fp, 40, 0);
	fwrite(&pcmwavhdr.data_size,sizeof(pcmwavhdr.data_size), 1, fp);

	fclose(fp);

	ret = QTTSSessionEnd(sess_id, "Normal");
	if ( ret != MSP_SUCCESS )
	{
		printf("QTTSSessionEnd: qtts end failed Error code %d.\n",ret);
	}
	return ret;
}

int main(int argc, char* argv[])
{
	///APPID请勿随意改动
	const char* m_configs = "appid=xxxxxxxx";
	const char* text  = "讯飞语音，沟通无限。";
	const char*  filename = "iflytekTTS.wav";
	const char* param1 = "ssm=1,auf=audio/L16;rate=16000,vcn=xiaoyan,tte=UTF8";
	const char* param2 = "ssm=1,auf=audio/L16;rate=16000,vcn=xiaoyu,tte=UTF8";
	const char* param3 = "ssm=1,auf=audio/L16;rate=16000,vcn=Catherine,tte=UTF8";
	const char* param4 = "ssm=1,auf=audio/L16;rate=16000,vcn=Henry,tte=UTF8";
	const char* param  = param1;
	char* text_array = 0;
	int ret = 0;

	if(argc == 1 || argc >= 5)
	{
		printf("USAGE:\n");
		printf("    param1 is the text ,please use underline '_' to instead space;\n");
		printf("    param2 is the speech wave filename (*.wav) , default is iflytekTTS.wav;\n");
		printf("    param3 is the voice name , default is xiaoyan , you also can use xiaoyu, Catherine or Henry.\n");
		printf("Example:\n");
		printf("    ./iflytekTTS 你好 hello.wav xiaoyu\n");
		printf("    ./iflytekTTS 讯飞语音，沟通无限。 iflytek.wav xiaoyan\n");
		printf("    ./iflytekTTS How_are_you how.wav Catherine\n");
		printf("    ./iflytekTTS I_am_fine,thank_you. fine.wav Henry\n");
		printf("\n");
		printf("edzhu @ iflytek\n");
		return 0;
	}
	if(argc >=2)
	{
		int i=0;
		text = argv[1];
		text_array = (char*)malloc(strlen(argv[1])+1);
		if(0 != text_array)
		{
			for(;i<=strlen(argv[1]);i++)
			{
				if(argv[1][i]=='_')
					text_array[i]=' ';
				else
					text_array[i]=argv[1][i];
			}
			text_array[strlen(argv[1])]=0;
			text = text_array;
		}
		
	}
	if(argc >=3) filename	= argv[2];
	if(argc >=4)
	{
		if(0==strcmp(argv[3],"xiaoyu"))
		{
			param = param2;
		}
		else if(0==strcmp(argv[3],"Catherine"))
		{
			param = param3;
		}
		else if(0==strcmp(argv[3],"Henry"))
		{
			param = param4;
		}
	}
	//引擎初始化
	ret = QTTSInit( m_configs);
	if ( ret != MSP_SUCCESS )
	{
		printf("QTTSInit: failed, Error code %d.\n", ret);
		if(0 != text_array)
		{
			free(text_array);
		}
		return ret;
	}
	//合成文本
	ret = text_to_speech(text,filename,param);
	if ( ret != MSP_SUCCESS )
	{
		printf("text_to_speech: failed , Error code %d.\n",ret);
	}
	//引擎关闭
	QTTSFini();

	if(0 != text_array)
	{
		free(text_array);
	}
	return ret;
}

