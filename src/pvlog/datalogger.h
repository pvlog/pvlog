#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <atomic>
#include <ctime>
#include <memory>
#include <unordered_map>
#include <vector>
#include <condition_variable>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/signals2.hpp>
#include <odb/database.hxx>
#include <pvlib/pvlib.h>

#include "pvlibhelper.h"

#include "models/spotdata.h"

class SunriseSunset;
class Database;
struct pvlib_plant;

namespace model {
	class Inverter;
}


//Logs one day of inverter data
class Datalogger {
public:
	boost::signals2::signal<void (std::string)> errorSig;
	boost::signals2::signal<void ()> dayEndSig;

	enum Status {
		OK = 0,
		NIGHT,
		WARNING,
		ERROR,
		PAUSED
	};


	Datalogger(odb::core::database* database);

	virtual ~Datalogger();

	virtual void work();

	const std::unordered_map<int64_t, model::SpotData>& getLiveData() const;

	void stop();

	void start();

	bool isRunning();

	Status getStatus();
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

	void logDayData(pvlib_plant* plant, int64_t inverterId);

	void logData();

	void sleepUntill(boost::posix_time::ptime time);

	void logger();
private:
	void logData(pvlib_plant* plant, int64_t inverterId);

	void openPlant(const model::Plant& plant);

	void openPlants();

	void closePlants();

	void updateDayArchive(pvlib_plant* plant, model::InverterPtr inverter);

	void updateEventArchive(pvlib_plant* plant, model::InverterPtr inverter);

	void updateArchiveData();

	std::atomic<bool> quit;
	bool active;
	std::condition_variable userEventSignal;
	std::mutex mutex;

	Status dataloggerStatus;



	odb::core::database* db;
	//pvlib::Pvlib* pvlib;
	boost::posix_time::time_duration timeout;
	boost::posix_time::time_duration updateInterval;
	std::unique_ptr<SunriseSunset> sunriseSunset;
	boost::posix_time::ptime sunset;
	boost::posix_time::ptime sunrise;

	using Inverters = std::unordered_set<int64_t>;
	using Plants    = std::unordered_map<pvlib_plant*, Inverters>;

	Plants plants;
	std::unordered_map<int64_t, std::shared_ptr<model::Inverter>> inverterInfo;
	std::unordered_map<int64_t, std::vector<model::SpotData>> curSpotDataList;

	std::unordered_map<int64_t, model::SpotData> curSpotData;
};

#endif // #ifndef DATA_LOGGER_H
