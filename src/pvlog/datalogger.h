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
	class SpotData;
	class DayData;
}


//Logs one day of inverter data
class Datalogger {
public:
	boost::signals2::signal<void (std::string)> errorSig;
	boost::signals2::signal<void ()> dayEndSig;
	boost::signals2::signal<void (const std::vector<model::SpotData>&)> spotDataSig;
	boost::signals2::signal<void (const std::vector<model::DayData>&)> dayDataSig;

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
	using Inverters = std::unordered_set<int64_t>;
	using Plants    = std::unordered_map<pvlib_plant*, Inverters>;

	void logDayData(pvlib_plant* plant, int64_t inverterId);

	void logData();

	void sleepUntill(boost::posix_time::ptime time) const;

	void logger();
private:
	void logSpotData(model::InverterPtr inverter, const pvlib_ac* ac, const pvlib_dc* dc);

	void logData(pvlib_plant* plant, int64_t inverterId);

	void openPlant(const model::Plant& plant);

	void openPlants();

	void closePlants();

	void updateDayArchive(pvlib_plant* plant, model::InverterPtr inverter);

	void updateEventArchive(pvlib_plant* plant, model::InverterPtr inverter);

	void updateArchiveData();

	void addDayYieldData(pvlib_plant* plant, model::InverterPtr inverter,
			model::SpotData& spotData) const;

	void addDayYieldData(const Plants& plants,
			/* out */std::unordered_map<int64_t, model::SpotData>& spotDatas) const;

	std::atomic<bool> quit;
	bool active;
	mutable std::condition_variable userEventSignal;
	mutable std::mutex mutex;

	Status dataloggerStatus;

	odb::core::database* db;
	boost::posix_time::time_duration timeout;
	boost::posix_time::time_duration updateInterval;
	std::unique_ptr<SunriseSunset> sunriseSunsetCalculator;
	boost::posix_time::ptime sunset;
	boost::posix_time::ptime sunrise;

	Plants plants;
	std::unordered_map<int64_t, model::InverterPtr> idInverterMapp;

	std::unordered_map<int64_t, std::vector<model::SpotData>> curSpotDataList;
	std::unordered_map<int64_t, model::SpotData> curSpotData;
};

#endif // #ifndef DATA_LOGGER_H
