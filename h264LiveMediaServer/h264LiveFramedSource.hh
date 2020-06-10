#pragma once
#ifndef _H264LIVEFRAMEDSOURCE_HH
#define _H264LIVEFRAMEDSOURCE_HH


#include <ByteStreamFileSource.hh>


class H264LiveFramedSource : public ByteStreamFileSource
{
public:
	static H264LiveFramedSource* createNew(UsageEnvironment& env, unsigned preferredFrameSize = 0, unsigned playTimePerFrame = 0);
	int openFile();


protected:
	H264LiveFramedSource(UsageEnvironment& env, unsigned preferredFrameSize, unsigned playTimePerFrame);
	~H264LiveFramedSource();

private:
	//重定义虚函数
	virtual void doGetNextFrame();
};

#endif
