#ifdef USE_NET
#include "httpserver.h"

#define BASECLASS PAL_EVENTTASK
#define CLASS PAL_HTTPSERVERTASK


namespace PAL_NAMESPACE
{

HTTPRequestHandler* PAL_HTTPREQUESTFACTORY::createRequestHandler(const HTTPServerRequest &)
{
	return (new HTTPREQ_CLASS());
}


HTTPREQ_CLASS::HTTPREQ_CLASS() : app(Poco::Util::Application::instance())
{
}

void HTTPREQ_CLASS::handlePOST(std::string &method, HTTPServerRequest & req, HTTPServerResponse &resp)
{
}
void HTTPREQ_CLASS::handleGET(std::string &method, HTTPServerRequest & req, HTTPServerResponse &resp)
{
}
void HTTPREQ_CLASS::handleGateway(std::string uri, std::string &method, HTTPServerRequest & req, HTTPServerResponse &resp)
{
}
void HTTPREQ_CLASS::handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp)
{
	resp.setStatus(HTTPResponse::HTTP_OK);
	resp.setContentType("text/plain");
	ostream &out = resp.send();
	Poco::LocalDateTime now;
	Poco::DateTime unow;
	std::string filename;
	std::string dt = DateTimeFormatter::format(now, "%Y-%m-%dT%H:%M:%S.%i%Z");
	std::string dt1 = DateTimeFormatter::format(unow, "%Y-%m-%dT%H:%M:%S.%i%Z");
	out << "Local: " << dt << endl;
	out << "UTC: " << dt1 << endl;
	out.flush();
}

CLASS::CLASS(std::string taskName) : BASECLASS(taskName)
{
	params = NULL;
	threadpool = NULL;
	listenSocket = NULL;
	isSSL = false;
	server = NULL;
}

CLASS::~CLASS()
{
	//LOG_NOTE << name() << " ~HTTPTask() " << endl;
	if (server != NULL)
	{
		server->stop();

#ifdef ENABLE_SSL
		if (isSSL)
		{
			Poco::Net::uninitializeSSL();
		}
#endif

		delete(server);
		server = NULL;
	}
	if (threadpool != NULL)
	{
		threadpool->stopAll();
		threadpool->joinAll();
		delete(threadpool);
		threadpool = NULL;
	}
	if (params != NULL)
	{
		// don't do anything with params, it causes a crash, so something else is deleting
		//params->release();
		params = NULL;
	}
	if (listenSocket != NULL)
	{
		delete(listenSocket);
		listenSocket = NULL;
	}
}

void CLASS::initServer(std::string listenstr, bool ssl, int numTasks)
{
	if (numTasks < 4)
	{
		numTasks = 4;
	}
	if (numTasks > 128)
	{
		numTasks = 128;
	}
	//LOG_DEBUG << "numtasks: " << numTasks << endl;

	params = new HTTPServerParams();
	params->setMaxThreads(numTasks);
	params->setTimeout(Poco::Timespan(5, 0));
	params->setSoftwareVersion("pocoHTTP/" + (std::string)STRINGIFY(APPVERSION));
	params->setKeepAlive(getBool("http.keepalive", false));
	params->setThreadIdleTime(Poco::Timespan(1L, 0L));
	params->setMaxQueued(numTasks);

	int idleTime = 5;
	threadpool = new ThreadPool(1, numTasks, idleTime, 128);

	//LOG_DEBUG << "SSL: " << ssl << endl;
	if (ssl)
	{
#ifdef ENABLE_SSL

		Poco::Net::initializeSSL();

		listenSocket = new Poco::Net::SecureServerSocket(SocketAddress(listenstr), numTasks);
		isSSL = true;
#else
		listenSocket = new Poco::Net::ServerSocket(SocketAddress(listenstr), numTasks);
#endif
	}
	else
	{
		listenSocket = new ServerSocket(SocketAddress(listenstr), numTasks);
	}
	if ((threadpool != NULL) && (listenSocket != NULL) && (params != NULL))
	{
		server  = new Poco::Net::HTTPServer(new PAL_HTTPREQUESTFACTORY(),
		                                    *threadpool, *listenSocket, params);
		server->start();
		LOG_DEBUG << "httpserver started (" << name() << ")" << endl;
	}
}


void CLASS::ReloadSSL(void)
{
#ifdef ENABLE_SSL
	if (isSSL)
	{
		LOG_NOTE << "reloading ssl certificate" << endl;
		try
		{
			SSLManager &ssl = SSLManager::instance();

			Context::Ptr ctx = ssl.defaultServerContext();
			if (!ctx.isNull())
			{

				std::string certfile = getConfig("openSSL.server.certificateFile", "");
				std::string keyfile = getConfig("openSSL.server.privateKeyFile", "");
				std::string chainfile = getConfig("openSSL.server.caConfig", "");

				Poco::Crypto::X509Certificate cert(certfile);
				Poco::Crypto::RSAKey key("", keyfile, "");

				ctx->useCertificate(cert);
				ctx->usePrivateKey(key);
				LOG_NOTE << "new SSL cert file loaded" << endl;
			}
		}
		catch (Poco::Exception &e)
		{
			LOG_WARN << "SSL Reload exception: " << e.displayText() << endl;
		}
	}
#endif
}

void CLASS::onOneSecond(void)
{
	BASECLASS::onOneSecond();
}

void CLASS::onOneDay(void)
{
	BASECLASS::onOneSecond();
	ReloadSSL();
}

void CLASS::onKeyPress(char ch)
{
	BASECLASS::onKeyPress(ch);

	if ((ch == 's') && (name() == "http_task"))
	{
		char *ptr = NULL;
		char ch = *ptr;
		if (ch)
		{
		}
	}
	if ((ch == 't') && (name() == "https_task"))
	{
		throw SignalException("task exception");
	}
}

#undef CLASS

}
#endif
