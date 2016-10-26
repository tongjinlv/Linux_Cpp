#ifndef __MSP_TYPES_H__
#define __MSP_TYPES_H__

#if !defined(MSPAPI)
#if defined(WIN32)
	#define MSPAPI __stdcall
#else
	#define MSPAPI
#endif /* WIN32 */
#endif /* MSPAPI */

#include "msp_errors.h"
/**
 *  MSPSampleStatus indicates how the sample buffer should be handled
 *  MSP_AUDIO_SAMPLE_FIRST		- The sample buffer is the start of audio
 *								  If recognizer was already recognizing, it will discard
 *								  audio received to date and re-start the recognition
 *  MSP_AUDIO_SAMPLE_CONTINUE	- The sample buffer is continuing audio
 *  MSP_AUDIO_SAMPLE_LAST		- The sample buffer is the end of audio
 *								  The recognizer will cease processing audio and
 *								  return results
 *  Note that sample statii can be combined; for example, for file-based input
 *  the entire file can be written with SAMPLE_FIRST | SAMPLE_LAST as the
 *  status.
 *  Other flags may be added in future to indicate other special audio
 *  conditions such as the presence of AGC
 */
enum
{
	MSP_AUDIO_SAMPLE_INIT           = 0x00,
    MSP_AUDIO_SAMPLE_FIRST          = 0x01,
    MSP_AUDIO_SAMPLE_CONTINUE       = 0x02,
    MSP_AUDIO_SAMPLE_LAST           = 0x04,
};

/*
 *  The enumeration MSPRecognizerStatus contains the recognition status
 *  MSP_REC_STATUS_SUCCESS				- successful recognition with partial results
 *  MSP_REC_STATUS_NO_MATCH				- recognition rejected
 *  MSP_REC_STATUS_INCOMPLETE			- recognizer needs more time to compute results
 *  MSP_REC_STATUS_NON_SPEECH_DETECTED	- discard status, no more in use
 *  MSP_REC_STATUS_SPEECH_DETECTED		- recognizer has detected audio, this is delayed status
 *  MSP_REC_STATUS_COMPLETE				- recognizer has return all result
 *  MSP_REC_STATUS_MAX_CPU_TIME			- CPU time limit exceeded
 *  MSP_REC_STATUS_MAX_SPEECH			- maximum speech length exceeded, partial results may be returned
 *  MSP_REC_STATUS_STOPPED				- recognition was stopped
 *  MSP_REC_STATUS_REJECTED				- recognizer rejected due to low confidence
 *  MSP_REC_STATUS_NO_SPEECH_FOUND		- recognizer still found no audio, this is delayed status
 */
enum
{
	MSP_REC_STATUS_SUCCESS              = 0, //识别成功，此时用户可以调用QISRGetResult来获取（部分）结果
	MSP_REC_STATUS_NO_MATCH             = 1, //识别结束，没有识别结果
	MSP_REC_STATUS_INCOMPLETE			= 2, //正在识别中
	MSP_REC_STATUS_NON_SPEECH_DETECTED  = 3, //保留
	MSP_REC_STATUS_SPEECH_DETECTED      = 4, //发现有效音频
	MSP_REC_STATUS_COMPLETE				= 5, //识别结束
	MSP_REC_STATUS_MAX_CPU_TIME         = 6, //保留
	MSP_REC_STATUS_MAX_SPEECH           = 7, //保留
	MSP_REC_STATUS_STOPPED              = 8, //保留
	MSP_REC_STATUS_REJECTED             = 9, //保留
	MSP_REC_STATUS_NO_SPEECH_FOUND      = 10, //没有发现音频
	MSP_REC_STATUS_FAILURE = MSP_REC_STATUS_NO_MATCH,
};

/**
 * The enumeration MSPepState contains the current endpointer state
 *  MSP_EP_LOOKING_FOR_SPEECH	- Have not yet found the beginning of speech
 *  MSP_EP_IN_SPEECH			- Have found the beginning, but not the end of speech
 *  MSP_EP_AFTER_SPEECH			- Have found the beginning and end of speech
 *  MSP_EP_TIMEOUT				- Have not found any audio till timeout
 *  MSP_EP_ERROR				- The endpointer has encountered a serious error
 *  MSP_EP_MAX_SPEECH			- Have arrive the max size of speech
 */
enum
{
	MSP_EP_LOOKING_FOR_SPEECH   = 0, // 还没有检测到音频的前端点
	MSP_EP_IN_SPEECH            = 1, // 已经检测到了音频前端点,正在进行正常的音频处理
	MSP_EP_AFTER_SPEECH         = 3, // 检测到音频的后端点，后继的音频会被MSC忽略。
	MSP_EP_TIMEOUT              = 4, // 超时
	MSP_EP_ERROR                = 5, // 出现错误
	MSP_EP_MAX_SPEECH           = 6, // 音频过大
	MSP_EP_IDLE                 = 7  // internal state after stop and before start
};

/* Synthesizing process flags */
enum
{
    MSP_TTS_FLAG_STILL_HAVE_DATA        = 1, //
    MSP_TTS_FLAG_DATA_END               = 2, //
    MSP_TTS_FLAG_CMD_CANCELED           = 4, //
};

#endif /* __MSP_TYPES_H__ */
