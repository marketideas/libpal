#include "app.h"

#define CLASS PAL_APPCLASS

// return a pointer to the actual Application class
PAL_BASEAPP *PAL::appFactory(void)
{
	return (new CLASS());
}

// you MUST supply this array 'appOptions'.  NULL line and end.
programOption PAL::appOptions[] = {
{ "debug","d","enable debug info (repeat for more verbosity)","",false,true},
{ "config-file","f","load configuration data from a <file>","file",false,false},
{ "","","","",false,false}
};


int CLASS::runServerApp(PAL_EVENTMANAGER *em)
{
	int res=-1;
	if (em != NULL)
	{
		PAL_BASEAPP::runServerApp(em);
		PAL_HTTPSERVERTASK *server = new PAL_HTTPSERVERTASK("httptask");
		if (server != NULL)
		{
			em->startTask(server);
			server->initServer(getConfig("http.listen","0.0.0.0:9080"),false,64);
			res=0;
		}
	}
	return(res);
}

int CLASS::runCommandLineApp(void)
{
	// only called if SERVERAPP not defined
	int res = -1;

	LOG_DEBUG << "command line mode" << endl;
	for (ArgVec::const_iterator it = commandargs.begin(); it != commandargs.end(); ++it)
	{
		logger().information(*it);
		res = 0;
	}

	return (res);
}



