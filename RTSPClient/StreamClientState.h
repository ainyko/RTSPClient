#pragma once
// Define a class to hold per-stream state that we maintain throughout each stream's lifetime:
class CStreamClientState
{
public:
	CStreamClientState(void);
	virtual ~CStreamClientState(void);

	MediaSubsessionIterator* iter;
	MediaSession* session;
	MediaSubsession* subsession;
	TaskToken streamTimerTask;
	double duration;


};

