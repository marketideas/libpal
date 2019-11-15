#include "eventtask.h"
#include "baseapp.h"


#define CLASS PAL_EVENTTASK

using namespace PAL_NAMESPACE;

namespace PAL_NAMESPACE
{

CLASS::CLASS(std::string name) : Poco::Task(name),
	app(Application::instance())
{
	init();
}

CLASS::~CLASS(void)
{
}

void CLASS::init()
{
	timerval = rand() % 50;
	timermax = 10;
	stopped = false;
	critical = false;
}

void CLASS::handleMessage(Notification::Ptr note)
{
	PAL_MESSAGE *msg;
	if (!note.isNull())
	{
		msg = (PAL_MESSAGE *)note.get();
		switch (msg->msgType)
		{
			case WM_KEYPRESS:
				onKeyPress((char)msg->wParam);
				break;
			case WM_TIMER:
				onTimer();
				break;
			case WM_IDLE:
				onIdle();
				break;
			case WM_ONESECOND:
				onOneSecond();
				break;
			case WM_ONEMINUTE:
				onOneMinute();
				break;
			case WM_ONEHOUR:
				onOneHour();
				break;
			case WM_ONEDAY:
				onOneDay();
				break;
			case WM_RELOAD:
				onReload();
				break;
			case WM_USER:
				onUser(msg->wParam - WM_USER);
				break;

			default:
				//LOG_DEBUG << "unhandled palMessage(" << msg->msgType << ")" << endl;
				break;
		}
	}
}

void CLASS::onIdle(void)
{

}

void CLASS::onOneSecond(void)
{
	//LOG_DEBUG << "onOneSecond: " << name() << endl;
}

void CLASS::onOneMinute(void)
{
	//LOG_DEBUG << "onOneMinute: " << name() << endl;
}
void CLASS::onOneHour(void)
{
	//LOG_DEBUG << "onOneHour: " << name() << endl;
}
void CLASS::onOneDay(void)
{
	//LOG_DEBUG << "onOneDay: " << name() << endl;
}

void CLASS::onTimer(void)
{

}
void CLASS::onKeyPress(char ch)
{
	char buff[16];
	char c = ch;
	if (c < 32)
	{
		c = ' ';
	}
	sprintf(buff, "%02X", ch & 0xFF);
	std::string h = buff;
	if (isDebug() > 2)
	{
		LOG_DEBUG << "Key: " << c << " (0x" << h << ")" << endl;
	}
}

void CLASS::onQuit()
{
	doShutdown();
}

void CLASS::onUser(uint32_t idx)
{
	if(idx) {}

}
void CLASS::onReload(void)
{

}

void CLASS::doShutdown(void)
{
	stopped = true;
}

bool CLASS::checkIdle(void)
{
	return(true);
}

void CLASS::runIdle(void)
{
}

void CLASS::runTask(void)
{

	//qa_throw_on_signal;
	//_qa_signalHandler.install();

	stopped = false;
	Notification::Ptr nptr;

	while ((!isCancelled()) && (!stopped))
	{
		nptr=0;
		runIdle();
		if (checkIdle())
		{
			nptr = nq.waitDequeueNotification();
		}
		else
		{
			nptr = nq.dequeueNotification();
		}
		while ((!nptr.isNull()) && (!stopped) && (!isCancelled()))
		{
			handleMessage(nptr);
			FastMutex::ScopedLock lock(_mutex);
			nptr->release();
			nptr = nq.dequeueNotification();
		}
	}
	//printf("Task %s has exited\n", name().c_str());
	//LOG_DEBUG << "Task <" << name() << "> has exited" << endl;
}

void CLASS::cancel(void)
{
	Poco::Task::cancel();
	nq.wakeUpAll();
}

void CLASS::TerminateProgram(void)
{
	PAL_EVENTMANAGER &inst = PAL_EVENTMANAGER::instance();
	//inst.SendMessageToAll(Notification::Ptr(new PAL_MESSAGE(PAL_EVENTTASK::WM_QUIT)));
	//usleep(1000);
	inst.Shutdown();
}

void CLASS::SendMessage(Poco::Notification::Ptr ptr)
{

	if (!ptr.isNull())
	{
		PAL_MESSAGE *msg = dynamic_cast<PAL_MESSAGE *>(ptr.get());
		if (msg != NULL)
		{
			if (msg->msgType == WM_QUIT)
			{
				onQuit();
				stopped = true; // don't even post this message, just set the stopped flag.
				nq.wakeUpAll();
			}
			else
			{
				FastMutex::ScopedLock lock(_smMutex);
				ptr->duplicate();
				nq.enqueueNotification(ptr);
			}
		}
	}
}

uint32_t CLASS::setTimer(uint32_t ms)
{
	uint32_t res = timermax;
	timermax = ms;
	timerval = 0; // reset the timer
	return (res);
}

bool CLASS::checkTimer(uint64_t diff)
{
	bool res = false;
	if (timermax > 0)
	{
		timerval += diff;
		if (timerval >= timermax)
		{
			res = true;
			timerval = 0;
		}
	}
	return (res);
}

#undef CLASS
#define CLASS PAL_EVENTMANAGER

PAL_EVENTMANAGER* PAL_EVENTMANAGER::_pInstance = 0;

CLASS::CLASS(void) : Poco::Task("evtManager"), app(Application::instance()),
	tpool(std::string("evtPool"), 1, 64, 1, 256),
	tm(tpool)
{
	stopped = false;
	running = false;
	ataskhasstarted = false;
	_pInstance = this;
}

CLASS::~CLASS()
{
	_pInstance = 0;
}

inline CLASS& CLASS::instance()
{
	poco_check_ptr (_pInstance);
	return *_pInstance;
}

void CLASS::SendMessageToAll(Poco::Notification::Ptr ptr)
{
	if (!ptr.isNull())
	{
		FastMutex::ScopedLock lock(_mutex);
		try
		{
			int ct = tm.count();

			if (ct > 0)
			{
				TaskManager::TaskList list = tm.taskList();
				for (TaskManager::TaskList::iterator it = list.begin(); it != list.end(); ++it)
				{
					TaskManager::TaskPtr  t = *it;
					AutoPtr<PAL_EVENTTASK> p(t.cast<PAL_EVENTTASK>());
					if (!p.isNull())
					{
						p->SendMessage(ptr);
					}
				}
			}
		}
		catch (...)
		{
		}
	}
}

// standard tasks can terminate and the application continues to run
void CLASS::startTask(PAL_EVENTTASK *task)
{
	if (task != NULL)
	{
		task->critical = false;
		tm.start(task);
	}
}

// a 'critical' task is one that will terminate the entire application if it terminates
void CLASS::startCriticalTask(PAL_EVENTTASK *task)
{
	if (task != NULL)
	{
		task->critical = true;
		tm.start(task);
	}
}

void CLASS::doNotification(TaskNotification *notice)
{
	bool termapp = false;
	if (notice != NULL)
	{
		PAL_EVENTTASK *etask = dynamic_cast<PAL_EVENTTASK *>(notice->task());
		if (etask != NULL)
		{
			if (etask->critical)
			{
				TaskFailedNotification *failed = dynamic_cast<TaskFailedNotification *>(notice);
				if (failed != NULL)
				{
					//LOG_FATAL << "critical task fail " << etask->name() << endl;
					termapp = true;
				}
				TaskFinishedNotification *finished = dynamic_cast<TaskFinishedNotification *>(notice);
				if (finished != NULL)
				{
					//LOG_FATAL << "critical task finished " << etask->name() << endl;
					termapp = true;
				}
			}
		}

		notice->release();
	}

	if (termapp)
	{
		running = false;
	}
}

void CLASS::runTask(void)
{

	uint64_t tick, lasttick, diff;
	Observer<CLASS, TaskNotification> obs(*this, &CLASS::doNotification);

	tm.addObserver(obs);

	lasttick = tick = GetTickCount();
	running = true;

	while ((!isCancelled()) && (!stopped) && (running))
	{
		//printf("em running %d %d\n",isCancelled(),stopped);
		tick = GetTickCount();
		int x = tm.count();
		if (x > 0)
		{
			ataskhasstarted = true;
			diff = tick - lasttick;
			lasttick = tick;
			FastMutex::ScopedLock lock(_mutex);

			TaskManager::TaskList list = tm.taskList();
			for (TaskManager::TaskList::iterator it = list.begin(); it != list.end(); ++it)
			{
				TaskManager::TaskPtr  t = *it;
				AutoPtr<PAL_EVENTTASK> p = t.cast<PAL_EVENTTASK>();
				if (!p.isNull())
				{

					if (p->checkTimer(diff))
					{
						p->SendMessage(Notification::Ptr(new PAL_MESSAGE(PAL_EVENTTASK::WM_TIMER, 0, 0, NULL)));
					}
				}
			}
		}
		else if ((x == 0) && (ataskhasstarted))
		{
			stopped = true;
			//LOG_FATAL << "all tasks have completed " << endl;
		}
		if (!stopped)
		{
			sleep(10);
		}
	}

	stopAll();

	tm.removeObserver(obs);

	running = false;

}

void CLASS::Shutdown(void)
{
	cancel();
}

void CLASS::stopAll(void)
{
	if (!stopped)
	{
		if (tm.count() > 0)
		{
			SendMessageToAll(Notification::Ptr(new PAL_MESSAGE(PAL_EVENTTASK::WM_QUIT)));
			tm.cancelAll();
			tm.joinAll();
		}
		if ((!stopped) && (state() < TaskState::TASK_CANCELLING))
		{
			//cancel();
		}
	}
	stopped = true;
}

#undef CLASS
}