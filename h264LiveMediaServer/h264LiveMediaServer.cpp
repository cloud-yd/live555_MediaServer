#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "h264LiveVideoServerMediaSubssion.hh"

UsageEnvironment* env;

// True：后启动的客户端总是从当前第一个客户端已经播放到的位置开始播放
// False：每个客户端都从头开始播放影视频文件
Boolean reuseFirstSource = False;

//该函数打印相关信息
static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms, char const* streamName, char const* inputFileName);

int main(int argc, char** argv)
{
	//创建任务调度器并初始化使用环境
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	env = BasicUsageEnvironment::createNew(*scheduler);
	UserAuthenticationDatabase* authDB = NULL;

	//创建RTSP服务器，开始监听模客户端的连接
	//注意这里的端口号不是默认的554端口，因此访问URL时，需指定该端口号
	RTSPServer* rtspServer = RTSPServer::createNew(*env, 8554, authDB);
	if (rtspServer == NULL)
	{
		*env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
		exit(1);
	}

	char const* descriptionString = "Session streamed by \"h264LiveMediaServer\"";

	//流名字，媒体名
	char const* streamName = "h264ESVideoTest";

	//文件名，当客户端输入的流名字为h264ESVideoTest时，实际上打开的是test.264文件。
	//这里需要特别注意一点，当通过IDE运行h264LiveMediaServer时，live555推送的是项目工作目录中的视频或音频。工作目录也就是和*.vcxproj同级的目录,
	//此时视频应该放在这个目录下。当双击h264LiveMediaServer.exe运行时，视频理所当然的和h264LiveMediaServer.exe放在一个目录。
	
	char const* inputFileName = "test.264";
	//当客户点播时，要输入流名字streamName，告诉RTSP服务器点播的是哪个流。  
	//创建媒体会话，流名字和文件名的对应关系是通过增加子会话建立起来的。媒体会话对会话描述、会话持续时间、流名字等与会话有关的信息进行管理。  
	//第2个参数:媒体名、3:媒体信息、4:媒体描述  
	
	ServerMediaSession* sms = ServerMediaSession::createNew(*env, streamName, streamName, descriptionString);
	
	//添加264子会话 这里的文件名才是真正要打开文件的名字 
	//H264VideoFileServerMediaSubsession类派生自FileServerMediaSubsession派生自OnDemandServerMediaSubsession  
	//而OnDemandServerMediaSubsession和PassiveMediaSubsession共同派生自ServerMediaSubsession  
	//关于读取文件之类都在这个类中实现的，如果要将点播改为直播就是要新建类继承此类然后添加新的方法  
	
	sms->addSubsession(H264VideoFileServerMediaSubsession::createNew(*env, inputFileName, reuseFirstSource));
	
	//为rtspserver添加session 
	rtspServer->addServerMediaSession(sms);
	
	//应答信息到标准输出
	announceStream(rtspServer, sms, streamName, inputFileName);

	// An AAC audio stream (ADTS-format file):
	{
		char const* streamName = "aacAudioTest";
		char const* inputFileName = "test.aac";
		ServerMediaSession* sms = ServerMediaSession::createNew(*env, streamName, streamName, descriptionString);
		sms->addSubsession(ADTSAudioFileServerMediaSubsession::createNew(*env, inputFileName, reuseFirstSource));
		rtspServer->addServerMediaSession(sms);

		announceStream(rtspServer, sms, streamName, inputFileName);
	}

	//进入事件循环，对套接字的读取事件和对媒体文件的延时发送操作都在这个循环中完成。 
	env->taskScheduler().doEventLoop();

	return 0;
}

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
	char const* streamName, char const* inputFileName) {
	char* url = rtspServer->rtspURL(sms);
	UsageEnvironment& env = rtspServer->envir();
	env << "\n\"" << streamName << "\" stream, from the file \""
		<< inputFileName << "\"\n";
	env << "Play this stream using the URL \"" << url << "\"\n";
	delete[] url;
}
