#pragma once
#include "palPoco.h"
#include "pallogger.h"

#undef CLASS
#define CLASS PAL_MESSAGE

namespace PAL_NAMESPACE {

class CLASS: public Notification
{
private:
	CLASS() { } ;
public:
	typedef AutoPtr<CLASS> Ptr;

	uint32_t msgType;
	uint32_t wParam;
	uint64_t lParam;
	void    *ptr;
	CLASS(uint32_t _type, uint32_t _wParam, uint64_t _lParam, void *_ptr) : Notification(),
		msgType(_type),
		wParam(_wParam),
		lParam(_lParam),
		ptr(_ptr) { }
	CLASS(uint32_t _type) : Notification(), msgType(_type), wParam(0), lParam(0), ptr(NULL) {}
	// ~CLASS() { cout << "free" << endl; }
};

#undef CLASS
#define CLASS PAL_EVENTTASK

class CLASS : public Poco::Task
{
public:
	typedef std::function<void(uint32_t)> TCallback;

	enum WM_MESSAGE
	{
		WM_NULL = 0, WM_IDLE,WM_TIMER,WM_TICK, WM_KEYPRESS, WM_ONESECOND, WM_ONEMINUTE, WM_ONEHOUR,WM_ONEDAY, WM_RELOAD,
		WM_QUIT, WM_USER,
		WM_MAXMESSAGE
	};
	bool critical;
private:
	mutable Poco::FastMutex _mutex; 
	mutable Poco::FastMutex _smMutex;

protected:
	bool stopped;
	Poco::NotificationQueue nq;
	Application &app;
	uint64_t timermax, timerval;

	virtual void init();
	virtual void doEvents(void);


public:
	CLASS(std::string name);

	virtual ~CLASS(void);
	virtual void handleMessage(Poco::Notification::Ptr note);
	virtual void runTask(void);
	virtual void doShutdown(void);
	virtual void cancel(void);
	virtual void TerminateProgram(void);


	// message handler methods
	virtual void onTick(void);
	virtual void onOneSecond(void);
	virtual void onOneMinute(void);
	virtual void onOneHour(void);
	virtual void onOneDay(void);
	virtual void onKeyPress(char ch);
	virtual void onTimer(void);
	virtual void onQuit(void);
	virtual void onReload(void);
	virtual void onIdle(void);
	virtual bool checkIdle(void);
	virtual void runIdle(void);

	virtual void onUser(uint32_t idx);

	void SendMessage(Poco::Notification::Ptr ptr);
	bool checkTimer(uint64_t diff);
	uint32_t setTimer(uint32_t ms);
};

#undef CLASS
#define CLASS PAL_EVENTMANAGER

class CLASS : public Poco::Task
{
private:
	bool stopped;
	mutable FastMutex _mutex;
	bool ataskhasstarted;
	static CLASS* _pInstance;

protected:
	Poco::NotificationQueue nq;
	Application &app;
	ThreadPool tpool;
	Poco::TaskManager tm;

	void doNotification(TaskNotification *notice);
	void stopAll(void);
	virtual void runTask(void);

public:
	bool running;

	CLASS();
	virtual ~CLASS(void);

	static CLASS &instance();

	void startTask(PAL_EVENTTASK *task);
	void startCriticalTask(PAL_EVENTTASK *task);

	void Shutdown(void);
	void SendMessageToAll(Poco::Notification::Ptr  ptr);
};


#undef CLASS

}