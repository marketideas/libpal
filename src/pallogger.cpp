// this line indicates to the .h file that this is the source file
#define PAL_LOGGER_C
#include "pal.h"
#ifdef USE_LOGGER

#include "pallogger.h"
#include "baseapp.h"
#include "pwd.h"

#undef CLASS
#define CLASS PAL_LOGGER


namespace PAL_NAMESPACE {

#ifdef USE_LOGGER

bool CLASS::enabled;

CLASS::CLASS() :
	app(Application::instance()),
	thelogger(Logger::get(PAL_POCO_LOGNAME)),
	thelogstream(Logger::get(PAL_POCO_LOGNAME))

{
	formatter = NULL;
	logpattern = getConfig("log.pattern", "%t");;

	//logpattern = "[%P:%I] %Y-%m-%d %H:%M:%S:%i %z %t";
	init();
}

CLASS::CLASS(std::string pattern) :
	app(Application::instance()),
	thelogger(Logger::get(PAL_POCO_LOGNAME)),
	thelogstream(Logger::get(PAL_POCO_LOGNAME))
{
	formatter = NULL;
	logpattern = pattern;
	init();
}

CLASS::~CLASS()
{
}

std::string CLASS::createLogFile(std::string path)
{
	std::string res = "";
	bool isroot = false;
	struct passwd *pw;
	uint32_t uid;
	//uint32_t gid;
	std::string tmpstr, cuserstr, userstr;
	std::string  groupstr;
	std::string logfilename, logdirname;

	tmpstr=path;
	tmpstr = getEnv("NO_DISK_LOG", "");
	if ((tmpstr != "") || (!getBool("log.disklog", false)))
	{
		return (""); // no disk logging if this environment var set to anything
	}

	cuserstr = "";
	uid = geteuid();
	if (uid == 0)
	{
		isroot = true;
	}
	pw = getpwuid(uid);
	if (pw != NULL)
	{
		cuserstr = pw->pw_name;
	}

	std::string dflt = "./log_${application.name}/${application.name}.log";
	dflt = SubstEnvironment(dflt);
	logfilename = getConfig("log.logfile", dflt);

	if (isroot)
	{
		userstr = getConfig("daemon.user", cuserstr);
	}
	else
	{
		userstr = cuserstr;
	}
	groupstr = getConfig("daemon.group", "users");

	//cout << "current user: " << cuserstr << endl;
	//cout << "running user: " << userstr << endl;
	//cout << "running group: " << groupstr << endl;


	Poco::Path logpath(logfilename);
	logpath = logpath.makeAbsolute();
	Poco::Path ldir = logpath.parent();
	logdirname = ldir.toString();
	logfilename = logpath.toString();

	Poco::File fDir(ldir);
	Poco::File fFile(logpath);

	//cout << logfilename << endl;
	//cout << logdirname << endl;

	//bool dircreated=false;
	try
	{
		try
		{
			fDir.createDirectories();
			//dircreated=true;
		}
		catch (...)
		{
			//cout << "unable to create directories" << endl;
		}
		if (fDir.exists())
		{
			if (!fDir.isDirectory())
			{
				cerr << "Log Directory is not a directory" << endl;
				return ("");
			}
			if (isroot)
			{
				changeOwner(logdirname, userstr, groupstr);
				changePermissions(logdirname, S_IRWXU | S_IRGRP | S_IXGRP | S_IWGRP | S_IROTH | S_IXOTH);
			}

			try
			{
				changeOwner(logfilename, userstr, groupstr);
				changePermissions(logfilename, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH );
				fFile.createFile();
				changeOwner(logfilename, userstr, groupstr);
				changePermissions(logfilename, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH );
			}
			catch (...)
			{
				//cerr << "exception creating the log file" << endl;
			}


			// canWrite only checks the current user's primary group (usually his/her username). It does not allow
			// a file group privilege from one of the groups the user belongs to.
			//if (!fFile.canWrite())
			//{
			//	cerr << "unable to write to log file (root?)" << endl;
			//	return ("");
			//}

		}
		else
		{
			cerr << "Log directory does not exist and can not be created" << endl;
			return ("");
		}
		res = logfilename;

	}
	catch (Poco::Exception &e)
	{
		//cout << "Exception: " << e.displayText() << endl;
		res = "";
	}
	catch (...)
	{
		res = "";
	}

	//cout << logfilename << endl;
	//cout << logdirname << endl;

	return (res);
}

void CLASS::init()
{
	int level = 0;
	std::string diskfile = "";

	enabled = false;


	std::string logfilename = getConfig("log.logfile", "./out.log");

	diskfile = createLogFile(logfilename);

	usingXML = false;
	palloginstance = this;
	formatter = (new Poco::PatternFormatter);
	formatter->setProperty("times", "local");
	SetPattern(logpattern);

	AutoPtr<Poco::ColorConsoleChannel> pCons(new Poco::ColorConsoleChannel());
	AutoPtr<Poco::ConsoleChannel> pCons1(new Poco::ConsoleChannel());


	pCons->setProperty("enableColors", "true");
	pCons->setProperty("traceColor", "lightblue");
	pCons->setProperty("debugColor", "lightblue");
	pCons->setProperty("informationColor", "white");
	pCons->setProperty("noticeColor", "green");
	pCons->setProperty("warningColor", "yellow");
	pCons->setProperty("errorColor", "red");
	pCons->setProperty("criticalColor", "lightMagenta");
	pCons->setProperty("fatalColor", "lightMagenta");

	AutoPtr<Poco::SplitterChannel> pSplitter(new Poco::SplitterChannel());

	if (getBool("log.usecolor", true))
	{
		pSplitter->addChannel(pCons);
	}
	else
	{
		pSplitter->addChannel(pCons1);
	}

	if (diskfile == "")
	{
		//cout << "disk logging disabled" << endl;
	}
	if ((diskfile != "") && (true))
	{
		//cout << "Enabling disk log: " << diskfile << endl;

		AutoPtr<Poco::FileChannel> pFile(new FileChannel);

		pFile->setProperty("path", diskfile);
		pFile->setProperty("rotation", "512 K");
		pFile->setProperty("compress", "true");
		pFile->setProperty("times", "local");
		pFile->setProperty("archive", "timestamp");
		pFile->setProperty("purgeAge", "7 days");

		pSplitter->addChannel(pFile);

	}

	AutoPtr<Poco::FormattingChannel> pFC(new Poco::FormattingChannel(formatter, pSplitter));

	if (getBool("log.async", true))
	{
		AutoPtr<Poco::AsyncChannel> pAsync(new Poco::AsyncChannel(pFC));
		Logger::get(PAL_POCO_LOGNAME).setChannel(pAsync);
	}
	else
	{
		Logger::get(PAL_POCO_LOGNAME).setChannel(pFC);
	}


	std::string lev = "";
	try
	{
		lev = Poco::Environment::get("LOG_LEVEL");
	}
	catch (...)
	{
		lev="";
	}
	if (lev == "")
	{
		lev = getConfig("log.loglevel", "notify");
	}

	int x = isDebug();
	if (x > 1)
	{
		lev = "debug";
	}

	if (lev == "")
	{
		lev = "notify";
	}
	//cout << " level: " << lev << endl;
	if (lev != "")
	{
		try
		{
			level = Poco::Logger::parseLevel(lev);
		}
		catch (...)
		{
			level = Poco::Message::PRIO_NOTICE;
		}
	}
	SetLevel(level);

	//cout << "logger init out" << endl;

}

bool CLASS::enable(bool on)
{
	bool res = enabled;
	enabled = on;
	return (res);
}

int CLASS::SetLevel(int newLevel)
{
	int res = logLevel;

	if (newLevel > Poco::Message::PRIO_TRACE)
	{
		newLevel = Poco::Message::PRIO_TRACE;
	}
	if (newLevel < Poco::Message::PRIO_FATAL)
	{
		newLevel = Poco::Message::PRIO_FATAL;
	}
	logLevel = newLevel;
	thelogger.setLevel(logLevel);
	return (res);
}

void CLASS::SetPattern(const std::string & newpat)
{
	if (formatter)
	{
		formatter->setProperty("pattern", newpat);
	}
}

std::ostream& CLASS::logstream(int outlevel, const char *file, const int line)
{
	//FastMutex::ScopedLock lock(_mutex);

	UNUSED(line);
	
	if ((outlevel <= 0) || (palloginstance == NULL) || (!enabled))
	{
		return (PAL_LOGGER::lognullstream);
	}

	std::string f = file;
	Poco::Message::Priority p;
	p = Message::PRIO_FATAL;
	switch (outlevel)
	{
		default: p = Message::PRIO_FATAL; break;
		case 1: p = Message::PRIO_FATAL; break;
		case 2: p = Message::PRIO_CRITICAL; break;
		case 3: p = Message::PRIO_ERROR; break;
		case 4: p = Message::PRIO_WARNING; break;
		case 5: p = Message::PRIO_NOTICE; break;
		case 6: p = Message::PRIO_INFORMATION; break;
		case 7: p = Message::PRIO_DEBUG; break;
		case 8: p = Message::PRIO_TRACE; break;

	}
	palloginstance->thelogstream.priority(p);
	//thelogstream << "[" << f << ":" << line << "]: ";
	return (palloginstance->thelogstream);
}
#endif
Poco::NullOutputStream CLASS::lognullstream;

}
#endif
