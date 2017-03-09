#include <thread>
#include <chrono>

#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include "boost/date_time/local_time_adjustor.hpp"
#include "boost/date_time/c_local_time_adjustor.hpp"
#include <boost/optional.hpp>
#include <config.h>
#include <configservice.h>
#include <datalogger.h>
#include <daydata.h>
#include <event.h>
#include <log.h>
#include <log.h>
#include <odb/query.hxx>
#include <plant.h>
#include <pvlib.h>
#include <pvlib.h>
#include <spotdata.h>
#include <sunrisesunset.h>
#include <timeutil.h>
#include <utility.h>

#include "pvlibhelper.h"
#include "daydata_odb.h"
#include "config_odb.h"
#include "plant_odb.h"
#include "inverter_odb.h"
#include "spotdata_odb.h"
#include "event_odb.h"

using model::Config;
using model::ConfigPtr;
using model::Plant;
using model::SpotData;
using model::Phase;
using model::DcInput;
using model::Inverter;
using model::InverterPtr;
using model::DayData;
using model::Event;

using std::unique_ptr;
using std::shared_ptr;
using std::mutex;
using std::unique_lock;

//using pvlib::Pvlib;
using pvlib::Ac;
using pvlib::Dc;
using pvlib::Status;
using pvlib::Stats;
using pvlib::invalid;
using pvlib::isValid;

using namespace pvlib;

namespace bg = boost::gregorian;
namespace pt = boost::posix_time;

typedef boost::date_time::c_local_adjustor<pt::ptime> local_adj;

namespace {

//helper functions to test values for validity
template<typename T>
void setIfValid(T& s, T t) {
	if (isValid(t)) {
		s = t;
	} else {
		PVLOG_EXCEPT("Invalid value!");
	}
}

template<typename T>
void setIfValid(boost::optional<T>& s, T t) {
	if (isValid(t)) {
		s = t;
	}
}

template<typename T>
void sumUp(boost::optional<T>& sumed, const boost::optional<T>& val) {
	if (sumed && val) {
		sumed = sumed.get() + val.get();
	} else {
		PVLOG_EXCEPT("Invalid spot data!");
	}
}

template<typename T>
void divideIfValid(boost::optional<T>& val, int divider) {
	if (val) {
		val = val.get() / divider;
	}
}

//Average over spotData
SpotData average(const std::vector<SpotData>& spotData) {
	if (spotData.empty()) {
		PVLOG_EXCEPT("Can not average spotData vector of length 0");
	}

	SpotData sum = spotData.at(0);

	for (auto it = ++spotData.begin(); it != spotData.end(); ++it) {
		if (sum.phases.size() != it->phases.size()
				|| (sum.dcInputs.size() != it->dcInputs.size())) {
			PVLOG_EXCEPT("Invalid spot data!");
		}

		sum.power += it->power;

		sumUp(sum.frequency, it->frequency);

		for (const auto& entry : it->dcInputs) {
			int dcLine = entry.first;
			const DcInput& dcInput = entry.second;

			auto valueIt = sum.dcInputs.find(dcLine);
			if (valueIt == sum.dcInputs.end()) {
				PVLOG_EXCEPT("Invalid data!");
			}

			sumUp(valueIt->second.power, dcInput.power);
			sumUp(valueIt->second.voltage, dcInput.voltage);
			sumUp(valueIt->second.current, dcInput.current);
		}

		for (const auto& entry : it->phases) {
			int numPhase = entry.first;
			const Phase& phase = entry.second;

			auto valueIt = sum.phases.find(numPhase);
			if (valueIt == sum.phases.end()) {
				PVLOG_EXCEPT("Invalid data!");
			}

			valueIt->second.power += phase.power;
			sumUp(valueIt->second.voltage, phase.voltage);
			sumUp(valueIt->second.current, phase.current);
		}
	}

	int num = spotData.size();

	sum.power /= spotData.size();
	divideIfValid(sum.frequency, num);

	for (auto& entry : sum.phases) {
		Phase& p = entry.second;
		p.power = p.power / num;
		divideIfValid(p.voltage, num);
		divideIfValid(p.current, num);
	}

	for (auto& entry : sum.dcInputs) {
		DcInput& p = entry.second;
		divideIfValid(p.power, num);
		divideIfValid(p.voltage, num);
		divideIfValid(p.current, num);
	}

	return sum;
}

} //namespace {

