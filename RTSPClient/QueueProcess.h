#pragma once

#define VIDEO_PICTURE_QUEUE_SIZE 1

typedef struct VideoPicture
{
	SDL_Overlay *bmp;
	int width, height; /* source height & width */
	int allocated;
	double pts;
} VideoPicture;

typedef struct PacketQueue 
{
	AVPacketList *first_pkt, *last_pkt;
	int nb_packets;
	int size;
	SDL_mutex *mutex;
	SDL_cond *cond;
} PacketQueue;


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
	SDL_mutex       *pictq_mutex;     //互斥变量
	SDL_mutex       *qget_mutex;     //互斥变量
	SDL_cond        *pictq_cond;      //条件变量
	//SDL_Thread      *read_tid;        //这是从文件读取数据放入队列的线程
	//SDL_Thread      *doEventLoop_tid; //网络视频流循环线程
	SDL_Thread      *video_tid;       //这是视频读取放入队列线程
	//SDL_Thread      *refresh_tid;     //这是刷新线程
	double          audio_clock;      //音频的时间戳 
	double          video_clock;      //视频的时间戳
	double          seek_time;        //要移动的时间（秒）
	int             seek_req;         //seek的标志 是否需要seek
	int             seek_flags;       //seek的方式 AVSEEK_FLAG_FRAME等
	int64_t         seek_pos;         //seek过后的时间
} SStreamState;

class CQueueProcess
{
public:
	CQueueProcess(void);
	virtual ~CQueueProcess(void);

	AVPacket m_flush_pkt;                    //刷新packet

	//始化队列
	void packet_queue_init(PacketQueue *q) ;   
	//将packet填入队列（这里只是做的和刷新packet判断）
	int packet_queue_put(PacketQueue *q, AVPacket *pkt);   
	//从队列里面取出数据
	int packet_queue_get(PacketQueue *q, AVPacket *pkt, int queue_type);
	//终止向队列输入输出
	void packet_queue_abort(PacketQueue *q);             
	//真正的填入队列函数
	int packet_queue_put_private(PacketQueue *q, AVPacket *pkt);    
	//刷新packet的初始化
	void packet_queue_start(PacketQueue *q); 
	//销毁队列 
	void packet_queue_destroy(PacketQueue *q);     
	//清空队列
	void packet_queue_flush(PacketQueue *q); 
};

