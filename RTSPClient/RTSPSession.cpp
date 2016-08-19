#include "StdAfx.h"
#include "RTSPSession.h"

static int thread_exit = 0;

CRTSPSession::CRTSPSession(void)
{
	rtspClient = NULL;
	m_rtspUrl = NULL;
	m_progName = NULL;
	scheduler = NULL;
	env = NULL;
	pStartRTSP_tid = NULL; 
	//当eventLoopWatchVariable为1的时候，doEventLoop结束循环,为0的时候，阻塞执行
	eventLoopWatchVariable = 0;
}


CRTSPSession::~CRTSPSession(void)
{
}

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient) {
	return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession) {
	return env << subsession.mediumName() << "/" << subsession.codecName();
}

void usage(UsageEnvironment& env, char const* progName) {
	env << "Usage: " << progName << " <rtsp-url-1> ... <rtsp-url-N>\n";
	env << "\t(where each <rtsp-url-i> is a \"rtsp://\" URL)\n";
}

//打开RTSP URL 
int CRTSPSession::openURL(UsageEnvironment& env, char const* progName, char const* rtspURL) {
	// Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
	// to receive (even if more than stream uses the same "rtsp://" URL).


	if(!rtspClient)
	{
		rtspClient = CourRTSPClient::createNew(env, rtspURL, RTSP_CLIENT_VERBOSITY_LEVEL, progName);
		if (rtspClient == NULL) {
			env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << env.getResultMsg() << "\n";
			return 0 ;
		}
		/*if(rtspClient->fInputSocketNum == -1)
		{
		return 0;
		}
		*/

		// Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
		// Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
		// Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:

		rtspClient->sendDescribeCommand(continueAfterDESCRIBE);

	}

	return 1;
}

//开启RTSP客户端
int CRTSPSession::startRTSPClient(char const* progName, CString rtspURL)
{
	rtspClient = NULL;
	m_rtspUrl = NULL;
	m_progName = NULL;
	scheduler = NULL;
	env = NULL;
	//当eventLoopWatchVariable为1的时候，doEventLoop结束循环,为0的时候，阻塞执行
	eventLoopWatchVariable = 0;

	m_progName = progName;

	//获取rtspURL字符串长度
	int UrlLen = ::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(rtspURL), -1, NULL, 0, NULL, NULL);
	//m_rtspUrl转换为Unicode所需要的长度
	m_rtspUrl= new char[UrlLen+1];
	::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)(rtspURL), -1, m_rtspUrl, UrlLen, NULL, NULL);
	m_rtspUrl[UrlLen] = 0;


	//当eventLoopWatchVariable为1的时候，doEventLoop结束循环,为0的时候，阻塞执行
	eventLoopWatchVariable = 0;

	//pStartRTSP_tid = SDL_CreateThread(rtsp_thread_fun,(void*)this);
	pStartRTSP_tid  = CreateThread(NULL,0,rtsp_thread_fun,this,0,NULL);
	if (!pStartRTSP_tid)
	{
		//*MessageBox(_T("打开线程已经存在"), NULL, MB_OK );*/
		return 0;

	}

	return 1;
}


//停止RTSP客户端
int CRTSPSession::stopRTSPClient()
{
	eventLoopWatchVariable = 1;
	return 0;
}

DWORD WINAPI CRTSPSession::rtsp_thread_fun(LPVOID param)
{
	CRTSPSession *pThis = (CRTSPSession*)param;
	pThis->rtsp_fun();
	return NULL;
}

void CRTSPSession::rtsp_fun()
{
	//创建TaskScheduler对象   提供了任务调度功能
	scheduler = BasicTaskScheduler::createNew();
	//创建UsageEnvironment对象  提供了错误记录和错误报告的功能
	env = BasicUsageEnvironment::createNew(*scheduler);
	if (openURL(*env, m_progName, m_rtspUrl) == 1)
	{
		//阻塞执行，只有当eventLoopWatchVariable为非零值时才返回
		env->taskScheduler().doEventLoop(&eventLoopWatchVariable);
		// This function call does not return, unless, at some point in time, "eventLoopWatchVariable" gets set to something non-zero.

		//释放url资源		
		delete m_rtspUrl;

		//释放资源
		if (rtspClient)
		{
			shutdownStream(rtspClient,0);
		}
		rtspClient = NULL;
		env->reclaim(); 
		env = NULL;
		delete scheduler; 
		scheduler = NULL;

		if(pStartRTSP_tid)
		{
			//关闭播放线程
			CloseHandle(pStartRTSP_tid);
			pStartRTSP_tid = NULL;
		}
	}
}

//停止RTSP客户端
int CRTSPSession::RTSPClient_exit()
{
	//关闭rtspClient
	stopRTSPClient();
	Sleep(500);
	return 1;
}

// Implementation of the RTSP 'response handlers':

