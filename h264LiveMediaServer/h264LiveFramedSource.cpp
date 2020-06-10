
#include "h264LiveFramedSource.hh"
#include "GroupsockHelper.hh"
#include "spsdecode.h"

int findStartCode(unsigned char *buf, int zeros_in_startcode)
{
	int info;
	int i;

	info = 1;
	for (i = 0; i < zeros_in_startcode; i++)
		if (buf[i] != 0)
			info = 0;

	if (buf[i] != 1)
		info = 0;
	return info;
}
//此处的NALU包括StartCode
int getNextNalu(FILE* inpf, unsigned char* buf)
{
	int pos = 0;
	int startCodeFound = 0;
	int info2 = 0;
	int info3 = 0;

	while (!feof(inpf) && (buf[pos++] = fgetc(inpf)) == 0);

	while (!startCodeFound)
	{
		if (feof(inpf))
		{
			return pos - 1;
		}
		buf[pos++] = fgetc(inpf);
		info3 = findStartCode(&buf[pos - 4], 3);
		startCodeFound = (info3 == 1);
		if (info3 != 1)
			info2 = findStartCode(&buf[pos - 3], 2);
		startCodeFound = (info2 == 1 || info3 == 1);
	}
	if (info2)
	{
		fseek(inpf, -3, SEEK_CUR);
		return pos - 3;
	}
	if (info3)
	{
		fseek(inpf, -4, SEEK_CUR);
		return pos - 4;
	}
}

FILE * inpf;
unsigned char* inBuf;
int inLen;
int nFrameRate;
H264LiveFramedSource::H264LiveFramedSource(UsageEnvironment& env, unsigned preferredFrameSize, unsigned playTimePerFrame)
	: ByteStreamFileSource(env, 0, preferredFrameSize, playTimePerFrame)
{
	openFile();
}

H264LiveFramedSource* H264LiveFramedSource::createNew(UsageEnvironment& env, unsigned preferredFrameSize, unsigned playTimePerFrame)
{
	H264LiveFramedSource* newSource = new H264LiveFramedSource(env, preferredFrameSize, playTimePerFrame);
	return newSource;
}

int H264LiveFramedSource::openFile()
{
	const char *fname = "test.264";
	inpf = NULL;
	if (fopen_s(&inpf, fname, "rb"))
	{
		return false;
	}
	
	inBuf = (unsigned char*)calloc(1024 * 100, sizeof(char));
	inLen = 0;
	inLen = getNextNalu(inpf, inBuf);
	// 读取SPS帧
	unsigned int nSpsLen = inLen - 4;
	unsigned char *pSps = (unsigned char*)malloc(nSpsLen);
	memcpy(pSps, inBuf + 4, nSpsLen);

	// 解码SPS,获取视频图像宽、高信息
	int width = 0, height = 0, fps = 0;

	h264_decode_sps(pSps, nSpsLen, width, height, fps);

	nFrameRate = 0;
	if (fps)
		nFrameRate = fps;
	else
		nFrameRate = 25;
}

H264LiveFramedSource::~H264LiveFramedSource()
{
	free(inBuf);
	fclose(inpf);
}

// This function is called when new frame data is available from the device.
// We deliver this data by copying it to the 'downstream' object, using the following parameters (class members):
// 'in' parameters (these should *not* be modified by this function):
//     fTo: The frame data is copied to this address.
//         (Note that the variable "fTo" is *not* modified.  Instead,
//          the frame data is copied to the address pointed to by "fTo".)
//     fMaxSize: This is the maximum number of bytes that can be copied
//         (If the actual frame is larger than this, then it should
//          be truncated, and "fNumTruncatedBytes" set accordingly.)
// 'out' parameters (these are modified by this function):
//     fFrameSize: Should be set to the delivered frame size (<= fMaxSize).
//     fNumTruncatedBytes: Should be set iff the delivered frame would have been
//         bigger than "fMaxSize", in which case it's set to the number of bytes
//         that have been omitted.
//     fPresentationTime: Should be set to the frame's presentation time
//         (seconds, microseconds).  This time must be aligned with 'wall-clock time' - i.e., the time that you would get
//         by calling "gettimeofday()".
//     fDurationInMicroseconds: Should be set to the frame's duration, if known.
//         If, however, the device is a 'live source' (e.g., encoded from a camera or microphone), then we probably don't need
//         to set this variable, because - in this case - data will never arrive 'early'.
void H264LiveFramedSource::doGetNextFrame()
{
	fFrameSize = inLen;
	if (fFrameSize > fMaxSize)
	{
		fNumTruncatedBytes = fFrameSize - fMaxSize;
		fFrameSize = fMaxSize;
	}
	else
	{
		fNumTruncatedBytes = 0;
	}
	memmove(fTo, inBuf, fFrameSize);

	inLen = 0;
	inLen = getNextNalu(inpf, inBuf);
	gettimeofday(&fPresentationTime, NULL);//时间戳 
	fDurationInMicroseconds = 1000000 / nFrameRate;//控制播放速度
	//表示延迟0秒后再执行afterGetting函数,也可以直接用afterGetting(this)
	nextTask() = envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc*)FramedSource::afterGetting, this);
}
