#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <ctime>
#include <memory>
#include <unordered_map>
#include <vector>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <odb/database.hxx>

#include "Daemon.h"
#include "DateTime.h"
#include "Pvlib.h"
#include "models/SpotData.h"

class SunriseSunset;
class Database;

namespace model {
	class Inverter;
}


class DataLogger: public DaemonWork {
public:
	DataLogger(odb::core::database* database, pvlib::Pvlib* pvlib);

	virtual ~DataLogger();

	virtual void work();

protected:
//	/**
//	 * Wait for begin of daylight.
//	 *
//	 * @return false if interrupted else true.
//	 */
//	bool waitForDay();
//
//	/**
//	 * Wait for timeout
//	 */
//	bool waitForLogTime(DateTime timeToWait);

	void logDayData();

	void logData();

	void sleepUntill(boost::posix_time::ptime time);
private:
	void openPlants();

	void closePlants();

	volatile bool quit;
	odb::core::database* db;
	pvlib::Pvlib* pvlib;
	boost::posix_time::time_duration timeout;
	boost::posix_time::time_duration updateInterval;
	std::unique_ptr<SunriseSunset> sunriseSunset;
	std::unordered_map<int64_t, std::shared_ptr<model::Inverter>> openInverter;
	std::unordered_map<int64_t, std::vector<model::SpotData>> curSpotData;
};

#endif // #ifndef DATA_LOGGER_H
