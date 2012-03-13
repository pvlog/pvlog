#ifndef DAEMON_H
#define DAEMON_H

#include <cstddef>

class DaemonWork {
public:
	virtual void work() = 0;
};

class Daemon {
protected:
	DaemonWork* work;

public:
	Daemon() : work(NULL) {}

	Daemon(DaemonWork *work)
	{
		this->work = work;
	}

	virtual ~Daemon() { delete work; }

	void setWork(DaemonWork* work)
	{
		this->work = work;
	}

	virtual void start() = 0;
};

#endif //#ifndef DAEMON_H