static SpotData fillSpotData(const Ac& ac, const Dc& dc) {
	SpotData spotData;

	setIfValid(spotData.power, ac.totalPower);
	setIfValid(spotData.frequency, ac.frequency);
	for (int i = 0; i < ac.phaseNum; ++i) {
		if (isValid(ac.power[i])) {
			Phase phase;
			setIfValid(phase.power, ac.power[i]);
			setIfValid(phase.voltage, ac.voltage[i]);
			setIfValid(phase.current, ac.current[i]);

			spotData.phases.emplace(i + 1, phase);
		}
	}

	for (int i = 0; i < dc.trackerNum; ++i) {
		DcInput dcInput;
		setIfValid(dcInput.power, dc.power[i]);
		setIfValid(dcInput.voltage, dc.voltage[i]);
		setIfValid(dcInput.current, dc.current[i]);

		spotData.dcInputs.emplace(i + 1, dcInput);
	}

	return spotData;
}

static void updateOrInsert(odb::database* db, DayData& dayData) {
	using Query  = odb::query<DayData>;

	Query query((Query::date == Query::_ref(dayData.date))
			&& (Query::inverter == Query::_ref(dayData.inverter->id)));

	std::shared_ptr<DayData> res(db->query_one<DayData>(query));
	if (res != nullptr) {
		res->dayYield = dayData.dayYield;
		LOG(Info) << dayData.inverter->name << " Updated day yield "
				<< res->dayYield << " -> " << dayData.dayYield;
		db->update(*res);
	} else {
		LOG(Info) << dayData.inverter->name << " Set day yield: " << dayData.dayYield;
		db->persist(dayData);
	}
}

static void updateOrInsert(odb::database* db, Event& event) {
	using Query  = odb::query<Event>;

	Query query((Query::time == Query::_ref(event.time))
			&& (Query::inverter == Query::_ref(event.inverter->id)));

	std::shared_ptr<Event> res(db->query_one<Event>(query));
	if (res != nullptr) {
		//nothing to do
	} else {
		LOG(Info) << event.inverter->name << " Set event: " << event.time << "(" << event.number << ")" << event.message;
		db->persist(event);
	}
}


Datalogger::Datalogger(odb::core::database* database) :
		quit(false), db(database)
{
	PVLOG_NOT_NULL(database);

	timeout = pt::seconds(std::stoi(readConfig(db, "timeout")));
	LOG(Info) << "Timeout: " << timeout;

	if (timeout.total_seconds() < 60) {
		PVLOG_EXCEPT("Timeout must be at least 60 seconds!");
	}
	if ((timeout.seconds()) != 0) {
		PVLOG_EXCEPT("Timeout must be a multiple of 60 seconds");
	}

	float longitude = std::stof(readConfig(db, "longitude"));
	float latitude = std::stof(readConfig(db, "latitude"));

	LOG(Info) << "Location longitude " << longitude << " latitude: " << latitude;


	sunriseSunset = std::unique_ptr<SunriseSunset>(
			new SunriseSunset(longitude, latitude));

	int julianDay = bg::day_clock::universal_day().julian_day();
	sunset  = sunriseSunset->sunset(julianDay);
	sunrise = sunriseSunset->sunrise(julianDay);

	openPlants();

	updateArchiveData();

	this->updateInterval = pt::seconds(20);
}

Datalogger::~Datalogger() {
	//nothing to do
}

