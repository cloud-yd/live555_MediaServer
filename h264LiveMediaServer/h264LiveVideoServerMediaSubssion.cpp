#include "h264LiveVideoServerMediaSubssion.hh"
#include "h264LiveFramedSource.hh"
#include "H264VideoStreamFramer.hh"

H264LiveVideoServerMediaSubssion* H264LiveVideoServerMediaSubssion::createNew(UsageEnvironment& env, Boolean reuseFirstSource)
{
	return new H264LiveVideoServerMediaSubssion(env, reuseFirstSource);
}

H264LiveVideoServerMediaSubssion::H264LiveVideoServerMediaSubssion(UsageEnvironment& env, Boolean reuseFirstSource)
	: H264VideoFileServerMediaSubsession(env, 0, reuseFirstSource)
{

}

H264LiveVideoServerMediaSubssion::~H264LiveVideoServerMediaSubssion()
{
}

FramedSource* H264LiveVideoServerMediaSubssion::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate)
{
	//estimate bitrate:估计的比特率，记得根据需求修改
	estBitrate = 1000; // kbps
	//创建视频源
	H264LiveFramedSource* liveSource = H264LiveFramedSource::createNew(envir());
	if (liveSource == NULL)
	{
		return NULL;
	}

	//为视频流创建Framer
	return H264VideoStreamFramer::createNew(envir(), liveSource);

}