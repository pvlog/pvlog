#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <ctime>
#include <memory>

#include <odb/database.hxx>

#include "Daemon.h"
#include "DateTime.h"
#include "Pvlib.h"

class SunriseSunset;
class Database;

namespace model {
	class Inverter;
}

class DataLogger: public DaemonWork {
public:
	DataLogger(odb::core::database* database, pvlib::Pvlib* pvlib, int timeout);

	virtual ~DataLogger() = default;

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
	bool waitForLogTime(DateTime timeToWait);

	void logDayData();

	void logData();
private:
	void openPlants();

	void closePlants();

	volatile bool quit;
	odb::core::database* db;
	pvlib::Pvlib* pvlib;
	int timeout;
	std::unique_ptr<SunriseSunset> sunriseSunset;
	std::unordered_map<int64_t, std::shared_ptr<model::Inverter>> openInverter;
};

#endif // #ifndef DATA_LOGGER_H
