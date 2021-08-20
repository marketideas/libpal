#pragma once

// class names
#define PAL_NAMESPACE    PAL
#define PAL_BASEAPP		 TBaseApp
#define PAL_EVENTMANAGER TEventManager
#define PAL_EVENTTASK    TEventTask
#define PAL_MESSAGE      TMessage
#define PAL_LOGGER		 TLogger

#define PAL_HTTPSERVERTASK THttpServerTask
#define PAL_HTTPREQUESTHANDLER THttpRequestHandler
#define PAL_HTTPREQUESTFACTORY THttpRequestFactory

#define PAL_HTTPCLIENTTASK THttpClientTask

#ifndef PAL_APPCLASS
#warning "PAL_APPCLASS is undefined. define it in config.h"
#define PAL_APPCLASS TPALCustomApplication
#endif

#ifndef DEBUG
#undef RELEASE
#define RELEASE
#endif

namespace PAL_NAMESPACE
{

}
using namespace PAL_NAMESPACE;
