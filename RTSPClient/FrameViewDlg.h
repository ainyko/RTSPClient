#pragma once
#include "DummySink.h"
#include "StreamClientState.h"
#include "ourRTSPClient.h"

// CFrameViewDlg dialog


#define VIDEO_PICTURE_QUEUE_SIZE 1
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
#define RTSP_CLIENT_VERBOSITY_LEVEL 1 // by default, print verbose output from each "RTSPClient"

typedef struct VideoPicture
{
	SDL_Overlay *bmp;
	int width, height; /* source height & width */
	int allocated;
	double pts;
} VideoPicture;

typedef struct StreamState_t 
{
	AVFormatContext *pFormatCtx;
	AVStream        *audio_st;
	AVStream        *video_st;
	PacketQueue     audioq;
	PacketQueue     videoq;
	uint8_t         audio_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
	unsigned int    audio_buf_size;
	unsigned int    audio_buf_index;
	AVPacket        audio_pkt;
	uint8_t         *audio_pkt_data;
	int             audio_pkt_size;
	VideoPicture    pictq[VIDEO_PICTURE_QUEUE_SIZE];
	int             pictq_size, pictq_rindex, pictq_windex;
	SDL_mutex       *pictq_mutex;     //�������
	SDL_cond        *pictq_cond;      //��������
	SDL_Thread      *read_tid;        //���Ǵ��ļ���ȡ���ݷ�����е��߳�
	SDL_Thread      *video_tid;       //������Ƶ��ȡ��������߳�
	SDL_Thread      *refresh_tid;     //����ˢ���߳�
	double          audio_clock;      //��Ƶ��ʱ��� 
	double          video_clock;      //��Ƶ��ʱ���
	double          seek_time;        //Ҫ�ƶ���ʱ�䣨�룩
	int             seek_req;         //seek�ı�־ �Ƿ���Ҫseek
	int             seek_flags;       //seek�ķ�ʽ AVSEEK_FLAG_FRAME��
	int64_t         seek_pos;         //seek�����ʱ��
} SStreamState;

class CFrameViewDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFrameViewDlg)

public:
	CFrameViewDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFrameViewDlg();


// Dialog Data
	enum { IDD = IDD_FRAMEVIEW_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()


public:
	SStreamState* m_streamstate;
	void ffmpeg_init();
	afx_msg void OnBnClickedOk();
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

// The main streaming routine (for each "rtsp://" URL):
int openURL(UsageEnvironment& env, char const* progName, char const* rtspURL);

// Used to iterate through each stream's 'subsessions', setting up each one:
void setupNextSubsession(RTSPClient* rtspClient);

// Used to shut down and close a stream (including its "RTSPClient" object):
void shutdownStream(RTSPClient* rtspClient, int exitCode = 1);

void packet_queue_init(PacketQueue *q);
int sfp_refresh_thread(void *opaque);
int play_thread(LPVOID lpParam);
static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block);