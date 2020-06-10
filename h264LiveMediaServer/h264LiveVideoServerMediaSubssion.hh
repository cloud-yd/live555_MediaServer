#pragma once
#ifndef _H264LIVEVIDEOSERVERMEDIASUBSSION_HH
#define _H264LIVEVIDEOSERVERMEDIASUBSSION_HH
#include "H264VideoFileServerMediaSubsession.hh"

class H264LiveVideoServerMediaSubssion : public H264VideoFileServerMediaSubsession {

public:
	static H264LiveVideoServerMediaSubssion* createNew(UsageEnvironment& env, Boolean reuseFirstSource);

protected:
	H264LiveVideoServerMediaSubssion(UsageEnvironment& env, Boolean reuseFirstSource);
	~H264LiveVideoServerMediaSubssion();

protected:
	//重定义虚函数
	FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
};

#endif
