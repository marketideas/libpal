#include "palutils.h"
#include "pwd.h"


namespace PAL_NAMESPACE
{

Poco::NullOutputStream nullstream;

//#define LOG_DEBUG nullstream
#define LOG_DEBUG cout

int isDebug(void)
{
	static int cached = 0;
#ifdef DEBUG
	static bool checked = false;

	if (!checked)
	{
		checked = true;
		cached = getInt("option.debug", 0);
	}
#endif
	return (cached);
}

uint64_t GetTickCount()
{
	static FastMutex _mutex;

	FastMutex::ScopedLock lock(_mutex);
	uint64_t x;
	struct timespec tv1;

	if (clock_gettime(CLOCK_MONOTONIC, &tv1) != 0)
	{
		return 0;
	}

	x = tv1.tv_sec;
	x = (x * 1000LL) + (tv1.tv_nsec / 1000000LL);
	return (x);
}

std::string getTimeAsString(void)
{
	std::string res = "";

	Poco::LocalDateTime now;
	res = Poco::DateTimeFormatter::format(now, DateTimeFormat::HTTP_FORMAT);
	return (res);
}

std::string getLibVersion(void)
{
	std::string s = "";
#ifdef DEBUG
	s = "(d)";
#endif
	return ((std::string)STRINGIFY(LIBVERSION) + s);
}

std::string getPocoVersion(void)
{
	Poco::Environment env;

	uint32_t version = env.libraryVersion();
	int major = (version >> 24) & 0xFF;
	int minor = (version >> 16) & 0xFF;
	int revision = (version >> 8) & 0xFF;
	int patch = (version) & 0xFF;
	std::string vstr = std::to_string(major) + "." + std::to_string(minor);
	vstr += "." + std::to_string(revision) + "." + std::to_string(patch);
	return (vstr);
}

std::string getEnv(std::string name, std::string dflt)
{
	std::string res = dflt;
	try
	{
		res = Poco::Environment::get(name);
	}
	catch (...)
	{
		res = dflt;
	}
	return (res);
}

std::string getConfig(std::string key, std::string dflt)
{
	std::string res = "";
	std::string f;

	try
	{
		f = Poco::Util::Application::instance().config().getString(key);
	}
	catch (...)
	{
		f = dflt;
	}
	f = SubstEnvironment(f);
	res = f;
	return (res);
}

int getInt(std::string key, int dflt)
{
	int x = dflt;
	std::string f, i;
	try
	{
		i = Poco::NumberFormatter::format(dflt);
		f = getConfig(key, i);
		x = Poco::NumberParser::parse(f);
	}
	catch (...)
	{
		x = dflt;
	}
	return (x);
}

bool toBool(std::string str)
{
	bool res = false;
	str = Poco::toUpper(str);
	if ((str == "Y") || (str == "T") || (str == "TRUE") || (str == "YES") || (str == "ON"))
	{
		res = true;
	}
	else
	{
		int x;
		try
		{
			x = Poco::NumberParser::parse(str);
		}
		catch (...)
		{
			x = 0;
		}
		if (x != 0)
		{
			res = true;
		}
	}
	return (res);
}

bool getBool(std::string key, bool dflt)
{
	bool res = false;
	std::string f;

	f = getConfig(key, dflt ? "1" : "0");
	res = toBool(f);
	return (res);
}

std::string SubstEnvironment(std::string instr)
{
	std::string env, res, f;
	uint32_t len, state, i;
	char ch;

	res = "";

	state = 0;
	len = instr.length();
	for (i = 0; i < len; i++)
	{
		ch = instr[i];
		switch (state)
		{
			case 0:
				if (ch == '%')
				{
					state = 1;
				}
				else if (ch == '$')
				{
					state = 4;
				}
				else
				{
					res = res + ch;
				}
				break;

			case 4:
				if (ch == '{')
				{
					env = "";
					state = 5;
				}
				else
				{
					res + res + '$' + ch;
					state = 0;
				}

				break;
			case 5:
				if (ch == '}')
				{
					f = getConfig(env, "${" + env + "}");
					res = res + f;
					env = "";
					state = 0;
				}
				else
				{
					env += ch;
				}
				break;
			case 1:
				if (ch == '%')
				{
					env = "";
					state = 2;
				}
				else
				{
					res = res + '%' + ch;
					state = 0;
				}
				break;
			case 2:
				if (ch == '%')
				{
					state = 3;
				}
				else
				{
					env = env + ch;
				}
				break;
			case 3:
				if (ch == '%')
				{
					f = getEnv(env, "%%" + env + "%%");
					res = res + f;
					env = "";
					state = 0;
				}
				else
				{
					env = env + '%' + ch;
					state = 2;
				}
				break;
		}
	}

	//cout << "Subst: |" << res << "| " << endl;
	return (res);
}

bool changeOwner(std::string filename, std::string userstr, std::string groupstr)
{
	bool res = false;
	int r;

	struct passwd *pw;

	struct group g;
	struct group *gptr = NULL;

	pw = getpwnam(userstr.c_str());
	if (pw != NULL)
	{

		std::string s = groupstr;
		char buff[1024];
		int gid = getgrnam_r(s.c_str(), &g, buff, sizeof(buff), &gptr);
		if (gid == 0)
		{
			r = chown(filename.c_str(), pw->pw_uid, g.gr_gid);
			if (r == 0)
			{
				res = true;
				//r = chmod(filename.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IWGRP | S_IROTH | S_IXOTH);
			}
		}
	}
	return (res);
}

bool changePermissions(std::string filename, uint32_t permissions)
{
	bool res = false;
	int r = chmod(filename.c_str(), permissions);
	if (r == 0)
	{
		res = true;
	}

	return (res);
}

uint8_t SetColor(uint8_t icolor)
{
	static uint8_t color = 0;
	static uint8_t bgcolor = 0;
	uint8_t res = color;
	static FastMutex _mutex;

	uint8_t col = 0;
	uint8_t bcol = 0;
	int attr = 0;
	//int battr = 0;
	uint8_t x = icolor & 0x07;

	if (!isatty(STDOUT_FILENO))
	{
		return(0);
	}

	switch (x)
	{
		case CL_RED:
			col = 1;
			attr = 1;
			break;
		case CL_YELLOW:
			col = 3;
			attr = 1;
			break;
		case CL_BLUE:
			col = 4;
			attr = 1;
			break;
		case CL_PURPLE:
			col = 5;
			attr = 1;
			break;
		case CL_GREEN:
			col = 2;
			attr = 1;
			break;
		case CL_CYAN:
			col = 6;
			attr = 1;
			break;
		case CL_WHITE:
			col = 7;
			attr = 1;
			break;
		default:
			col = 7;
			break;
	}
	if (icolor & CL_BOLD)
	{
		col |= CL_BOLD;
	}

	x = (icolor & 0x70);

	switch (x)
	{
		case BG_RED:
			bcol = 1;
			break;
		case BG_YELLOW:
			bcol = 3;
			break;
		case BG_BLUE:
			bcol = 4;
			break;
		case BG_PURPLE:
			bcol = 5;
			break;
		case BG_GREEN:
			bcol = 2;
			break;
		case BG_CYAN:
			bcol = 6;
			break;
		case BG_WHITE:
			bcol = 7;
			break;
		default:
			bcol = 0;
			break;
	}

	if ((col != color) || (bcol != bgcolor))
	{

		char command[32];
		int fg = col & 0x07;
		int bg = bcol & 0x07;
		//bg=0;
		if (col & CL_BOLD)
		{
			attr = 1;
		}

		FastMutex::ScopedLock lock(_mutex);

		//printf("col=%02X bg=%02X\n",fg,bg);
		sprintf(command, "%c[%d;%d;%dm", 0x1b, attr, fg + 30, bg + 40);
		printf("%s", command);
		fflush(stdout);
		color = col;
		bgcolor = bcol;
	}
	return (res);
}

}