void Datalogger::openPlants() {
	std::unordered_map<std::string, uint32_t> connections = getConnections();
	std::unordered_map<std::string, uint32_t> protocols   = getProtocols();

	odb::session s;
	odb::transaction t (db->begin ());
	odb::result<Plant> r  = db->query<Plant>();

	for (odb::result<Plant>::iterator it(r.begin()); it != r.end (); ++it) {
		const Plant& plant = *it;

		LOG(Info) << "Connecting plant " << plant.name << " ["
				<< plant.connection << ", " << plant.protocol << "]";

		pvlib_plant* pvlibPlant = nullptr;
		try {
			pvlibPlant = connectPlant(plant.connection, plant.protocol, plant.connectionParam, plant.protocolParam);
		} catch (PvlogException& ex) {
			LOG(Error) << "Error opening plant: " << ex.what();
			continue; //ignore plant
		}

		LOG(Info) << "Successfully connected plant " << plant.name << " ["
				<< plant.connectionParam << ", " << plant.protocolParam << "]";

		Inverters availableInverterIds = getInverters(pvlibPlant);
		LOG(Info) << "Available inverters: ";
		for (int64_t id : availableInverterIds) {
			LOG(Info) << id;
		}

		for (const auto& inverter : plant.inverters) {
			if (availableInverterIds.count(inverter->id) != 1) {
				LOG(Error) << "Could not open inverter: " << inverter->name;
			}
		}

		std::unordered_set<int64_t> plantInverterIds;
		for (InverterPtr inverter : plant.inverters) {
			plantInverterIds.insert(inverter->id);
			inverterInfo.emplace(inverter->id, inverter);
		}

		for (auto inverterIt = availableInverterIds.begin(); inverterIt != availableInverterIds.end(); ) {
			if (plantInverterIds.count(*inverterIt) == 0) {
				LOG(Warning) << "Found inverter not in database. Id: " << *inverterIt;
				inverterIt = availableInverterIds.erase(inverterIt);
			} else {
				++inverterIt;
			}
		}

		plants.emplace(pvlibPlant, availableInverterIds);

		LOG(Info) << "Opened plant " << plant.name << " ["
				<< plant.connection << ", " << plant.protocol << "]";
	}
	t.commit();
}

void Datalogger::updateArchiveData() {
	odb::session session;

	for (auto plantEntry : plants) {
		pvlib_plant* plant = plantEntry.first;
		for (int64_t inverterId : plantEntry.second) {
			odb::transaction t(db->begin());
			InverterPtr inverter(db->load<Inverter>(inverterId));

			pt::ptime lastRead = inverter->archiveLastRead.get_value_or(pt::from_iso_string("20000101T000000"));
			pt::ptime currentTime = pt::second_clock::universal_time();

			LOG(Info) << "Reading archive data for " << inverter->name << " " << lastRead << " -> " << currentTime;

			pvlib_day_yield* y;
			int ret;
			if ((ret = pvlib_get_day_yield(plant, inverterId, pt::to_time_t(lastRead),
					pt::to_time_t(currentTime), &y)) < 0) {
				LOG(Error) << "Reading archive day data for " << inverter->name << " Error code: " << ret;
				continue; //Ignore inverter
			}
			std::unique_ptr<pvlib_day_yield[], decltype(free)*> dayYields(y, free);

			LOG(Debug) << "Got " << ret << " day yield archive entries";

			for (int i = 0; i < ret; ++i) {
				pvlib_day_yield* dy = &dayYields[i];

				//Local time so we can convert it to local date
				pt::ptime time = local_adj::utc_to_local(pt::from_time_t(dy->date - 12 * 3600));
				bg::date date = time.date();

				DayData dayData(inverter, date, dy->dayYield);

				LOG(Debug) << "Updating or inserting DayData " << dayData.date << " " << dayData.dayYield;
				updateOrInsert(db, dayData);
			}

			//handle events
			pvlib_event* es;

			if ((ret = pvlib_get_events(plant, inverterId, pt::to_time_t(lastRead),
					pt::to_time_t(currentTime), &es)) < 0) {
				LOG(Error) << "Error reading archive event data for " << inverter->name << " "
						<< lastRead << " -> " << currentTime;
				return;
			}
			std::unique_ptr<pvlib_event[], decltype(free)*> events(es, free);
			LOG(Debug) << "Got " << ret << " event archive entries";

			for (int i = 0; i < ret; ++i) {
				const pvlib_event& e = events[i];
				Event event(inverter, pt::from_time_t(e.time), e.value, e.message);
				updateOrInsert(db, event);
			}

			inverter->archiveLastRead = currentTime;
			db->update(inverter);
			t.commit();

			LOG(Info) << "Read archive data for " << inverter->name << " " << lastRead << " -> " << currentTime;
		}
	}
}

