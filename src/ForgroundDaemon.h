#ifndef FORGROUND_DAEMON_H
#define FORGROUND_DAEMON_H

#include "Daemon.h"

class ForgroundDaemon : public Daemon {
public:
	ForgroundDaemon(DaemonWork *work) : Daemon(work) { /*nothing to do */ }

	virtual void start()
	{
		work->work();
	}
};

#endif //#ifndef FORGROUND_DAEMON_H
