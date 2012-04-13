#ifndef FORGROUND_DAEMON_H
#define FORGROUND_DAEMON_H

#include "Daemon.h"
#include "Log.h"

class ForgroundDaemon: public Daemon {
public:
	ForgroundDaemon(std::unique_ptr<DaemonWork> work) :
		Daemon(std::move(work))
	{
		/*nothing to do */
	}

	virtual void start()
	{
		LOG(Debug) << "Starting foreground work.";
		work->work();
	}
};

#endif //#ifndef FORGROUND_DAEMON_H