void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString)
{
	do {
		UsageEnvironment& env = rtspClient->envir(); // alias
		CStreamClientState& scs = ((CourRTSPClient*)rtspClient)->scs; // alias
		//CRTSPClientDlg *pDlg=(CRTSPClientDlg*)AfxGetApp()->m_pMainWnd;

		if (resultCode != 0) {
			env << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
			delete[] resultString;
			////打开打开图片按钮
			//pDlg->m_bitmap_open.SetIcon(pDlg->hIcon_open); 
			//pDlg->m_bitmap_catch.SetIcon(pDlg->hIcon_catch_b); 
			//pDlg->m_bitmap_play.SetIcon(pDlg->hIcon_play_b);

			//pDlg->GetDlgItem(btnOpen)->EnableWindow(TRUE);
			//pDlg->GetDlgItem(IDC_BUTTON_PAUSE)->EnableWindow(FALSE);
			//pDlg->GetDlgItem(IDC_btnCatchPic)->EnableWindow(FALSE);
			::AfxMessageBox(_T("打开RTSP视频流失败，请检查rtsp地址！"), NULL, MB_OK );
			break;
		}

		char* const sdpDescription = resultString;
		env << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";

		// Create a media session object from this SDP description:
		scs.session = MediaSession::createNew(env, sdpDescription);
		delete[] sdpDescription; // because we don't need it anymore
		if (scs.session == NULL) {
			env << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
			break;
		} else if (!scs.session->hasSubsessions()) {
			env << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
			break;
		}

		// Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
		// calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
		// (Each 'subsession' will have its own data source.)
		scs.iter = new MediaSubsessionIterator(*scs.session);
		setupNextSubsession(rtspClient);
		return;
	} while (0);

	// An unrecoverable error occurred with this stream.
	//shutdownStream(rtspClient);
}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString)
{
	do {
		UsageEnvironment& env = rtspClient->envir(); // alias
		CStreamClientState& scs = ((CourRTSPClient*)rtspClient)->scs; // alias

		if (resultCode != 0) {
			env << *rtspClient << "Failed to set up the \"" << *scs.subsession << "\" subsession: " << resultString << "\n";
			break;
		}

		env << *rtspClient << "Set up the \"" << *scs.subsession << "\" subsession (";
		if (scs.subsession->rtcpIsMuxed()) {
			env << "client port " << scs.subsession->clientPortNum();
		} else {
			env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
		}
		env << ")\n";

		// Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
		// (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
		// after we've sent a RTSP "PLAY" command.)

		scs.subsession->sink = CDummySink::createNew(env, *scs.subsession, rtspClient->url());
		// perhaps use your own custom "MediaSink" subclass instead
		if (scs.subsession->sink == NULL) {
			env << *rtspClient << "Failed to create a data sink for the \"" << *scs.subsession
				<< "\" subsession: " << env.getResultMsg() << "\n";
			break;
		}

		env << *rtspClient << "Created a data sink for the \"" << *scs.subsession << "\" subsession\n";
		scs.subsession->miscPtr = rtspClient; // a hack to let subsession handle functions get the "RTSPClient" from the subsession 
		scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
			subsessionAfterPlaying, scs.subsession);
		// Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
		if (scs.subsession->rtcpInstance() != NULL) {
			scs.subsession->rtcpInstance()->setByeHandler(subsessionByeHandler, scs.subsession);
		}
	} while (0);
	delete[] resultString;

	// Set up the next subsession, if any:
	setupNextSubsession(rtspClient);
}

void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString)
{
	bool success = False;

	do {
		UsageEnvironment& env = rtspClient->envir(); // alias
		CStreamClientState& scs = ((CourRTSPClient*)rtspClient)->scs; // alias

		if (resultCode != 0) {
			env << *rtspClient << "Failed to start playing session: " << resultString << "\n";
			break;
		}

		// Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
		// using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
		// 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
		// (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
		if (scs.duration > 0) {
			unsigned const delaySlop = 2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
			scs.duration += delaySlop;
			unsigned uSecsToDelay = (unsigned)(scs.duration*1000000);
			scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)streamTimerHandler, rtspClient);
		}

		env << *rtspClient << "Started playing session";
		if (scs.duration > 0) {
			env << " (for up to " << scs.duration << " seconds)";
		}
		env << "...\n";

		success = True;
	} while (0);
	delete[] resultString;

	if (!success) {
		// An unrecoverable error occurred with this stream.
		shutdownStream(rtspClient);
	}
}



// By default, we request that the server stream its data using RTP/UDP.
// If, instead, you want to request that the server stream via RTP-over-TCP, change the following to True:
#define REQUEST_STREAMING_OVER_TCP False

