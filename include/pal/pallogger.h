#pragma once

#include "palPoco.h"

// Logging includes
#include <Poco/Logger.h>
#include <Poco/LogStream.h>
#include <Poco/SplitterChannel.h>
#include <Poco/ConsoleChannel.h>

#include <Poco/FileChannel.h>
#include <Poco/FormattingChannel.h>
#include <Poco/AsyncChannel.h>
#include <Poco/SimpleFileChannel.h>
#include <Poco/PatternFormatter.h>
#include "palutils.h"

#undef CLASS
#define CLASS PAL_LOGGER

using Poco::Logger;
using Poco::FileChannel;
using Poco::AutoPtr;

#define PAL_POCO_LOGGER "pal_poco_logger"
#define PAL_POCO_LOGNAME PAL_POCO_LOGGER

namespace PAL_NAMESPACE
{


#define LOG_SYSLOG   cout
#define LOG_CONSOLE  cout
#define LOG_STDOUT  cout
#define LOG_STDERR  cerr

#ifdef USE_LOGGER
#define LOG_BASE(x,y,z) PAL_LOGGER::logstream(x,y,z)
#else
//#define LOG_BASE(x,y,z) PAL_LOGGER::logstream(-1,y,z)
#define LOG_BASE(x,y,z) LOG_STDERR
#endif

#define LOG_FATAL  LOG_BASE(1,__FILE__,__LINE__)
#define LOG_CRIT  LOG_BASE(2,__FILE__,__LINE__)
#define LOG_ERROR  LOG_BASE(3,__FILE__,__LINE__)
#define LOG_WARN  LOG_BASE(4,__FILE__,__LINE__)
#define LOG_NOTE  LOG_BASE(5,__FILE__,__LINE__)
#define LOG_INFO  LOG_BASE(6,__FILE__,__LINE__)
#define LOG_DEBUG  LOG_BASE(7,__FILE__,__LINE__)
#define LOG_TRACE  LOG_BASE(8,__FILE__,__LINE__)


class CLASS
{
#ifdef USE_LOGGER
private:
	mutable FastMutex _mutex;
protected:
	//bool enabled;
	Application &app;
	Logger &thelogger;
	std::string logpattern;
	bool usingXML;
	int logLevel;
	AutoPtr<Poco::PatternFormatter> formatter;

public:
	static Poco::NullOutputStream lognullstream;

	Poco::LogStream thelogstream;
	static bool enabled;

	CLASS(std::string pattern);
	CLASS();
	~CLASS();
	void init();
	void SetPattern(const std::string &newpat);
	int SetLevel(int newLevel);
	bool enable(bool val);
	std::string createLogFile(std::string path);

	static std::ostream& logstream(int outlevel,const char *file, const int line);
#else
public:
	static Poco::NullOutputStream lognullstream;

	CLASS(std::string pattern) { UNUSED(pattern);}
	CLASS() {}
	~CLASS() {}
	void init() {}
	void SetPattern(const std::string &newpat) {UNUSED(newpat);}
	int SetLevel(int newLevel) {UNUSED(newLevel); return 0;}
	bool enable(bool val) {UNUSED(val); return false;}
	std::string createLogFile(std::string path) {UNUSED(path); return ("");}

	static std::ostream& logstream(int outlevel, const char *file, const int line) {
		UNUSED(outlevel); UNUSED(file); UNUSED(line); return lognullstream; }
#endif
};


#ifndef PAL_LOGGER_C
extern CLASS *palloginstance;
#else
CLASS *palloginstance = NULL;
//Poco::NullOutputStream lognullstream;
#endif
#undef CLASS
}
