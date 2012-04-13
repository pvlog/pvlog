#ifndef DAEMON_H
#define DAEMON_H

#include <cstddef>
#include <memory>

class DaemonWork {
public:
	virtual void work() = 0;
};

class Daemon {
protected:
	std::unique_ptr<DaemonWork> work;

public:
	Daemon()
	{
		/* nothing to do */
	}

	Daemon(std::unique_ptr<DaemonWork> work) :
		work(std::move(work))
	{
		//nothing to do
	}

	virtual ~Daemon()
	{
		/* nothing to do */
	}

	void setWork(std::unique_ptr<DaemonWork> work)
	{
		this->work = std::move(work);
	}

	virtual void start() = 0;
};

#endif //#ifndef DAEMON_H
