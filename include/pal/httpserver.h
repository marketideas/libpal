#pragma once
#include "config.h"
#include "pal_config.h"
#include "palPoco.h"
#include "eventtask.h"

#ifdef USE_NET

#define HTTPREQ_CLASS PAL_HTTPREQUESTHANDLER

namespace PAL_NAMESPACE {






#define CLASS PAL_HTTPSERVERTASK

class CLASS : public PAL_EVENTTASK
{
protected:
	Poco::Net::HTTPServer *server;
	ThreadPool *threadpool;
	ServerSocket *listenSocket;
	HTTPServerParams *params;
public:
	bool _isSSL;

	CLASS(std::string taskName);
	virtual ~CLASS();
	bool isSSL() { return _isSSL;};
	void initServer(std::string listenstr, bool ssl, int numTasks);
	virtual void onOneSecond(void);
	virtual void onOneDay(void);
	virtual void onKeyPress(char ch);
	virtual void ReloadSSL(void);
};

#undef CLASS

class HTTPREQ_CLASS : public HTTPRequestHandler
{
protected:
	PAL_HTTPSERVERTASK &server;
	std::string theuri;
	std::string dir;
	std::string docroot;
	std::string defaultfile;
	std::string postdata;
	std::stringstream reqbody;
	Application &app;
	std::string webPath;
	std::string appPath;
public:
	HTTPREQ_CLASS(PAL_HTTPSERVERTASK &_server);

	virtual void handlePOST(std::string &method,HTTPServerRequest & req, HTTPServerResponse &resp);
	virtual void handleGET(std::string &method,HTTPServerRequest & req, HTTPServerResponse &resp);
	virtual void handleGateway(std::string uri,std::string &method,HTTPServerRequest & req, HTTPServerResponse &resp);
	virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp);
};

class PAL_HTTPREQUESTFACTORY : public HTTPRequestHandlerFactory
{
protected:
	PAL_HTTPSERVERTASK &server;

public:
	PAL_HTTPREQUESTFACTORY(PAL_HTTPSERVERTASK &_server);
	virtual HTTPRequestHandler* createRequestHandler(const HTTPServerRequest &);
};


}
#endif
