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
	SDL_mutex       *pictq_mutex;     //�������
	SDL_mutex       *qget_mutex;     //�������
	SDL_cond        *pictq_cond;      //��������
	//SDL_Thread      *read_tid;        //���Ǵ��ļ���ȡ���ݷ�����е��߳�
	//SDL_Thread      *doEventLoop_tid; //������Ƶ��ѭ���߳�
	SDL_Thread      *video_tid;       //������Ƶ��ȡ��������߳�
	//SDL_Thread      *refresh_tid;     //����ˢ���߳�
	double          audio_clock;      //��Ƶ��ʱ��� 
	double          video_clock;      //��Ƶ��ʱ���
	double          seek_time;        //Ҫ�ƶ���ʱ�䣨�룩
	int             seek_req;         //seek�ı�־ �Ƿ���Ҫseek
	int             seek_flags;       //seek�ķ�ʽ AVSEEK_FLAG_FRAME��
	int64_t         seek_pos;         //seek�����ʱ��
} SStreamState;

class CQueueProcess
{
public:
	CQueueProcess(void);
	virtual ~CQueueProcess(void);

	AVPacket m_flush_pkt;                    //ˢ��packet

	//ʼ������
	void packet_queue_init(PacketQueue *q) ;   
	//��packet������У�����ֻ�����ĺ�ˢ��packet�жϣ�
	int packet_queue_put(PacketQueue *q, AVPacket *pkt);   
	//�Ӷ�������ȡ������
	int packet_queue_get(PacketQueue *q, AVPacket *pkt, int queue_type);
	//��ֹ������������
	void packet_queue_abort(PacketQueue *q);             
	//������������к���
	int packet_queue_put_private(PacketQueue *q, AVPacket *pkt);    
	//ˢ��packet�ĳ�ʼ��
	void packet_queue_start(PacketQueue *q); 
	//���ٶ��� 
	void packet_queue_destroy(PacketQueue *q);     
	//��ն���
	void packet_queue_flush(PacketQueue *q); 
};

