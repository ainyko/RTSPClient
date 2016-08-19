#include "StdAfx.h"
#include "CffmpegProcess.h"


CffmpegProcess::CffmpegProcess(void)
{
	//��ʼ��ָ��
	pStartPlay_tid = NULL;
	codec = NULL;
	m_screen = NULL;
	codec_context = NULL;
}


CffmpegProcess::~CffmpegProcess(void)
{

}


void CffmpegProcess::ffmpeg_initDecoder(HWND hwnd)
{
	//��ʼ��ָ��
	pStartPlay_tid = NULL;
	codec = NULL;
	m_screen = NULL;
	codec_context = NULL;

	av_register_all();

	/* find the decoder */
	codec = avcodec_find_decoder(AV_CODEC_ID_H264);

	if (!codec) {
		fprintf(stderr, "codec not found\n");
		exit(1);
	}
	codec_context = avcodec_alloc_context3(codec);

	/* open it */
	if (avcodec_open2(codec_context, codec, NULL) < 0) {
		fprintf(stderr, "could not open codec\n");
		exit(1);
	}

	char sdl_var[256];    
	sprintf(sdl_var, "SDL_WINDOWID=0x%1x",hwnd );    //�����ھ��      //����һ�������пո�SDL_WINDOWID=%d"
	SDL_putenv(sdl_var);


	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) 
	{
		fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
		exit(1);
	}

	//������Ƶ��������С
	m_screen = SDL_SetVideoMode(MFCVIDEOSIZE_WIDTH , MFCVIDEOSIZE_HEIGHT, 32, 0);


}

void CffmpegProcess::ffmpeg_flushQueue()
{
	//ˢ��packet��ʼ��
	av_init_packet(&(queueProcess.m_flush_pkt));
	queueProcess.m_flush_pkt.data = (uint8_t *)(intptr_t)"FLUSH";
}

int CffmpegProcess::ffmpeg_initQueue()
{
	/////////////////////////////////////////////////////////////////////////
	codec_context = NULL;
	/////////////////////////////////////////////////////////////////////////
	//����Ƶȫ�ֽṹ���ʼ��
	m_streamstate->video_st = NULL;
	m_streamstate->videoq.first_pkt = NULL;
	m_streamstate->videoq.last_pkt = NULL;
	m_streamstate->videoq.nb_packets = 0;
	m_streamstate->videoq.size = 0;
	m_streamstate->videoq.mutex = NULL;
	m_streamstate->videoq.cond = NULL;
	//AVPacket       audio_pkt;
	m_streamstate->pictq[0].bmp = NULL;
	m_streamstate->pictq[0].width = 0;
	m_streamstate->pictq[0].height = 0;
	m_streamstate->pictq[0].allocated = 0;
	m_streamstate->pictq[0].pts = 0.0;
	m_streamstate->pictq_size = 0;
	m_streamstate->pictq_rindex = 0;
	m_streamstate->pictq_windex = 0;
	m_streamstate->pictq_mutex = NULL;
	m_streamstate->qget_mutex = NULL;
	m_streamstate->pictq_cond = NULL;
	//����ȫ�����������������
	m_streamstate->pictq_mutex = SDL_CreateMutex();
	m_streamstate->qget_mutex = SDL_CreateMutex();
	m_streamstate->pictq_cond  = SDL_CreateCond();
	//m_streamstate->read_tid = NULL; 
	//m_streamstate->refresh_tid = NULL;
	m_streamstate->video_clock = 0.0;
	m_streamstate->seek_time = 0.0;
	m_streamstate->seek_req = 0;
	m_streamstate->seek_flags = 0;
	m_streamstate->seek_pos = 0;

	queueProcess.packet_queue_init(&m_streamstate->videoq);//��ʼ����Ƶ���������
	//ˢ��packet��ʼ��
	av_init_packet(&(queueProcess.m_flush_pkt));
	queueProcess.m_flush_pkt.data = (uint8_t *)(intptr_t)"FLUSH";

	//����Ƶȫ�ֽṹ���ʼ�� �����Ѿ�����
	m_streamstate = NULL;
	m_streamstate = (SStreamState *)calloc(1,sizeof(SStreamState));
	if (!m_streamstate)
	{
		return 0;
	}

	//ˢ��packet��ʼ��
	ffmpeg_flushQueue();

	//////////////////////////////////////////////////////////////////////////
	return 1;
}

void CffmpegProcess::ffmpeg_flushQueueDestroy()
{
	//�ͷ�ˢ��packet
	if (queueProcess.m_flush_pkt.data) 
	{
		av_free_packet(&(queueProcess.m_flush_pkt));
	}
}

