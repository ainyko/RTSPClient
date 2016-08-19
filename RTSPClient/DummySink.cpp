#include "StdAfx.h"
#include "DummySink.h"

//定义音视频全局结构体
SStreamState* m_streamstate = (SStreamState *)calloc(1,sizeof(SStreamState));

CDummySink* CDummySink::createNew(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId)
{
	return new CDummySink(env, subsession, streamId);
}

CDummySink::CDummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId): MediaSink(env), fSubsession(subsession) 
{

	p_nalu_tail = nalu_buffer;
	fStreamId = strDup(streamId);
	fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];

}

CDummySink::~CDummySink() {
	delete[] fReceiveBuffer;
	delete[] fStreamId;
}

void CDummySink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds)
{
	CDummySink* sink = (CDummySink*)clientData;
	sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

// If you don't want to see debugging output for each received frame, then comment out the following line:
//#define DEBUG_PRINT_EACH_RECEIVED_FRAME 1

void CDummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned /*durationInMicroseconds*/) 
{


	// We've just received a frame of data.  (Optionally) print out information about it:
#ifdef DEBUG_PRINT_EACH_RECEIVED_FRAME
	if (fStreamId != NULL)
		envir() << "Stream \"" << fStreamId << "\"; ";
	envir() << fSubsession.mediumName() << "/" << fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";

	if (numTruncatedBytes > 0) 
		envir() << " (with " << numTruncatedBytes << " bytes truncated)";


	char uSecsStr[6+1]; // used to output the 'microseconds' part of the presentation time

	sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);

	envir() << ".\tPresentation time: " << (int)presentationTime.tv_sec << "." << uSecsStr;
	//if (fSubsession.rtpSource() != NULL && !fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP()) {
	//  envir() << "!"<<"\n"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
	//}
	if (fSubsession.rtpSource() != NULL) {
		//  envir() << fSubsession.rtpSource()->curPacketRTPSeqNum()<<"\n";
		fprintf(stderr, "yyyyyyyyyyyy%d\n", fSubsession.rtpSource()->curPacketRTPSeqNum());
	}
#ifdef DEBUG_PRINT_NPT
	envir() << "\tNPT: " << fSubsession.getNormalPlayTime(presentationTime);
#endif
	envir() << "\n";
#endif

	static struct timeval pre_time_stamp = {0,0};


	unsigned char const start_code[4] = {0x00, 0x00, 0x00, 0x01};
	if(strcmp(fSubsession.codecName(),"H264") ==0)
	{
		static bool fHaveWrittenFirstFrame = false;
		if (!fHaveWrittenFirstFrame) 
		{
			// If we have PPS/SPS NAL units encoded in a "sprop parameter string", prepend these to the file:
			unsigned numSPropRecords;
			SPropRecord* sPropRecords = parseSPropParameterSets(fSubsession.fmtp_spropparametersets(), numSPropRecords);
			for (unsigned i = 0; i < numSPropRecords; ++i) {
				memcpy(p_nalu_tail, start_code, sizeof(start_code));
				p_nalu_tail += sizeof(start_code);
				memcpy(p_nalu_tail, sPropRecords[i].sPropBytes, sPropRecords[i].sPropLength);
				p_nalu_tail += sPropRecords[i].sPropLength;
			}

			fHaveWrittenFirstFrame = true; // for next time

			memcpy(p_nalu_tail, start_code, sizeof(start_code));
			p_nalu_tail += sizeof(start_code);
			memcpy(p_nalu_tail, fReceiveBuffer, frameSize);
			p_nalu_tail += frameSize;
		}
		else
		{
			if(presentationTime.tv_sec == pre_time_stamp.tv_sec && presentationTime.tv_usec == pre_time_stamp.tv_usec)
			{
				memcpy(p_nalu_tail, start_code, sizeof(start_code));
				p_nalu_tail += sizeof(start_code);
				memcpy(p_nalu_tail, fReceiveBuffer, frameSize);
				p_nalu_tail += frameSize;
			}
			else
			{

				if(p_nalu_tail != nalu_buffer){
					AVPacket packet, *pkt = &packet;
					av_new_packet(pkt, p_nalu_tail - nalu_buffer);
					memcpy(pkt->data , nalu_buffer, p_nalu_tail - nalu_buffer);
					queueProcess.packet_queue_put(&m_streamstate->videoq, pkt);
				}

				p_nalu_tail = nalu_buffer;
				memcpy(p_nalu_tail, start_code, sizeof(start_code));
				p_nalu_tail += sizeof(start_code);
				memcpy(p_nalu_tail, fReceiveBuffer, frameSize);
				p_nalu_tail += frameSize;


				/* if the queue are full, no need to read more */
				if (m_streamstate->videoq.size  > MAX_QUEUE_SIZE || m_streamstate->videoq.nb_packets > MIN_FRAMES)
				{
					//TRACE("积压包数目大于最大包数<%d  内存= %d\n",m_streamstate->videoq.nb_packets,m_streamstate->videoq.size);
					/* wait 10 ms */

					if(!m_streamstate->videoq.first_pkt)
						queueProcess.packet_queue_destroy(&m_streamstate->videoq);
					m_streamstate->videoq.cond = NULL;
					m_streamstate->videoq.first_pkt= NULL;
					m_streamstate->videoq.last_pkt = NULL;
					m_streamstate->videoq.mutex = NULL;
					m_streamstate->videoq.size = 0;
					m_streamstate->videoq.nb_packets = 0;

					//TRACE("释放后的包数= %d  内存=%d\n",m_streamstate->videoq.nb_packets,m_streamstate->audioq.size);


				}


				//TRACE("积压包数目= %d  内存=%d\n",m_streamstate->videoq.nb_packets,m_streamstate->audioq.size);
			}
		}
		pre_time_stamp = presentationTime;
	}




	// Then continue, to request the next frame of data:

	continuePlaying();
}

bool CDummySink::continuePlaying()
{

	if (fSource == NULL ) return False; // sanity check (should not happen)

	// Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:

	fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE,afterGettingFrame, this,onSourceClosure, this);
	return True;
}
