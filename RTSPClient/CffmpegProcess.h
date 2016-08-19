#pragma once
#include "RTSPSession.h"

//默认显示分辨率1600*600
#define MFCVIDEOSIZE_WIDTH  1600 
#define MFCVIDEOSIZE_HEIGHT 600


class CffmpegProcess
{
public:
	CffmpegProcess(void);
	virtual ~CffmpegProcess(void);

	int frameFinished;//成功解码标志
	int m_Isstop;   //是否停止 0表示播放 1表示停止
	int m_pause_play;  //0表示暂停 1表示播放
	int catch_frameFinished;//抓图标志
	bool closePlayThreadFlag;
	

	//ffmpeg//
	AVCodec *codec;//解码器
	SDL_Surface * m_screen; //sdl 屏幕显示结构体
	AVCodecContext *codec_context ;//视频解码context

	HANDLE pStartPlay_tid ; //播放线程句柄
	//ffmpeg初始化
	void ffmpeg_initDecoder(HWND hwnd);
	int ffmpeg_initQueue();//初始化流缓冲区队列
	void ffmpeg_queueDestroy();//释放流缓冲区队列
	void ffmpeg_flushQueue();//刷新pack初始化
	void ffmpeg_flushQueueDestroy();//释放刷新pack
	int startPlay_tidFun();//播放线程
	int closePlayThread();//关闭播放线程
	int ffmpeg_exit();//关闭ffmpeg
	int ffmpeg_init();//初始化ffmpeg
	static DWORD WINAPI decoder_thread(LPVOID param);//播放线程函数

	CQueueProcess queueProcess;
};