void Datalogger::closePlants() {
	for (auto plantEntry : plants) {
		pvlib_plant* p = plantEntry.first;
		pvlib_close(p);
	}

	plants.clear();
}

//bool DataLogger::waitForDay()
//{
//	/*	if (sunriseSunset->isDay(DateTime().julianDay())) {
//	 return true;
//	 }
//	 */
//	LOG(Debug) << "Waiting for day begin...";
//	DateTime time = sunriseSunset->sunrise(DateTime().julianDay() + 1);
//	return DateTime::sleepUntil(time);
//}
//
//bool DataLogger::waitForLogTime(DateTime timeToWait)
//{
//	return DateTime::sleepUntil(timeToWait);
//}

void Datalogger::sleepUntill(pt::ptime time) {
	using system_clock = std::chrono::system_clock;

	time_t unixTime = pt::to_time_t(time);
	system_clock::time_point sleepUntil = system_clock::from_time_t(unixTime);

	std::unique_lock<std::mutex> uniqueLock(mutex);

	userEventSignal.wait_until(uniqueLock, sleepUntil, [this](){ return quit; });
}

void Datalogger::logDayData(pvlib_plant* plant, int64_t inverterId) {
	Stats stats;
	InverterPtr inverter = inverterInfo.at(inverterId);

	LOG(Debug) << "logging day yield";
	if (pvlib_get_stats(plant, inverterId, &stats) < 0) {
		LOG(Error) << "Failed getting statistics of inverter: " << inverter->name;
		return;
	}

	if (isValid(stats.dayYield)) {
		odb::session session;
		odb::transaction t (db->begin ());
		bg::date curDate(bg::day_clock::local_day());
		DayData dayData(inverter, curDate, stats.dayYield);

		updateOrInsert(db, dayData);
		t.commit();
	} else {
		LOG(Error) << "Could not read dayYield (Invalid value!)";
	}
	LOG(Debug) << "logged day yield";
}

