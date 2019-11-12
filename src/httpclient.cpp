#include "pal.h"

#define CLASS PAL_HTTPCLIENTTASK
#define BASE_CLASS PAL_EVENTTASK


namespace PAL_NAMESPACE
{

CLASS::CLASS(std::string name) : BASE_CLASS(name)
{
	timermax = 50;
	timerval = 0;
	setState(TCP_IDLE);
}

CLASS::~CLASS()
{
}

uint32_t CLASS::setState(uint32_t newState)
{
	uint32_t res = curState;
	if (curState != newState)
	{
		stateTime = GetTickCount();
		curState = newState;
		//LOG_DEBUG << "state=" << newState << endl;
	}
	return (res);
}

void CLASS::get(std::string host, std::string url, TCallback cb)
{
	// do this in a specific order because it is being done in a different thread
	uri = host + url;
	callback = cb;
	setState(TCP_REQUEST);
	timermax = 10; // switch to 10ms high frequency timer
}

void CLASS::onOneSecond(void)
{
	BASE_CLASS::onOneSecond();
	//LOG_DEBUG << "Time: " << getTimeAsString() << endl;

}

void CLASS::onOneMinute(void)
{
	BASE_CLASS::onOneSecond();
	runRequest();
}

void CLASS::onQuit(void)
{
	BASE_CLASS::onQuit();
	LOG_DEBUG << "quit" << endl;
}

void CLASS::onTimer(void)
{
	BASE_CLASS::onTimer();
	//uint64_t tick;

	//tick = GetTickCount();
	switch (curState)
	{
		default:
			setState(TCP_IDLE);
			break;
		case TCP_IDLE:
			timermax = 200;
			break;
		case TCP_REQUEST:
			runRequest();  // after the result is given via call back, the parent/owner can call and get the data before the next 'get'
			setState(TCP_IDLE);
			break;
	}
}

void CLASS::runRequest(void)
{
	uint32_t result = 0;

	std::string path(uri.getPathAndQuery());
	std::string scheme = uri.getScheme();
	if (path.empty())
	{
		path = "/";
	}
#if 0
	cout << scheme << endl;
	cout << uri.getHost() << endl;
	cout << uri.getPort() << endl;
	cout << path << endl;
#endif

	HTTPRequest request(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
	HTTPResponse response;

	if (scheme == "http")
	{
		HTTPClientSession session(uri.getHost(), uri.getPort());
		result=doRequest(session, request, response);
	}
	else if (scheme == "https")
	{
		HTTPSClientSession session(uri.getHost(), uri.getPort());
		result=doRequest(session, request, response);
	}
	if (result>0)
	{
		//LOG_DEBUG << "request OK" << endl;
		//response.write(cout);
	}
	else
	{
		result=0;
		//LOG_DEBUG << "request FAIL" << endl;
	}
	if (callback != NULL)
	{
		callback(result);
		//callback = NULL;
	}
}

uint32_t CLASS::doRequest(Poco::Net::HTTPClientSession& session, Poco::Net::HTTPRequest& request,
                      Poco::Net::HTTPResponse& response)
{
	uint32_t reqresult = 0;
	try
	{
		uint32_t r;
		session.sendRequest(request);
		std::istream& rs = session.receiveResponse(response);
		r = response.getStatus();

		LOG_INFO << r << " " << response.getReason() << std::endl;
		if (r != Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED)
		{
			reqresult=r;
			//std::ofstream ofs("Poco_banner.jpg", std::fstream::binary);
			Poco::NullOutputStream nullstream;
			StreamCopier::copyStream(rs, nullstream);
		}
	}
	catch (Poco::Exception &e)
	{
		//LOG_WARN << e.displayText() << endl;
		reqresult = 0;
	}
	return (reqresult);
}

}
