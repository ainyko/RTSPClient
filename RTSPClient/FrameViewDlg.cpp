// FrameViewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RTSPClient.h"
#include "FrameViewDlg.h"
#include "afxdialogex.h"

int thread_exit=0;
char eventLoopWatchVariable = 0;
static unsigned rtspClientCount = 0; // Counts how many streams (i.e., "RTSPClient"s) are currently in use.
//ffmpeg//
AVCodec *codec;
AVCodecContext *c= NULL;
AVFrame *decoded_frame = NULL;
SDL_Surface     *screen;


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

void CFrameViewDlg::ffmpeg_init()
{
	av_register_all();

	/* find the decoder */
	codec = avcodec_find_decoder(AV_CODEC_ID_H264);

	if (!codec) {
		fprintf(stderr, "codec not found\n");
		exit(1);
	}
	c = avcodec_alloc_context3(codec);

	/* open it */
	if (avcodec_open2(c, codec, NULL) < 0) {
		fprintf(stderr, "could not open codec\n");
		exit(1);
	}
	char sdl_var[256];    
	CWnd* pWnd = this->GetDlgItem(IDC_VIEWFRAME);
	sprintf(sdl_var, "SDL_WINDOWID=0x%1x", pWnd->GetSafeHwnd());    //主窗口句柄      //这里一定不能有空格SDL_WINDOWID=%d"
	SDL_putenv(sdl_var);

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) 
	{
		fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
		exit(1);
	}

	// Make a screen to put our video
#ifndef __DARWIN__
	screen = SDL_SetVideoMode(640, 480, 32, 0);
#else
	screen = SDL_SetVideoMode(640, 480, 0, 0);
#endif

	if(!screen)
	{
		fprintf(stderr, "SDL: could not set video mode - exiting\n");
		exit(1);
	}
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



int play_thread(LPVOID lpParam)//播放线程
{

	/*CRTSPClientDlg * pDlg = (CRTSPClientDlg *)lpParam;
	ASSERT(pDlg);*/

	int             frameFinished;
	static struct SwsContext *img_convert_ctx;
	/// SDL variables
	SDL_Overlay     *bmp;
	SDL_Rect        rect;

	//创建刷新线程
	/*pDlg->m_streamstate->refresh_tid = SDL_CreateThread(sfp_refresh_thread, pDlg);*/
	SDL_Thread *video_tid = SDL_CreateThread(sfp_refresh_thread,NULL);

	decoded_frame = avcodec_alloc_frame();

	AVPacket pkt, *packet = &pkt;

	//-----刷新线程---------
	//SDL_Thread *video_tid = SDL_CreateThread(sfp_refresh_thread,NULL);
	//显示SDL标题
	//SDL_WM_SetCaption("Simple FFmpeg Player ",NULL);

	//Event Loop
	SDL_Event event;

	for (;;) {

		//Wait
		SDL_WaitEvent(&event);
		if(event.type==SFM_REFRESH_EVENT)
		{

			//------------------------------
			if(packet_queue_get(&videoq, packet, 1)>=0)
			{
				/*while(packet_queue_get(&videoq, packet, 1)>=0)
				{*/

				// Decode video frame
				avcodec_decode_video2(c, decoded_frame, &frameFinished, packet);

				// Did we get a video frame?
				if(frameFinished) 
				{
					int w = c->width;
					int h = c->height;
					/// Allocate a place to put our YUV image on that screen
					bmp = SDL_CreateYUVOverlay(w,h, SDL_YV12_OVERLAY, screen);
					img_convert_ctx = sws_getContext(w, h, c->pix_fmt,w, h, PIX_FMT_YUV420P,SWS_BICUBIC, NULL, NULL, NULL);


					SDL_LockYUVOverlay(bmp);

					AVPicture pict;
					pict.data[0] = bmp->pixels[0];
					pict.data[1] = bmp->pixels[2];             
					pict.data[2] = bmp->pixels[1];

					pict.linesize[0] = bmp->pitches[0];
					pict.linesize[1] = bmp->pitches[2];
					pict.linesize[2] = bmp->pitches[1];

					// Convert the image into YUV format that SDL uses
					sws_scale(img_convert_ctx, (const uint8_t * const *)decoded_frame->data,decoded_frame->linesize, 0, c->height,pict.data, pict.linesize);

					SDL_UnlockYUVOverlay(bmp);

					rect.x = 0;
					rect.y = 0;
					rect.w = c->width;
					rect.h = c->height;
					SDL_DisplayYUVOverlay(bmp, &rect);
					//延时40ms                                             
					SDL_Delay(40);
					SDL_FreeYUVOverlay(bmp);
					sws_freeContext(img_convert_ctx);
				}

				av_free_packet(packet);
				SDL_FreeSurface(screen);


			}
		}
	}
	av_free(decoded_frame);
	SDL_KillThread(video_tid);
	


	return 0;
}
// CFrameViewDlg dialog

IMPLEMENT_DYNAMIC(CFrameViewDlg, CDialogEx)

CFrameViewDlg::CFrameViewDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFrameViewDlg::IDD, pParent)
{

	//m_streamstate = NULL;
	//m_streamstate = (SStreamState *)calloc(1,sizeof(SStreamState));
	//m_streamstate->read_tid = NULL; 

	//ffmpeg_init();

	//packet_queue_init(&videoq);


	//// Begin by setting up our usage environment:
	//TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	//UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

	////char url[] = "rtsp://admin:12345@192.168.23.98";
	//char url[] = "rtsp://192.168.23.105:8554/test";
	//openURL(*env, "test_client", url);
	///*if(!openURL(*env, "test_client", url))
	//{
	//	MessageBox(_T("打开RTSP视频流失败，请检查rtsp地址！"), NULL, MB_OK );
	//	return;
	//}*/

	//m_streamstate->read_tid  = SDL_CreateThread(play_thread,NULL);
	//if (!m_streamstate->read_tid)
	//{
	//	MessageBox(_T("创建播放线程失败 请重新创建"), NULL, MB_OK );
	//	free(m_streamstate);
	//	return;
	//	
	//}
	//// All subsequent activity takes place within the event loop:
	//env->taskScheduler().doEventLoop(&eventLoopWatchVariable);
	//// This function call does not return, unless, at some point in time, "eventLoopWatchVariable" gets set to something non-zero.

}

