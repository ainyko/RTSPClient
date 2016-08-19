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

	char*  m_rtspUrl;//RTSP URL地址
	char const*  m_progName;//客户端名称
	TaskScheduler* scheduler;//live555环境变量
	UsageEnvironment* env;//live555环境变量

	RTSPClient* rtspClient;//RTSP客户端
	static int thread_exit;	//刷新线程退出标志
	char eventLoopWatchVariable;//监听标志


	HANDLE pStartRTSP_tid;//客户端打开线程
	static DWORD WINAPI rtsp_thread_fun (LPVOID param);//客户端打开线程函数
	void rtsp_fun();//创建环境函数

	//停止RTSP客户端
	int stopRTSPClient();
	//打开RTSP客户端
	int startRTSPClient(char const* progName, CString rtspURL);
	//打开RTSP URL
	int openURL(UsageEnvironment& env, char const* progName, char const* rtspURL);
	//关闭RTSP客户端
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