void setupNextSubsession(RTSPClient* rtspClient)
{
	UsageEnvironment& env = rtspClient->envir(); // alias
	CStreamClientState& scs = ((CourRTSPClient*)rtspClient)->scs; // alias

	scs.subsession = scs.iter->next();
	if (scs.subsession != NULL) {
		if (!scs.subsession->initiate()) {
			env << *rtspClient << "Failed to initiate the \"" << *scs.subsession << "\" subsession: " << env.getResultMsg() << "\n";
			setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
		} else {
			env << *rtspClient << "Initiated the \"" << *scs.subsession << "\" subsession (";
			if (scs.subsession->rtcpIsMuxed()) {
				env << "client port " << scs.subsession->clientPortNum();
			} else {
				env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
			}
			env << ")\n";

			if (scs.subsession->rtpSource() != NULL) 
			{
				// Because we're saving the incoming data, rather than playing
				// it in real time, allow an especially large time threshold
				// (1 second) for reordering misordered incoming packets:
				unsigned const thresh = 100000; // 1 second
				scs.subsession->rtpSource()->setPacketReorderingThresholdTime(thresh);

				// Set the RTP source's OS socket buffer size as appropriate - either if we were explicitly asked (using -B),
				// or if the desired FileSink buffer size happens to be larger than the current OS socket buffer size.
				// (The latter case is a heuristic, on the assumption that if the user asked for a large FileSink buffer size,
				// then the input data rate may be large enough to justify increasing the OS socket buffer size also.)
				int socketNum = scs.subsession->rtpSource()->RTPgs()->socketNum();
				unsigned curBufferSize = getReceiveBufferSize(env, socketNum);
				unsigned int fileSinkBufferSize = 20000000;
				if ( fileSinkBufferSize > curBufferSize) {
					unsigned newBufferSize =  fileSinkBufferSize;
					newBufferSize = setReceiveBufferTo(env, socketNum, newBufferSize);
					printf("actual buffer size = %d\n", newBufferSize);
				}
			}
			// Continue setting up this subsession, by sending a RTSP "SETUP" command:
			rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, False, REQUEST_STREAMING_OVER_TCP);
		}
		return;
	}

	// We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
	if (scs.session->absStartTime() != NULL) {
		// Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
		rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY, scs.session->absStartTime(), scs.session->absEndTime());
	} else {

		scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
		rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
	}
}
// Implementation of the other event handlers:

void subsessionAfterPlaying(void* clientData) 
{
	MediaSubsession* subsession = (MediaSubsession*)clientData;
	RTSPClient* rtspClient = (RTSPClient*)(subsession->miscPtr);

	// Begin by closing this subsession's stream:
	Medium::close(subsession->sink);
	subsession->sink = NULL;

	// Next, check whether *all* subsessions' streams have now been closed:
	MediaSession& session = subsession->parentSession();
	MediaSubsessionIterator iter(session);
	while ((subsession = iter.next()) != NULL) {
		if (subsession->sink != NULL) return; // this subsession is still active
	}

	// All subsessions' streams have now been closed, so shutdown the client:
	shutdownStream(rtspClient);
}

void subsessionByeHandler(void* clientData) 
{
	MediaSubsession* subsession = (MediaSubsession*)clientData;
	RTSPClient* rtspClient = (RTSPClient*)subsession->miscPtr;
	UsageEnvironment& env = rtspClient->envir(); // alias

	env << *rtspClient << "Received RTCP \"BYE\" on \"" << *subsession << "\" subsession\n";

	// Now act as if the subsession had closed:
	subsessionAfterPlaying(subsession);
}

void streamTimerHandler(void* clientData)
{
	CourRTSPClient* rtspClient = (CourRTSPClient*)clientData;
	CStreamClientState& scs = rtspClient->scs; // alias

	scs.streamTimerTask = NULL;

	// Shut down the stream:
	shutdownStream(rtspClient);
}

void shutdownStream(RTSPClient* rtspClient, int exitCode) 
{
	UsageEnvironment& env = rtspClient->envir(); // alias
	CStreamClientState& scs = ((CourRTSPClient*)rtspClient)->scs; // alias

	// First, check whether any subsessions have still to be closed:
	if (scs.session != NULL) { 
		bool someSubsessionsWereActive = False;
		MediaSubsessionIterator iter(*scs.session);
		MediaSubsession* subsession;

		while ((subsession = iter.next()) != NULL) {
			if (subsession->sink != NULL) {
				Medium::close(subsession->sink);
				subsession->sink = NULL;

				if (subsession->rtcpInstance() != NULL) {
					subsession->rtcpInstance()->setByeHandler(NULL, NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
				}

				someSubsessionsWereActive = True;
			}
		}

		if (someSubsessionsWereActive) {
			// Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
			// Don't bother handling the response to the "TEARDOWN".
			rtspClient->sendTeardownCommand(*scs.session, NULL);
		}
	}

	env << *rtspClient << "Closing the stream.\n";
	Medium::close(rtspClient);
	// Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.


	// The final stream has ended, so exit the application now.
	// (Of course, if you're embedding this code into your own application, you might want to comment this out,
	// and replace it with "eventLoopWatchVariable = 1;", so that we leave the LIVE555 event loop, and continue running "main()".)

	//exit(exitCode);

}

int sfp_refresh_thread(void *opaque)//  刷新线程
{
	while (thread_exit==0) {
		SDL_Event event;
		event.type = SFM_REFRESH_EVENT;
		SDL_PushEvent(&event);
		//Wait 40 ms
		SDL_Delay(40);
	}
	return 0;
}

