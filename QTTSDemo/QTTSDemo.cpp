// QTTSDemo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <msp_cmn.h>
#include <msp_errors.h>
#include <qtts.h>
#include <string.h>
#include <Windows.h>

#ifdef _WIN64
#pragma comment(lib,"msc_x64.lib")
#else
#pragma comment(lib,"msc.lib")
#endif  //_WIN64

#pragma comment(lib,"winMM.lib")   //可播放音频

/*wav音频头部格式*/
typedef struct _wave_pcm_hdr
{
	char		riff[4];			// = "RIFF"
	int			size_8;				// = FileSize-8
	char		wave[4];			// = "WAVE"
	char		fmt[4];				// = "fmt"
	int			fmt_size;			// = 下一个结构体的大小 : 16

	short int	format_tag;			// = PCM : 1
	short int	channels;			// = 通道数 : 1
	int			samples_per_sec;	// = 采样率 : 8000 | 6000 | 11025 | 16000
	int			avg_bytes_per_sec;	// = 每秒字节数 : samples_per_sec * bits_per_sample
	short int	block_align;		// = 每采样点字节数 : wBitsperSample / 8
	short int	bits_per_sample;	// = 量化比特数 : 8 | 16

	char		data[4];			// = "data"
	int			data_size;			// = 纯数据长度 : FileSize - 44
} wave_pcm_hdr;

/* 默认wav音频头部数据 */
wave_pcm_hdr default_wav_hdr =
{
	{'R','I','F','F'},
	0,
	{'W','A','V','E'},
	{'f','m','t',' '},
	16,
	1,
	1,
	16000,
	32000,
	2,
	16,
	{'d','a','t','a'},
	0
};
int main()
{
	//登录
	const char* usr = NULL;
	const char* pwd = NULL;
	const char* lgi_param = "appid =  5953c9e6";
	int ret = MSPLogin(usr, pwd, lgi_param);
	if (MSP_SUCCESS != ret)
	{
		printf("MSPLogin failed, error code is: %d", ret);
	}

	//开始合成
	const char * ssb_param = "voice_name = xiaoyan, aue = speex-wb;7, sample_rate = 16000, speed = 50, volume = 50, pitch = 50, rdn = 2";
	ret = -1;
	const char * sessionID = QTTSSessionBegin(ssb_param, &ret);
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSSessionBegin failed, error code is : %d", ret);
	}

	//设置待合成文本
	const char* src_text = "操你妈";
	unsigned int text_len = strlen(src_text); //textLen参数为合成文本所占字节数
	ret = QTTSTextPut(sessionID, src_text, text_len, NULL);
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSTextPut failed, error code is : %d", ret);
	}

	//获取合成音频wav
	FILE* fp = fopen("qttsDemo.wav", "wb");
	fwrite(&default_wav_hdr, sizeof(default_wav_hdr), 1, fp);
	unsigned int audio_len = 0;
	int synth_status = 0;
	while (1)
	{
		const void * data = QTTSAudioGet(sessionID, &audio_len, &synth_status, &ret);
		if (NULL != data)
		{
			fwrite(data, audio_len, 1, fp);
			default_wav_hdr.data_size += audio_len;    //计算整体的长度
		}
		if (MSP_TTS_FLAG_DATA_END == synth_status || MSP_SUCCESS != ret)
		{
			break;
		}
	}
	
	default_wav_hdr.size_8 += default_wav_hdr.data_size + (sizeof(default_wav_hdr) - 8);
	fseek(fp, 4, 0);
	fwrite(&default_wav_hdr.size_8, sizeof(default_wav_hdr.size_8), 1, fp);  //写入size_8的值
	fseek(fp, 40, 0);   //将文件指针偏移到储存data_size值的位置
	fwrite(&default_wav_hdr.data_size, sizeof(default_wav_hdr.data_size), 1, fp);  //写入data_size的值
	fclose(fp);

	ret = QTTSSessionEnd(sessionID, "normal end");
	if (MSP_SUCCESS != ret)
	{
		printf("QTTSSessionEnd failed, error code is : %d", ret);
	}
	PlaySound("qttsDemo.wav", NULL, SND_ASYNC);

	//记得退出登录
	ret = MSPLogout();
	if (MSP_SUCCESS != ret)
	{
		printf("MSPLogout failed, error code is: %d", ret);
	}

	system("pause");
    return 0;
}