void Datalogger::logData() {
	Plants plantsCopy(plants); //Copy plants: so wen can delete plant from original plant

	for (auto plantEntry : plantsCopy) {
		pvlib_plant* plant  = plantEntry.first;
		Inverters inverters = plantEntry.second;
		for (int64_t inverterId : inverters) {
			InverterPtr inverter = inverterInfo.at(inverterId);

			int ret;
			Ac ac;
			Dc dc;
			Status status;

			if ((ret = pvlib_get_ac_values(plant, inverterId, &ac)) < 0 ||
				(ret = pvlib_get_dc_values(plant, inverterId, &dc)) < 0 ||
				(ret = pvlib_get_status(plant, inverterId, &status)) < 0) {
				LOG(Error) << "Error reading inverter data. Error: " << ret;
			}

			pt::ptime curTime = pt::second_clock::universal_time();

			if (status.status != PVLIB_STATUS_OK) {
				//TODO: handle error: for know just ignore it!!!
				LOG(Error) << "Status of " << inverter->name << "not OK but " << status.status;
			}

			if (!isValid(ac.totalPower) || ac.totalPower == 0) {
				pt::time_duration diffSunset  = sunset - curTime;
				pt::time_duration diffSunrise = curTime - sunrise;

				if (diffSunset >= pt::hours(1) && diffSunrise >= pt::hours(1)) {
					//TODO: handle invalid power: for now just ignore it!!!
					LOG(Error) << "Total power of " << inverter->name << " 0";

					continue;
				} else if (diffSunset <= pt::hours(0)) {
					//sunset and 0 power so we can log day data
					logDayData(plant, inverterId);

					//close inverter for this day
					Inverters& openInverters = plants.at(plant);
					openInverters.erase(inverterId);
					curSpotData.erase(inverterId);

					LOG(Info) << "Closed inverter: " << inverter->name;
					if (openInverters.empty()) {
						LOG(Info) << "Closing plant!";
						pvlib_close(plant);
						plants.erase(plant);
					}
					continue;
				} else {
					//wait till day end or day start
					continue;
				}
			}

			SpotData spotData = fillSpotData(ac, dc);
			spotData.inverter = inverter;
			spotData.time = util::roundUp(pt::second_clock::universal_time(), updateInterval);

			curSpotData[inverterId] = spotData;

			LOG(Trace) << "Spot data: " << spotData;

			curSpotDataList[inverterId].push_back(spotData);

			if (pt::to_time_t(spotData.time) % timeout.total_seconds() == 0) {
				LOG(Debug) << "logging current power, voltage, ...";
				for (const auto& entry : curSpotDataList) {
					try {
						SpotData averagedSpotData = average(entry.second);
						averagedSpotData.time = util::roundUp(pt::second_clock::universal_time(), timeout);

						LOG(Debug) << "Persisting spot data: " << averagedSpotData;
						odb::transaction t (db->begin ());
						db->persist(averagedSpotData);
						t.commit();
					} catch (const PvlogException& e) {
						LOG(Error) << "Error averaging spot data: " << e.what() ;
					}
				}
				curSpotDataList.clear();
			}
		}
	}
}

const std::unordered_map<int64_t, model::SpotData>& Datalogger::getLiveData() const {
	return curSpotData;
}

void Datalogger::stop() {
	std::unique_lock<std::mutex> lock(mutex);
	if (active == false) {
		return;
	}
	quit = true;
	lock.unlock();

	userEventSignal.notify_one();
}

void Datalogger::start() {
	std::unique_lock<std::mutex> lock(mutex);
	if (active) {
		return;
	}
	quit = false;
	lock.unlock();

	userEventSignal.notify_one();
}

bool Datalogger::isRunning() {
	std::lock_guard<std::mutex> lock(mutex);
	return active;
}

void Datalogger::work() {
	for (;;) {
		active = true;
		logger();

		if (!quit) {
			continue;
		}

		active = false;

		std::unique_lock<std::mutex> lock(mutex);
		userEventSignal.wait(lock, [this]() { return !quit; });
	}
}

void Datalogger::logger()
{
	try {
		while (!quit) {
			pt::ptime curTime = pt::second_clock::universal_time();
			pt::ptime nextUpdate = util::roundUp(curTime, timeout);

			LOG(Debug) << "Sunset: " << pt::to_simple_string(sunset);

			if (plants.empty()) {
				//no more plants are open => wait for next day

				int nextJulianDay = bg::day_clock::universal_day().julian_day() + 1;
				sunrise = sunriseSunset->sunrise(nextJulianDay);
				sunset  = sunriseSunset->sunset(nextJulianDay);

				LOG(Info) << "Waiting for next days sunrise: " << sunrise;
				sleepUntill(sunrise);

				if (quit) return;

				openPlants();

				updateArchiveData();
			}

			LOG(Debug) << "current time: " << pt::to_simple_string(curTime);
			LOG(Debug) << "time till wait: " << pt::to_simple_string(nextUpdate);

			nextUpdate = util::roundUp(curTime, updateInterval);
			sleepUntill(nextUpdate);
			if (quit) return;

			logData();
		}
	} catch (const PvlogException& ex) {
		LOG(Error) << ex.what();
	}
}