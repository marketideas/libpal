#include "baseapp.h"
#include <termios.h>
#include <signal.h>

#define CLASS PAL_BASEAPP

namespace PAL_NAMESPACE
{

CLASS::CLASS()
{
	evtManager = NULL;
	pallogger = NULL;
}

CLASS::~CLASS()
{
	if (pallogger != NULL)
	{
		delete(pallogger);
		pallogger = NULL;
	}
}

void CLASS::uninitialize()
{
	// add your own uninitialization code here
	POCOAPP_CLASS::uninitialize();
}

void CLASS::reinitialize(Application& self)
{
	POCOAPP_CLASS::reinitialize(self);
	// add your own reinitialization code here
}

void CLASS::defineOptions(OptionSet& options)
{
	POCOAPP_CLASS::defineOptions(options);

	int ct = 0;
	programOption *ptr = &appOptions[ct++];
	while (ptr->name != "")
	{
		if (ptr->argument == "") // this is a single option letter, etc (no parameter)
		{
			options.addOption(
			    Option(ptr->name, ptr->shortname, ptr->description)
			    .required(ptr->required)
			    .repeatable(ptr->repeatable)
			    .callback(OptionCallback<CLASS>(this, &CLASS::handleSingleOption)));
		}
		else
		{
			options.addOption(
			    Option(ptr->name, ptr->shortname, ptr->description)
			    .required(ptr->required)
			    .repeatable(ptr->repeatable)
			    .argument(ptr->argument)
			    .callback(OptionCallback<CLASS>(this, &CLASS::handleParamOption)));
		}
		ptr = &appOptions[ct++];
	}

	options.addOption(
	    Option("version", "v", "display application version")
	    .required(false)
	    .repeatable(false)
	    .callback(OptionCallback<PAL_BASEAPP>(this, &PAL_BASEAPP::handleSingleOption)));

	options.addOption(
	    Option("help", "h", "display help information on command line arguments")
	    .required(false)
	    .repeatable(false)
	    .callback(OptionCallback<PAL_BASEAPP>(this, &PAL_BASEAPP::handleSingleOption)));
}


void CLASS::handleSingleOption(const std::string &name, const std::string &value)
{
	int x;

	std::string dname;
	std::string dvalue=value;
	dname = "option." + name;
	x = getInt(dname, 0);
	x++;
	dvalue = Poco::NumberFormatter::format(x);
	config().setString(dname, dvalue);
}

void CLASS::handleParamOption(const std::string& name, const std::string& value)
{
	std::string dname;
	std::string dvalue;
	dname = "option." + name;
	dvalue = value;
	config().setString(dname, dvalue);
}

void CLASS::displayVersion()
{
	std::string s = "";
#ifdef DEBUG
	s = "(d)";
#endif
	cout << "Application Version: " << (std::string)STRINGIFY(APPVERSION) << s << endl;
	cout << "PAL Library Version: " << getLibVersion() << endl;
	cout << "Poco Library Version: " << getPocoVersion() << endl;
}

void CLASS::displayHelp()
{
	std::string usage = "<options>";
	std::string purpose = "sample Poco command line program";
#ifdef HELP_USAGE
	usage = HELP_USAGE;
#endif
#ifdef HELP_PURPOSE
	purpose = HELP_PURPOSE;
#endif

	HelpFormatter helpFormatter(options());
	helpFormatter.setCommand(commandName());
	helpFormatter.setUsage(usage);
	helpFormatter.setHeader(purpose);
	helpFormatter.setFooter(" ");

	cerr << endl;
	helpFormatter.format(std::cerr);

	if (getInt("option.debug", 0) > 1)
	{
		std::cout << endl << endl << endl;
		printProperties("");
	}
}

int CLASS::main(const ArgVec& args)
{
	int res = -1;

	commandargs = args;
	int v = getInt("option.version", 0);
	int h = getInt("option.help", 0);
	if ((v == 0) && (h == 0))
	{
		res = runApp();
	}
	else
	{
		if (h > 0)
		{
			displayHelp();
		}
		if (v > 0)
		{
			displayVersion();
		}
	}
	return res;
}

void CLASS::printProperties(const std::string & base)
{
	Poco::Util::AbstractConfiguration::Keys keys;
	config().keys(base, keys);
	if (keys.empty())
	{
		if (config().hasProperty(base))
		{
			std::string msg;
			msg.append(base);
			msg.append(" = ");
			msg.append(config().getString(base));
			logger().information(msg);
		}
	}
	else
	{
		for (Poco::Util::AbstractConfiguration::Keys::const_iterator it = keys.begin(); it != keys.end(); ++it)
		{
			std::string fullKey = base;
			if (!fullKey.empty()) { fullKey += '.'; }
			fullKey.append(*it);
			printProperties(fullKey);
		}
	}
}


void CLASS::initialize(Application & self)
{
	POCOAPP_CLASS::initialize(self);

	try
	{
		std::string f, appname;
		appname = self.config().getString("application.name", "");;

		try
		{
			f = Poco::Path::config() + appname + "/" + appname + ".ini";
			//cout << "first config " << f << endl;
			loadConfiguration(f);
		}
		catch (...)
		{
		}
		try
		{
			f = Poco::Path::current() + appname + ".ini";
			//cout << "second config " << f << endl;
			loadConfiguration(f);
		}
		catch (...)
		{
		}
		try
		{
			f = Poco::Path::configHome() + appname + ".ini";
			//cout << "third config " << f << endl;
			loadConfiguration(f);
		}
		catch (...)
		{
		}

#if 0
		f = getConfig("option.config - file", "");
		if (f == "")
		{
			//loadConfiguration(f);
		}
		if (f != "")
		{
			if (isDebug() > 1)
			{
				cout << "Loading Config file | " << f << " | " << endl;
			}
			loadConfiguration(f);
		}
#endif
	}
	catch (...)
	{
	}

	char *tz1 = getenv("TZ");
	if ((tz1 == NULL) || (strlen(tz1) == 0))
	{
		tz1 = (char *)"UTC";
	}
	std::string tz = getConfig("application.timezone", tz1);
	Poco::Environment::set("TZ", tz);

	pallogger = new PAL_LOGGER();
	pallogger->enable(true);
	//LOG_DEBUG << "tz=" << tz << endl;


}

bool CLASS::InitApp(void)
{
	return (true);
}

void CLASS::SetTerminal(void)
{
	tcgetattr( fileno( stdin ), &oldSettings );
	memcpy(&newSettings, &oldSettings, sizeof(oldSettings));
	newSettings.c_lflag &= (~ICANON & ~ECHO);
	tcsetattr( fileno( stdin ), TCSANOW, &newSettings );
}

void CLASS::ResetTerminal(void)
{
	tcsetattr( fileno( stdin ), TCSANOW, &oldSettings );

}

char CLASS::getKey(void)
{
	char res = 0;
	fd_set set;
	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = 500;

	FD_ZERO( &set );
	FD_SET( fileno( stdin ), &set );

	int r = select( FD_SETSIZE, &set, NULL, NULL, &tv );

	if ( r > 0 )
	{
		char c;
		int x = (int)read( fileno( stdin ), &c, 1 );
		if (x == 1)
		{
			res = c;
		}
	}
	else if ( r < 0 )
	{
	}
	else
	{
	}
	return (res);
}

int CLASS::runCommandLineApp(void)
{
	cout << "This is a command line application sample" << endl << endl;
	return (0);
}

int CLASS::runServerApp(PAL_EVENTMANAGER * em)
{
	if (em)
	{
	}
	// override this class to create your EventTasks
	return (-1);
}

int CLASS::runApp(void)
{
	int res = -1;
	int oldLogLevel = logger().getLevel();

	ThreadPool tpool(std::string("apppool"), 1, 4, 1, 256);
	TaskManager tm(tpool);


	int h = getInt("option.help", 0);
	int v = getInt("option.version", 0);

	if ((v + h)  == 0)
	{

		SetTerminal();
		try
		{
#ifndef NO_SIGNAL_HANDLING
			qa_throw_on_signal;
			_qa_signalHandler.install();
#endif
			if (InitApp())
			{

#ifdef SERVERAPP
				evtManager = new PAL_EVENTMANAGER();
				tm.start(evtManager);
				while (!evtManager->running)
				{
					usleep(10);
				}
				res = runServerApp(evtManager);

				if (res == 0)
				{
					res = Poco::Util::Application::EXIT_OK;

					bool appstop = false;
					while (!appstop)
					{
						usleep(100 * 1000);
						if (tm.count() == 0) // if the eventmanager task has shutdown, there is a fatal error
						{
							res = Poco::Util::Application::EXIT_SOFTWARE;
							appstop = true;
						}
						else
						{
							tick = GetTickCount();
							if (tick >= (lasttick + 1000))
							{
								evtManager->SendMessageToAll(Notification::Ptr(new PAL_MESSAGE(PAL_EVENTTASK::WM_ONESECOND)));
								lasttick = tick;
							}
							if (tick >= (mintick + (60 * 1000)))
							{
								evtManager->SendMessageToAll(Notification::Ptr(new PAL_MESSAGE(PAL_EVENTTASK::WM_ONEMINUTE)));

								mintick = tick;
							}
							if (tick >= (hourtick + (60 * 60 * 1000)))
							{
								evtManager->SendMessageToAll(Notification::Ptr(new PAL_MESSAGE(PAL_EVENTTASK::WM_ONEHOUR)));

								hourtick = tick;
							}

							if (tick >= (daytick + (24 * 60 * 60 * 1000)))
							{
								evtManager->SendMessageToAll(Notification::Ptr(new PAL_MESSAGE(PAL_EVENTTASK::WM_ONEDAY)));

								daytick = tick;
							}


							char ch = getKey();
							if (ch > 0)
							{
								switch (ch)
								{
									default:
										evtManager->SendMessageToAll(Notification::Ptr(new PAL_MESSAGE(PAL_EVENTTASK::WM_KEYPRESS, ch, 0, NULL)));
										break;
									case 'Q':
									case 'q':
										appstop = true;
										break;
								}
							}
						}
					}
				}
#else
				res = runCommandLineApp();
#endif
			}
		}
		catch (Poco::SignalException &e)
		{
			cerr << "RunApp() Poco::Signal: " << e.displayText() << endl;
			cerr.flush();
			res = Poco::Util::Application::EXIT_UNAVAILABLE;

		}
		catch (Poco::Exception &e)
		{
			cerr << "RunApp() Poco::Exception: " << e.displayText() << endl;
			cerr.flush();
			res = Poco::Util::Application::EXIT_UNAVAILABLE;


		}
		catch (std::exception &e)
		{
			cerr << "RunApp() std::exception: " << e.what() << endl;
			cerr.flush();
			res = Poco::Util::Application::EXIT_UNAVAILABLE;

		}
		catch (...)
		{
			cerr << "RunApp general exception" << endl;
			res = Poco::Util::Application::EXIT_UNAVAILABLE;
		}
		if (tm.count() > 0)
		{
			tm.cancelAll();
			tm.joinAll();
		}

		ResetTerminal();
		if (isDebug())
		{
			//LOG_DEBUG << "RunApp Exit: " << res << endl;
		}
		logger().setLevel(oldLogLevel);

	}
	return (res);
}

}
