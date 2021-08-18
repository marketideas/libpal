#include "httpserver.h"
#include "app.h"

#ifdef USE_NET

#define BASECLASS PAL_EVENTTASK
#define CLASS PAL_HTTPSERVERTASK


namespace PAL_NAMESPACE
{

PAL_HTTPREQUESTFACTORY::PAL_HTTPREQUESTFACTORY(PAL_HTTPSERVERTASK &_server) : server(_server)
{

}

HTTPRequestHandler* PAL_HTTPREQUESTFACTORY::createRequestHandler(const HTTPServerRequest &)
{
    return (new HTTPREQ_CLASS(server));
}


HTTPREQ_CLASS::HTTPREQ_CLASS(PAL_HTTPSERVERTASK &_server) : server(_server),app(Poco::Util::Application::instance())
{
}

void HTTPREQ_CLASS::handlePOST(std::string &method, HTTPServerRequest & req, HTTPServerResponse &resp)
{
    LOG_DEBUG << "Post" << endl;
}
void HTTPREQ_CLASS::handleGET(std::string &method, HTTPServerRequest & req, HTTPServerResponse &resp)
{
    LOG_DEBUG << "Get" << endl;
    HTTPResponse::HTTPStatus response=HTTPResponse::HTTP_NOT_FOUND;

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
    response=HTTPResponse::HTTP_OK;

    resp.setStatus(response);

}
void HTTPREQ_CLASS::handleGateway(std::string uri, std::string &method, HTTPServerRequest & req, HTTPServerResponse &resp)
{
    LOG_DEBUG << "Gateway" << endl;
}

void HTTPREQ_CLASS::handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp)
{
    uint32_t tid=Poco::Thread::currentTid();
    std::string client=req.clientAddress().host().toString();
    std::string secure=req.secure()?"SSL":"HTTP";
    std::string method=Poco::toUpper(req.getMethod());

    theuri=req.getURI();
    if (theuri=="")
    {
        theuri="/";
    }


    std::string hostname;
    try
    {
        hostname=req.getHost();
        Poco::StringTokenizer tok(hostname,":",Poco::StringTokenizer::TOK_IGNORE_EMPTY|Poco::StringTokenizer::TOK_TRIM);
        if (tok.count()>0)
        {
            hostname=tok[0];
        }
        else
        {
            hostname="";
        }

    }
    catch(...)
    {
        hostname="";
    }

    LOG_DEBUG << "Request(" <<method<<"):" << secure << " client: " << client << " host: " << hostname << " uri: " << theuri << " thread: " << tid << endl;

    if ((server.isSSL()) && (!req.secure()))
    {
        std::string p="";
        int port=getInt("https.port",443);
        if (port!=443)
        {
            p=":"+Poco::NumberFormatter::format(port);
        }

        //hostname="";
        if (hostname!="")
        {
            std::string r="https://"+hostname+p+theuri;
            //LOG_DEBUG << "redirecting: " << r << endl;
            resp.redirect(r,HTTPResponse::HTTP_MOVED_PERMANENTLY);
        }
        else
        {
            //LOG_DEBUG << "forbidden" << endl;
            resp.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST,"404 NOT FOUND");
            ostream &out = resp.send();
            out.flush();
        }
        return;
    }

    if (method=="GET")
        handleGET(method,req,resp);
    else if (method=="POST")
        handlePOST(method,req,resp);
    else
    {
        resp.setStatus(HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
    }
}

CLASS::CLASS(std::string taskName) : BASECLASS(taskName)
{
    params = NULL;
    threadpool = NULL;
    listenSocket = NULL;
    _isSSL = false;
    server = NULL;
    LOG_DEBUG << "server created" << endl;
}

CLASS::~CLASS()
{
    //LOG_NOTE << name() << " ~HTTPTask() " << endl;
    if (server != NULL)
    {
        server->stop();

#ifdef ENABLE_SSL
        if (isSSL())
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

        _isSSL=true;
        Poco::Net::initializeSSL();
        ReloadSSL();
        _isSSL=true;
        listenSocket = new Poco::Net::SecureServerSocket(SocketAddress(listenstr), numTasks);

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
        server  = new Poco::Net::HTTPServer(new PAL_HTTPREQUESTFACTORY(*this),
                                            *threadpool, *listenSocket, params);
        server->start();
        LOG_DEBUG << "httpserver started (" << name() << ")" << endl;
    }
}


void CLASS::ReloadSSL(void)
{
#ifdef ENABLE_SSL
    if (isSSL())
    {
        LOG_NOTE << "reloading ssl certificate" << endl;
        try
        {

            std::string certfile = getConfig("openSSL.server.certificateFile", "");
            std::string keyfile = getConfig("openSSL.server.privateKeyFile", "");
            std::string chainfile = getConfig("openSSL.server.caConfig", "");

            LOG_DEBUG << "certfile: " << certfile << endl;
            LOG_DEBUG << "keyfile: " << keyfile << endl;
            LOG_DEBUG << "chainfile: " << chainfile << endl;


            SSLManager &ssl = SSLManager::instance();
            //LOG_NOTE << "ssl 2" << endl;

            //ssl.initializeServer();
            Context::Ptr ctx = ssl.defaultServerContext();
            //LOG_NOTE << "ssl 3" << endl;
            if (!ctx.isNull())
            {

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
