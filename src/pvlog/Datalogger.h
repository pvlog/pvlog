#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <ctime>
#include <memory>

#include "Daemon.h"

class SunriseSunset;
class Database;
class Pvlib;

class DataLogger: public DaemonWork {
public:
	DataLogger(Database* database, Pvlib* pvlib, int timeout);

	virtual void work();

protected:
	/**
	 * Wait for begin of daylight.
	 *
	 * @return false if interrupted else true.
	 */
	bool waitForDay();

	/**
	 * Wait for timeout
	 */
	bool waitForLogTime(int timeout);

	void logDayData();

	void logData();
private:
	volatile bool quit;
	Database* database;
	Pvlib* pvlib;
	int timeout;
	std::unique_ptr<SunriseSunset> sunriseSunset;
};

#endif // #ifndef DATA_LOGGER_H
