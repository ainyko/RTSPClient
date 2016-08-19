#pragma once
#include "QueueProcess.h"
#include "StreamClientState.h"
// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.


// Implementation of "DummySink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 100000
#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES      5

//声明音视频全局结构体
extern  SStreamState* m_streamstate;

class CDummySink:public MediaSink
{
public:
	CDummySink(void);
	virtual ~CDummySink(void);

	static CDummySink* createNew(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId = NULL);
private:
	CDummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId);
	// called only by "createNew()"

	static void afterGettingFrame(void* clientData, unsigned frameSize,  unsigned numTruncatedBytes,struct timeval presentationTime, unsigned durationInMicroseconds);
	void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime, unsigned durationInMicroseconds);

private:
	// redefined virtual functions:
	virtual bool continuePlaying();

private:
	u_int8_t* fReceiveBuffer;
	MediaSubsession& fSubsession;
	char* fStreamId;
	int   ID;
	FILE* fp;
	char nalu_buffer[1024*1024];
	char* p_nalu_tail;
public:
	CQueueProcess queueProcess;
	//SStreamState* m_streamstate; //音视频全局结构体
};