CFrameViewDlg::~CFrameViewDlg()
{

	
}

void CFrameViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFrameViewDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CFrameViewDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CFrameViewDlg message handlers

int openURL(UsageEnvironment& env, char const* progName, char const* rtspURL) {
	// Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
	// to receive (even if more than stream uses the same "rtsp://" URL).
	RTSPClient* rtspClient = CourRTSPClient::createNew(env, rtspURL, RTSP_CLIENT_VERBOSITY_LEVEL, progName);
	if (rtspClient == NULL) {
		env << "Failed to create a RTSP client for URL \"" << rtspURL << "\": " << env.getResultMsg() << "\n";
		return 0 ;
	}

	++rtspClientCount;

	// Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
	// Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
	// Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
	rtspClient->sendDescribeCommand(continueAfterDESCRIBE);
	return 1;
}


// Implementation of the RTSP 'response handlers':

void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) {
	do {
		UsageEnvironment& env = rtspClient->envir(); // alias
		CStreamClientState& scs = ((CourRTSPClient*)rtspClient)->scs; // alias

		if (resultCode != 0) {
			env << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
			delete[] resultString;
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
	shutdownStream(rtspClient);
}

// By default, we request that the server stream its data using RTP/UDP.
// If, instead, you want to request that the server stream via RTP-over-TCP, change the following to True:
#define REQUEST_STREAMING_OVER_TCP False

void setupNextSubsession(RTSPClient* rtspClient) {
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

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {
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

void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString) {
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


// Implementation of the other event handlers:

void subsessionAfterPlaying(void* clientData) {
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

void subsessionByeHandler(void* clientData) {
	MediaSubsession* subsession = (MediaSubsession*)clientData;
	RTSPClient* rtspClient = (RTSPClient*)subsession->miscPtr;
	UsageEnvironment& env = rtspClient->envir(); // alias

	env << *rtspClient << "Received RTCP \"BYE\" on \"" << *subsession << "\" subsession\n";

	// Now act as if the subsession had closed:
	subsessionAfterPlaying(subsession);
}

void streamTimerHandler(void* clientData) {
	CourRTSPClient* rtspClient = (CourRTSPClient*)clientData;
	CStreamClientState& scs = rtspClient->scs; // alias

	scs.streamTimerTask = NULL;

	// Shut down the stream:
	shutdownStream(rtspClient);
}

void shutdownStream(RTSPClient* rtspClient, int exitCode) {
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

	if (--rtspClientCount == 0) {
		// The final stream has ended, so exit the application now.
		// (Of course, if you're embedding this code into your own application, you might want to comment this out,
		// and replace it with "eventLoopWatchVariable = 1;", so that we leave the LIVE555 event loop, and continue running "main()".)
		exit(exitCode);
	}
}




void packet_queue_init(PacketQueue *q)
{
	memset(q, 0, sizeof(PacketQueue));
	q->mutex = SDL_CreateMutex();
	q->cond = SDL_CreateCond();
}


/**
SDL_LockMutex() locks the mutex in the queue so we can add something to it, 
and then SDL_CondSignal() sends a signal to our get function (if it is 
waiting) through our condition variable to tell it that there is data and 
it can proceed, then unlocks the mutex to let it go.
*/

static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
{
	AVPacketList *pkt1;
	int ret;

	SDL_LockMutex(q->mutex);

	for(;;) {
		pkt1 = q->first_pkt;
		if (pkt1) {
			q->first_pkt = pkt1->next;
			if (!q->first_pkt)
				q->last_pkt = NULL;
			q->nb_packets--;
			q->size -= pkt1->pkt.size;
			*pkt = pkt1->pkt;
			av_free(pkt1);
			ret = 1;
			break;
		} else if (!block) {
			ret = 0;
			break;
		} else {
			SDL_CondWait(q->cond, q->mutex);
		}
	}
	SDL_UnlockMutex(q->mutex);
	return ret;
}




void CFrameViewDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	m_streamstate = NULL;
	m_streamstate = (SStreamState *)calloc(1,sizeof(SStreamState));
	m_streamstate->read_tid = NULL; 

	ffmpeg_init();

	packet_queue_init(&videoq);


	// Begin by setting up our usage environment:
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

	//char url[] = "rtsp://admin:12345@192.168.23.98";
	char url[] = "rtsp://192.168.23.105:8554/test";
	openURL(*env, "test_client", url);
	/*if(!openURL(*env, "test_client", url))
	{
		MessageBox(_T("打开RTSP视频流失败，请检查rtsp地址！"), NULL, MB_OK );
		return;
	}*/

	m_streamstate->read_tid  = SDL_CreateThread(play_thread,NULL);
	if (!m_streamstate->read_tid)
	{
		MessageBox(_T("创建播放线程失败 请重新创建"), NULL, MB_OK );
		free(m_streamstate);
		return;
		
	}
	// All subsequent activity takes place within the event loop:
	env->taskScheduler().doEventLoop(&eventLoopWatchVariable);
	// This function call does not return, unless, at some point in time, "eventLoopWatchVariable" gets set to something non-zero.
		UpdateData(FALSE);
	CDialogEx::OnOK();
}
