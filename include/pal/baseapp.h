#pragma once

#include "palPoco.h"

#include <sys/time.h>
#ifndef NO_SIGNAL_HANDLING
#include <sys/timerfd.h>
#endif
#include <iostream>
#include <stdexcept>
#include <termios.h>
#include <signal.h>
#include "palutils.h"
#include "pallogger.h"
#include "eventtask.h"

#define CLASS PAL_BASEAPP

#ifdef NO_SIGNAL_HANDLING
#define POCO_NO_SIGNAL_HANDLER NO_SIGNAL_HANDLING
#endif

#ifdef SERVERAPP
#define POCOAPP_CLASS Poco::Util::ServerApplication
#else
#define POCOAPP_CLASS Poco::Util::Application
#endif

namespace PAL_NAMESPACE {

class CLASS;

extern bool appShutdown;
extern PAL_BASEAPP *appFactory(void);

typedef struct {
	std::string name;
	std::string shortname;
	std::string description;
	std::string  argument;
	bool required;
	bool repeatable;
} programOption;

extern programOption appOptions[];

class TQA_SignalHandler : public Poco::SignalHandler
{
public:
	TQA_SignalHandler() : Poco::SignalHandler() {
	}

	static void install()
	{
#ifndef POCO_NO_SIGNAL_HANDLER
		struct sigaction sa;
		sa.sa_handler = handleSignal;
		sa.sa_flags   = 0;
		sigemptyset(&sa.sa_mask);
		sigaction(SIGILL,  &sa, 0);
		sigaction(SIGBUS,  &sa, 0);
		sigaction(SIGSEGV, &sa, 0);
		sigaction(SIGSYS,  &sa, 0);
		sigaction(SIGUSR1,  &sa, 0);
		sigaction(SIGHUP,  &sa, 0);
		sigaction(SIGINT,  &sa, 0);
		sigaction(SIGQUIT,  &sa, 0);
		sigaction(SIGTRAP,  &sa, 0);
		sigaction(SIGABRT,  &sa, 0);
		sigaction(SIGKILL,  &sa, 0);
		sigaction(SIGUSR2,  &sa, 0);
		sigaction(SIGIO,  &sa, 0);
		sigaction(SIGSYS,  &sa, 0);
		sigaction(SIGPWR,  &sa, 0);
		sigaction(SIGTERM,  &sa, 0);
#endif
	}
};

#ifndef POCO_NO_SIGNAL_HANDLER
#define qa_throw_on_signal \
	TQA_SignalHandler _qa_signalHandler; \
	int _qa_signal = sigsetjmp(_qa_signalHandler.jumpBuffer(), 1); \
	if (_qa_signal) _qa_signalHandler.throwSignalException(_qa_signal);
#else
#define qa_throw_on_signal
#endif

class CLASS : public POCOAPP_CLASS
{
private:
#ifndef NOT_TTY_SETUP
	struct termios oldSettings, newSettings;
#endif
	void ResetTerminal(void);
	void SetTerminal(void);

protected:
	PAL_LOGGER  *pallogger;
	PAL_EVENTMANAGER *evtManager;
	ArgVec commandargs;

	virtual int runApp(void);
	virtual int runCommandLineApp(void);
	virtual int runServerApp(PAL_EVENTMANAGER *em);

	char getKey(void);

public:
	CLASS();
	~CLASS();

protected:

	virtual bool InitApp(void);
	virtual void initialize(Application& self);
	virtual void uninitialize();
	virtual void reinitialize(Application& self);
	virtual void defineOptions(OptionSet& options);
	virtual void handleSingleOption(const std::string &name, const std::string &value);
	virtual void handleParamOption(const std::string& name, const std::string& value);
	virtual void displayHelp();
	virtual void displayVersion();

	int main(const ArgVec& args);
	void printProperties(const std::string& base);
};


#undef CLASS
}
