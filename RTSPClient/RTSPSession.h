#pragma once
#include "DummySink.h"
#include "ourRTSPClient.h"

#define RTSP_CLIENT_VERBOSITY_LEVEL 1 // by default, print verbose output from each "RTSPClient"
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

class CRTSPSession
{
public:
	CRTSPSession(void);
	virtual ~CRTSPSession(void);

	char*  m_rtspUrl;//RTSP URL��ַ
	char const*  m_progName;//�ͻ�������
	TaskScheduler* scheduler;//live555��������
	UsageEnvironment* env;//live555��������

	RTSPClient* rtspClient;//RTSP�ͻ���
	static int thread_exit;	//ˢ���߳��˳���־
	char eventLoopWatchVariable;//������־


	HANDLE pStartRTSP_tid;//�ͻ��˴��߳�
	static DWORD WINAPI rtsp_thread_fun (LPVOID param);//�ͻ��˴��̺߳���
	void rtsp_fun();//������������

	//ֹͣRTSP�ͻ���
	int stopRTSPClient();
	//��RTSP�ͻ���
	int startRTSPClient(char const* progName, CString rtspURL);
	//��RTSP URL
	int openURL(UsageEnvironment& env, char const* progName, char const* rtspURL);
	//�ر�RTSP�ͻ���
	int CRTSPSession::RTSPClient_exit();


};

// Forward function definitions:
// RTSP 'response handlers':
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);

// Other event handler functions:
void subsessionAfterPlaying(void* clientData); // called when a stream's subsession (e.g., audio or video substream) ends
void subsessionByeHandler(void* clientData); // called when a RTCP "BYE" is received for a subsession
void streamTimerHandler(void* clientData);
// called at the end of a stream's expected duration (if the stream has not already signaled its end using a RTCP "BYE")

// Used to iterate through each stream's 'subsessions', setting up each one:
void setupNextSubsession(RTSPClient* rtspClient);

// Used to shut down and close a stream (including its "RTSPClient" object):
void shutdownStream(RTSPClient* rtspClient, int exitCode = 1);
