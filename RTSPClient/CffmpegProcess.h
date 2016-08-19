#pragma once
#include "RTSPSession.h"

//Ĭ����ʾ�ֱ���1600*600
#define MFCVIDEOSIZE_WIDTH  1600 
#define MFCVIDEOSIZE_HEIGHT 600


class CffmpegProcess
{
public:
	CffmpegProcess(void);
	virtual ~CffmpegProcess(void);

	int frameFinished;//�ɹ������־
	int m_Isstop;   //�Ƿ�ֹͣ 0��ʾ���� 1��ʾֹͣ
	int m_pause_play;  //0��ʾ��ͣ 1��ʾ����
	int catch_frameFinished;//ץͼ��־
	bool closePlayThreadFlag;
	

	//ffmpeg//
	AVCodec *codec;//������
	SDL_Surface * m_screen; //sdl ��Ļ��ʾ�ṹ��
	AVCodecContext *codec_context ;//��Ƶ����context

	HANDLE pStartPlay_tid ; //�����߳̾��
	//ffmpeg��ʼ��
	void ffmpeg_initDecoder(HWND hwnd);
	int ffmpeg_initQueue();//��ʼ��������������
	void ffmpeg_queueDestroy();//�ͷ�������������
	void ffmpeg_flushQueue();//ˢ��pack��ʼ��
	void ffmpeg_flushQueueDestroy();//�ͷ�ˢ��pack
	int startPlay_tidFun();//�����߳�
	int closePlayThread();//�رղ����߳�
	int ffmpeg_exit();//�ر�ffmpeg
	int ffmpeg_init();//��ʼ��ffmpeg
	static DWORD WINAPI decoder_thread(LPVOID param);//�����̺߳���

	CQueueProcess queueProcess;
};

