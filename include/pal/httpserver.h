#pragma once
#include "palPoco.h"
#include "eventtask.h"

#define HTTPREQ_CLASS PAL_HTTPREQUESTHANDLER

namespace PAL_NAMESPACE {

class PAL_HTTPREQUESTFACTORY : public HTTPRequestHandlerFactory
{
public:
	virtual HTTPRequestHandler* createRequestHandler(const HTTPServerRequest &);
};

class HTTPREQ_CLASS : public HTTPRequestHandler
{
protected:
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
	HTTPREQ_CLASS();

	virtual void handlePOST(std::string &method,HTTPServerRequest & req, HTTPServerResponse &resp);
	virtual void handleGET(std::string &method,HTTPServerRequest & req, HTTPServerResponse &resp);
	virtual void handleGateway(std::string uri,std::string &method,HTTPServerRequest & req, HTTPServerResponse &resp);
	virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp);
};


#define CLASS PAL_HTTPSERVERTASK

class CLASS : public PAL_EVENTTASK
{
protected:
	Poco::Net::HTTPServer *server;
	ThreadPool *threadpool;
	ServerSocket *listenSocket;
	HTTPServerParams *params;
	bool isSSL;
public:
	CLASS(std::string taskName);
	virtual ~CLASS();
	void initServer(std::string listenstr, bool ssl, int numTasks);
	virtual void onOneSecond(void);
	virtual void onOneDay(void);
	virtual void onKeyPress(char ch);
	virtual void ReloadSSL(void);
};

#undef CLASS
}