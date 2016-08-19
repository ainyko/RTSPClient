#include "StdAfx.h"
#include "QueueProcess.h"


CQueueProcess::CQueueProcess(void)
{
}


CQueueProcess::~CQueueProcess(void)
{
}


void CQueueProcess::packet_queue_init(PacketQueue *q) 
{
	memset(q, 0, sizeof(PacketQueue));
	q->mutex = SDL_CreateMutex();
	q->cond = SDL_CreateCond();
}

int CQueueProcess::packet_queue_put(PacketQueue *q, AVPacket *pkt) 
{
	int ret;

	/* duplicate the packet */
	if (pkt != &m_flush_pkt && av_dup_packet(pkt) < 0)
	{
		return -1;
	}

	SDL_LockMutex(q->mutex);
	ret = packet_queue_put_private(q, pkt);
	SDL_UnlockMutex(q->mutex);


	if (pkt != &m_flush_pkt && ret < 0)
	{
		av_free_packet(pkt);
	}

	return ret;
}

void CQueueProcess::packet_queue_start(PacketQueue *q)
{
	SDL_LockMutex(q->mutex);
	packet_queue_put_private(q, &m_flush_pkt);
	SDL_UnlockMutex(q->mutex);
}

int CQueueProcess::packet_queue_get(PacketQueue *q, AVPacket *pkt, int queue_type)
{
	AVPacketList *pkt1;
	int ret;

	SDL_LockMutex(q->mutex);

	for(;;)
	{
		//TRACE("222222222222222\n");
		pkt1 = q->first_pkt;
		if (pkt1) 
		{
			q->first_pkt = pkt1->next;
			if (!q->first_pkt)
			{
				q->last_pkt = NULL;
			}
			q->nb_packets--;
			//q->size -= pkt1->pkt.size;
			q->size -= pkt1->pkt.size;
			*pkt = pkt1->pkt;
			av_free(pkt1);
			ret = 1;
			break;
		}
		else 
		{
			int sdlcondwait = SDL_CondWait(q->cond, q->mutex);
		}
	}
	SDL_UnlockMutex(q->mutex);
	//TRACE("·µ»ØÖµret= %d\n",ret);
	return ret;
}

void CQueueProcess::packet_queue_abort(PacketQueue *q)
{
	SDL_LockMutex(q->mutex);
	SDL_CondSignal(q->cond);
	SDL_UnlockMutex(q->mutex);
}


void CQueueProcess::packet_queue_flush(PacketQueue *q) 
{
	AVPacketList *pkt, *pkt1;

	SDL_LockMutex(q->mutex);
	for(pkt = q->first_pkt; pkt != NULL; pkt = pkt1) 
	{
		pkt1 = pkt->next;
		av_free_packet(&pkt->pkt);
		av_freep(&pkt);
	}
	q->last_pkt = NULL;
	q->first_pkt = NULL;
	q->nb_packets = 0;
	q->size = 0;
	SDL_UnlockMutex(q->mutex);
}

void CQueueProcess::packet_queue_destroy(PacketQueue *q)
{
	packet_queue_flush(q);
	SDL_DestroyMutex(q->mutex);
	SDL_DestroyCond(q->cond);
}

int CQueueProcess::packet_queue_put_private(PacketQueue *q, AVPacket *pkt)
{
	AVPacketList *pkt1;

	pkt1 = (AVPacketList * )av_malloc(sizeof(AVPacketList));
	if (!pkt1)
	{
		return -1;
	}
	pkt1->pkt = *pkt;
	pkt1->next = NULL;

	if (!q->last_pkt)
	{
		q->first_pkt = pkt1;
	}
	else
	{
		q->last_pkt->next = pkt1;
	}
	q->last_pkt = pkt1;
	q->nb_packets++;
	//q->size += pkt1->pkt.size + sizeof(*pkt1);
	q->size += pkt1->pkt.size;
	/* XXX: should duplicate packet data in DV case */
	//TRACE("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  %d,%d\n",q->size,sizeof(pkt1));
	SDL_CondSignal(q->cond);
	return 0;
}
