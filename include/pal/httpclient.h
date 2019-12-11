#ifdef USE_NET
#pragma once
#include "palPoco.h"
#include "eventtask.h"
#include "pallogger.h"

#undef CLASS
#define CLASS PAL_HTTPCLIENTTASK

using namespace PAL_NAMESPACE;
namespace PAL_NAMESPACE {

class CLASS : public PAL_EVENTTASK
{
private:
	enum tcpStates { TCP_IDLE, TCP_CONNECT, TCP_REQUEST, TCP_RESPONSE, TCP_CLOSE, TCP_COMPLETE, TCP_TERM,
		TCP_MAX };
	uint32_t curState;
	uint64_t stateTime;
	TCallback callback;
	URI uri;
protected:
	void runRequest(void);
	uint32_t doRequest(Poco::Net::HTTPClientSession& session, Poco::Net::HTTPRequest& request,
                      Poco::Net::HTTPResponse& response);
public:
	CLASS(std::string name);
	virtual ~CLASS();
	virtual uint32_t setState(uint32_t newstate);
	virtual void get(std::string host, std:: string url, TCallback cb);
	virtual void onTimer(void);
	virtual void onOneSecond(void);
	virtual void onOneMinute(void);

	virtual void onQuit(void);
};

}

#undef CLASS
#endif