void CffmpegProcess::ffmpeg_queueDestroy()
{

	//�ͷŻ������ ��������
	if (m_streamstate->pictq_mutex)
	{
		SDL_DestroyMutex(m_streamstate->pictq_mutex);
	}
	if (m_streamstate->qget_mutex)
	{
		SDL_DestroyMutex(m_streamstate->qget_mutex);
	}
	if (m_streamstate->pictq_cond)
	{
		SDL_DestroyCond(m_streamstate->pictq_cond);
	}

	//�ͷ���Ƶ������
	if (m_streamstate->videoq.mutex)
	{
		queueProcess.packet_queue_destroy(&m_streamstate->videoq);
	}
	//�ͷ���Ƶ������
	if (m_streamstate->audioq.mutex)
	{
		queueProcess.packet_queue_destroy(&m_streamstate->audioq);
	}



	if(!m_streamstate)
	{
		//�ͷ�ȫ�ֽṹ��
		m_streamstate->audio_st = NULL;
		m_streamstate->video_st = NULL;
		m_streamstate->audioq.first_pkt = NULL;
		m_streamstate->audioq.last_pkt = NULL;
		m_streamstate->audioq.nb_packets = 0;
		m_streamstate->audioq.size = 0;
		m_streamstate->audioq.mutex = NULL;
		m_streamstate->audioq.cond = NULL;
		m_streamstate->videoq.first_pkt = NULL;
		m_streamstate->videoq.last_pkt = NULL;
		m_streamstate->videoq.nb_packets = 0;
		m_streamstate->videoq.size = 0;
		m_streamstate->videoq.mutex = NULL;
		m_streamstate->videoq.cond = NULL;
		memset(m_streamstate->audio_buf,0,(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2);
		m_streamstate->audio_buf_size = 0;
		m_streamstate->audio_buf_index = 0;
		//AVPacket       audio_pkt;
		m_streamstate->audio_pkt_data = NULL;
		m_streamstate->audio_pkt_size = 0;
		m_streamstate->pictq[0].bmp = NULL;
		m_streamstate->pictq[0].width = 0;
		m_streamstate->pictq[0].height = 0;
		m_streamstate->pictq[0].allocated = 0;
		m_streamstate->pictq[0].pts = 0.0;
		m_streamstate->pictq_size = 0;
		m_streamstate->pictq_rindex = 0;
		m_streamstate->pictq_windex = 0;
		m_streamstate->qget_mutex = NULL;
		m_streamstate->pictq_mutex = NULL;
		m_streamstate->pictq_cond = NULL;
		//m_streamstate->read_tid = NULL; 
		//m_streamstate->refresh_tid = NULL;
		m_streamstate->audio_clock = 0.0;
		m_streamstate->video_clock = 0.0;
		m_streamstate->seek_time = 0.0;
		m_streamstate->seek_req = 0;
		m_streamstate->seek_flags = 0;
		m_streamstate->seek_pos = 0;
		free(m_streamstate);
		m_streamstate = NULL;
	}

}

int CffmpegProcess::ffmpeg_exit()
{
	VideoPicture * vp;
	int i;
	m_Isstop = 1;  //�������Ϊ1 Ҫ��Ȼ����߳̿��ܻ�û�˳� �������˱���
	m_pause_play = 0;  
	//�ر��߳�
	closePlayThread();
	//�ͷ�������������
	ffmpeg_queueDestroy();
	//�ͷ�ˢ��packet
	ffmpeg_flushQueueDestroy();

	//////////////////////////////////////////////////////////////////////////
	//�ͷŴ��� 
	if (m_screen)
	{ 
		SDL_FreeSurface(m_screen);
		m_screen = NULL;
	}
	//�ͷ���Ƶ����context
	if(codec_context)
	{
		avcodec_close(codec_context);
		av_free(codec_context);
		codec_context = NULL;
	}

	//////////////////////////////////////////////////////////////////////////

	/* free all pictures */
	for (i = 0; i < VIDEO_PICTURE_QUEUE_SIZE; i++) 
	{
		vp = &m_streamstate->pictq[i];
		if (vp->bmp) 
		{
			SDL_FreeYUVOverlay(vp->bmp);
			vp->bmp = NULL;
		}
	}
	return 1;
}

int CffmpegProcess::ffmpeg_init()
{
	//���������ļ��մ�ʱ���״̬m_Isstop=0 ��ͣ m_pause_play = 1���� 
	m_Isstop = 0;
	m_pause_play = 1; 

	//��ʼ��������������
	ffmpeg_initQueue();
	m_screen = NULL;
	catch_frameFinished = 0;
	return 1;
}

DWORD WINAPI CffmpegProcess::decoder_thread(LPVOID lpParam)//�����߳�
{
	CffmpegProcess *pThis = (CffmpegProcess*)lpParam;
	/*CRTSPClientDlg * pDlg = (CRTSPClientDlg *)lpParam;
	ASSERT(pDlg);*/

	struct SwsContext *img_convert_ctx;
	/// SDL variables
	SDL_Overlay     *bmp;
	SDL_Rect        rect;
	AVFrame *decoded_frame  = NULL;

	//����ˢ���߳�
	//m_streamstate->refresh_tid = SDL_CreateThread(sfp_refresh_thread,NULL);

	while (pThis->closePlayThreadFlag) 
	{
		AVPacket pkt;
		pThis->frameFinished = 0;

		//Wait
		/*SDL_WaitEvent(&event);
		if(event.type==SFM_REFRESH_EVENT)
		{*/
		if (pThis->m_Isstop)
		{
			break;
		}
		//TRACE("@@@@@@@@@@@@@m_pause_play@@@@@@@m_pause_play@@@@@@@@@@@@@\n");
		if (m_streamstate->videoq.size != 0 && pThis->m_pause_play) //������ڲ���
		{
			//------------------------------

			decoded_frame = avcodec_alloc_frame();//������Ƶ֡
			if( pThis->queueProcess.packet_queue_get(&m_streamstate->videoq, &pkt, 1) < 0 || m_streamstate->videoq.nb_packets < 0)
			{
				// means we quit getting packets
				break;
			}

			if(pkt.data == pThis->queueProcess.m_flush_pkt.data) 
			{
				avcodec_flush_buffers(m_streamstate->video_st->codec);
				continue;
			}


			/*while(packet_queue_get(&videoq, packet, 1)>=0)
			{*/

			// Decode video frame
			////TRACE("ǰpkt = %x \n",&pkt);
			avcodec_decode_video2(pThis->codec_context, decoded_frame, &pThis->frameFinished, &pkt);
			av_free_packet(&pkt);
			//TRACE("33333333333333333333333333333 %d , %x \n" ,frameFinished,&decoded_frame);
			////TRACE("��pkt = %x \n",&pkt);
			// �Ƿ�ɹ�����
			if(pThis->frameFinished) 
			{
				//TRACE("444444444444444444444444444444%d \n",frameFinished);
				/// Allocate a place to put our YUV image on that screen
				bmp = SDL_CreateYUVOverlay(pThis->codec_context->width,pThis->codec_context->height, SDL_YV12_OVERLAY, pThis->m_screen);
				img_convert_ctx = sws_getContext(pThis->codec_context->width, pThis->codec_context->height, pThis->codec_context->pix_fmt,pThis->codec_context->width, pThis->codec_context->height, PIX_FMT_YUV420P,SWS_BICUBIC, NULL, NULL, NULL);


				SDL_LockYUVOverlay(bmp);

				AVPicture pict;
				pict.data[0] = bmp->pixels[0];
				pict.data[1] = bmp->pixels[2];             
				pict.data[2] = bmp->pixels[1];

				pict.linesize[0] = bmp->pitches[0];
				pict.linesize[1] = bmp->pitches[2];
				pict.linesize[2] = bmp->pitches[1];

				// Convert the image into YUV format that SDL uses
				sws_scale(img_convert_ctx, (const uint8_t * const *)decoded_frame->data,decoded_frame->linesize, 0, pThis->codec_context->height,pict.data, pict.linesize);

				SDL_UnlockYUVOverlay(bmp);


				rect.x = 0;
				rect.y = 0;
				rect.w = MFCVIDEOSIZE_WIDTH / 2;
				rect.h = MFCVIDEOSIZE_HEIGHT;
				SDL_DisplayYUVOverlay(bmp, &rect);

				//ͼƬ��Ϣ��ʾ������
				if(pThis->catch_frameFinished)
				{
					rect.x = MFCVIDEOSIZE_WIDTH / 2;
					rect.y = 0; 
					rect.w = MFCVIDEOSIZE_WIDTH / 2;
					rect.h = MFCVIDEOSIZE_HEIGHT;
					SDL_DisplayYUVOverlay(bmp, &rect);
					pThis->catch_frameFinished = 0;
				}


				SDL_FreeYUVOverlay(bmp);
				sws_freeContext(img_convert_ctx);
				avcodec_free_frame(&decoded_frame);

			}


			SDL_FreeSurface(pThis->m_screen);
			//TRACE("555555555555555555555555555555555555 %x\n",&decoded_frame);

		}


	}

	return 1;
}


int CffmpegProcess::startPlay_tidFun()
{
	closePlayThreadFlag = true;
	pStartPlay_tid  = CreateThread(NULL,0,decoder_thread,this,0,NULL);
	if (!pStartPlay_tid)
	{
		//*MessageBox(_T("���߳��Ѿ�����"), NULL, MB_OK );*/
		return 0;

	}
	return 1;
}

int CffmpegProcess::closePlayThread()
{
	if(pStartPlay_tid)
	{
		closePlayThreadFlag = false;
		//�رղ����߳�
		CloseHandle(pStartPlay_tid);
		pStartPlay_tid = NULL;

		return 1;
	}
	return 0;
}

