/*
 * This file is part of Pvlog.
 *
 * Copyright (C) 2017 pvlogdev@gmail.com
 *
 * Pvlog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pvlog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pvlog.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <thread>
#include <chrono>

#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <odb/query.hxx>

#include "datalogger.h"
#include "log.h"
#include "sunrisesunset.h"
#include "timeutil.h"
#include "utility.h"
#include "pvlibhelper.h"

#include "models/config.h"
#include "models/configservice.h"
#include "models/config_odb.h"
#include "models/daydata.h"
#include "models/daydata_odb.h"
#include "models/event.h"
#include "models/event_odb.h"
#include "models/plant.h"
#include "models/plant_odb.h"
#include "models/spotdata.h"
#include "models/spotdata_odb.h"
#include "models/inverter.h"
#include "models/inverter_odb.h"


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

using pvlib::invalid;
using pvlib::isValid;

using namespace pvlib;

namespace bg = boost::gregorian;
namespace pt = boost::posix_time;

namespace bt = boost;

typedef boost::date_time::c_local_adjustor<pt::ptime> local_adj;

namespace {

const pt::ptime ARCHIVE_START = pt::from_iso_string("20000101T000000");

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

static SpotData fillSpotData(const pvlib_ac* ac, const pvlib_dc* dc) {
	SpotData spotData;

	setIfValid(spotData.power, ac->totalPower);
	setIfValid(spotData.frequency, ac->frequency);
	for (int i = 0; i < ac->phaseNum; ++i) {
		if (isValid(ac->power[i])) {
			Phase phase;
			setIfValid(phase.power, ac->power[i]);
			setIfValid(phase.voltage, ac->voltage[i]);
			setIfValid(phase.current, ac->current[i]);

			spotData.phases.emplace(i + 1, phase);
		}
	}

	for (int i = 0; i < dc->trackerNum; ++i) {
		DcInput dcInput;
		setIfValid(dcInput.power, dc->power[i]);
		setIfValid(dcInput.voltage, dc->voltage[i]);
		setIfValid(dcInput.current, dc->current[i]);

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
		LOG(Info) << event.inverter->name << " Set event: "
				<< event.time << "(" << event.number << ")" << event.message;
		db->persist(event);
	}
}

static void saveDayArchiveData(odb::database* db, InverterPtr inv, pvlib_day_yield* dayYields, int num, pt::ptime readTime) {
	odb::transaction t(db->begin());
	for (int i = 0; i < num; ++i) {
		pvlib_day_yield* dy = &dayYields[i];

		//Local time so we can convert it to local date
		pt::ptime time = local_adj::utc_to_local(pt::from_time_t(dy->date - 12 * 3600));
		bg::date date = time.date();

		DayData dayData(inv, date, dy->dayYield);

		LOG(Debug) << "Updating or inserting DayData " << dayData.date << " " << dayData.dayYield;
		updateOrInsert(db, dayData);
	}

	inv->dayArchiveLastRead = readTime;
	db->update(inv);
	t.commit();
}

static void saveEventArchiveData(odb::database* db, InverterPtr inv, pvlib_event* events, int num, pt::ptime readTime) {
	odb::transaction t(db->begin());
	for (int i = 0; i < num; ++i) {
		const pvlib_event& e = events[i];
		Event event(inv, pt::from_time_t(e.time), e.value, e.message);
		updateOrInsert(db, event);
	}

	inv->eventArchiveLastRead = readTime;
	db->update(inv);
	t.commit();
}

static std::unordered_map<int64_t, SpotData> averageSpotData(const std::unordered_map<int64_t, std::vector<SpotData>>& curSpotDataList,
		pt::time_duration timeout) {
	std::unordered_map<int64_t, SpotData> spotDatas;
	for (const auto& entry : curSpotDataList) {
		try {
			SpotData averagedSpotData = average(entry.second);
			averagedSpotData.time = util::roundUp(pt::second_clock::universal_time(), timeout);
			spotDatas.emplace(averagedSpotData.inverter->id, averagedSpotData);
		} catch (const PvlogException& e) {
			LOG(Error) << "Error averaging spot data: " << e.what() ;
		}
	}

	return spotDatas;
}

void Datalogger::addDayYieldData(pvlib_plant* plant, InverterPtr inverter,
		/* out */SpotData& spotData) const {
	int64_t inverterId = inverter->id;
	std::unique_ptr<pvlib_stats, decltype(pvlib_free_stats)*> stats(pvlib_alloc_stats(), pvlib_free_stats);
	if (pvlib_get_stats(plant, inverterId, stats.get()) < 0) {
		std::string errorMsg = bt::str(bt::format("Failed getting statistics of inverter %1%")
				% inverter->name);
		LOG(Error) << errorMsg;
		errorSig(errorMsg);
		return;
	}

	if (isValid(stats->dayYield)) {
			spotData.dayYield = stats->dayYield;
	} else {
		LOG(Warning) << "Got invalid day Yield";
	}
}

void Datalogger::addDayYieldData(const Datalogger::Plants& plants,
		/* out */std::unordered_map<int64_t, SpotData>& spotDatas) const {
	for (auto plantEntry : plants) {
		pvlib_plant* plant  = plantEntry.first;
		Inverters inverters = plantEntry.second;
		for (int64_t inverterId : inverters) {
			InverterPtr inverter = idInverterMapp.at(inverterId);
			auto it = spotDatas.find(inverterId);
			if (it == spotDatas.end()) {
				LOG(Error) << "This should never happen";
				continue;
			}

			addDayYieldData(plant, inverter, it->second);
		}
	}
}

Datalogger::Datalogger(odb::core::database* database) :
		quit(false), active(false), dataloggerStatus(OK), db(database)
{
	PVLOG_NOT_NULL(database);
}

Datalogger::~Datalogger() {
	//nothing to do
}


void Datalogger::openPlant(const Plant& plant) {
	LOG(Info) << "Connecting plant " << plant.name << " ["
			<< plant.connection << ", " << plant.protocol << "]";

	pvlib_plant* pvlibPlant = nullptr;
	try {
		pvlibPlant = connectPlant(plant.connection, plant.protocol, plant.connectionParam, plant.protocolParam);
	} catch (PvlogException& ex) {
		std::string errorMsg = bt::str(bt::format("Error opening plant %1% %2%")
				% plant.name % ex.what());
		LOG(Error) << errorMsg;
		errorSig(errorMsg);
		return; //Ignore plant
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
			std::string errorMsg = bt::str(bt::format("Could not open inverter %1%")
					% inverter->name);
			LOG(Error) << errorMsg;
			errorSig(errorMsg);
		}
	}

	std::unordered_set<int64_t> plantInverterIds;
	for (InverterPtr inverter : plant.inverters) {
		plantInverterIds.insert(inverter->id);
		idInverterMapp.emplace(inverter->id, inverter);
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

void Datalogger::openPlants() {
	std::unordered_map<std::string, uint32_t> connections = getConnections();
	std::unordered_map<std::string, uint32_t> protocols   = getProtocols();

	odb::session s;
	odb::transaction t (db->begin ());
	odb::result<Plant> r  = db->query<Plant>();

	for (odb::result<Plant>::iterator it(r.begin()); it != r.end (); ++it) {
		const Plant& plant = *it;
		openPlant(plant);

	}
	t.commit();
}

void Datalogger::updateDayArchive(pvlib_plant* plant, InverterPtr inverter) {
	pt::ptime lastRead = inverter->dayArchiveLastRead.get_value_or(ARCHIVE_START);
	pt::ptime currentTime = pt::second_clock::universal_time();

	LOG(Info) << "Reading day archive data for "
			<< inverter->name << " " << lastRead << " -> " << currentTime;

	int64_t inverterId = inverter->id;
	pvlib_day_yield* y;
	time_t from = pt::to_time_t(lastRead);
	time_t to   = pt::to_time_t(currentTime);
	int numEntries;
	if ((numEntries = pvlib_get_day_yield(plant, inverterId, from, to, &y)) < 0) {
		std::string errorMsg = bt::str(bt::format("Reading archive event data for %1% from %2% to %3% failed. Error code: %4%")
				% inverter->name % lastRead % currentTime % numEntries);
		LOG(Error) << errorMsg;
		errorSig(errorMsg);
		return;
	}
	LOG(Debug) << "Got " << numEntries << " day yield archive entries";
	if (numEntries == 0) {
		return;
	}
	std::unique_ptr<pvlib_day_yield[], decltype(free)*> dayYields(y, free);

	saveDayArchiveData(db, inverter, dayYields.get(), numEntries, currentTime);

	LOG(Info) << "Read day archive data for "
			<< inverter->name << " " << lastRead << " -> " << currentTime;
}

void Datalogger::updateEventArchive(pvlib_plant* plant, InverterPtr inverter) {
	pt::ptime lastRead = inverter->eventArchiveLastRead.get_value_or(ARCHIVE_START);
	pt::ptime currentTime = pt::second_clock::universal_time();

	LOG(Info) << "Reading event archive data for "
			<< inverter->name << " " << lastRead << " -> " << currentTime;

	//handle events
	pvlib_event* es;
	int64_t inverterId = inverter->id;
	time_t from = pt::to_time_t(lastRead);
	time_t to   = pt::to_time_t(currentTime);
	int numEntries;
	if ((numEntries = pvlib_get_events(plant, inverterId, from, to, &es)) < 0) {
		std::string errorMsg = bt::str(bt::format("Reading archive event data for %1% from %2% to %3% failed. Error code: %4%")
				% inverter->name % lastRead % currentTime % numEntries);
		LOG(Error) << errorMsg;
		errorSig(errorMsg);
		return;
	}
	LOG(Debug) << "Got " << numEntries << " event archive entries";
	if (numEntries == 0) {
		return;
	}
	std::unique_ptr<pvlib_event[], decltype(free)*> events(es, free);

	saveEventArchiveData(db, inverter, events.get(), numEntries, currentTime);

	LOG(Info) << "Read event archive data for "
			<< inverter->name << " " << lastRead << " -> " << currentTime;
}

void Datalogger::updateArchiveData() {
	odb::session session;

	for (auto plantEntry : plants) {
		pvlib_plant* plant = plantEntry.first;
		for (int64_t inverterId : plantEntry.second) {
			odb::transaction t(db->begin());
			InverterPtr inverter(db->load<Inverter>(inverterId));
			t.commit();

			updateDayArchive(plant, inverter);
			updateEventArchive(plant, inverter);
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

void Datalogger::sleepUntill(pt::ptime time) const {
	using system_clock = std::chrono::system_clock;

	time_t unixTime = pt::to_time_t(time);
	system_clock::time_point sleepUntil = system_clock::from_time_t(unixTime);

	std::unique_lock<std::mutex> uniqueLock(mutex);

	userEventSignal.wait_until(uniqueLock, sleepUntil, [this]() -> bool { return quit; });
}

void Datalogger::logDayData(pvlib_plant* plant, int64_t inverterId) {
	std::unique_ptr<pvlib_stats, decltype(pvlib_free_stats)*> stats(pvlib_alloc_stats(), pvlib_free_stats);

	InverterPtr inverter = idInverterMapp.at(inverterId);

	LOG(Debug) << "logging day yield";
	if (pvlib_get_stats(plant, inverterId, stats.get()) < 0) {
		std::string errorMsg = bt::str(bt::format("Failed getting statistics of inverter %1%")
				% inverter->name);
		LOG(Error) << errorMsg;
		errorSig(errorMsg);
		return;
	}

	if (isValid(stats->dayYield)) {
		odb::session session;
		odb::transaction t (db->begin ());
		bg::date curDate(bg::day_clock::local_day());
		DayData dayData(inverter, curDate, stats->dayYield);

		updateOrInsert(db, dayData);
		t.commit();
	} else {
		std::string errorMsg = "Could not read dayYield (Invalid value)!";
		LOG(Error) << errorMsg;
		errorSig(errorMsg);
	}
	LOG(Debug) << "logged day yield";
}

void Datalogger::logSpotData(InverterPtr inverter, const pvlib_ac* ac, const pvlib_dc* dc) {
	// extra function
	SpotData spotData = fillSpotData(ac, dc);
	spotData.inverter = inverter;
	spotData.time = util::roundUp(pt::second_clock::universal_time(), updateInterval);

	curSpotData[inverter->id] = spotData;

	LOG(Trace) << "Spot data: " << spotData;

	curSpotDataList[inverter->id].push_back(spotData);
}

void Datalogger::logData(pvlib_plant* plant, int64_t inverterId) {
	int ret;
	std::unique_ptr<pvlib_ac, decltype(pvlib_free_ac)*> ac(pvlib_alloc_ac(), pvlib_free_ac);
	std::unique_ptr<pvlib_dc, decltype(pvlib_free_dc)*> dc(pvlib_alloc_dc(), pvlib_free_dc);
	std::unique_ptr<pvlib_status, decltype(pvlib_free_status)*> status(pvlib_alloc_status(), pvlib_free_status);

	InverterPtr inverter = idInverterMapp.at(inverterId);

	if ((ret = pvlib_get_ac_values(plant, inverterId, ac.get())) < 0 ||
		(ret = pvlib_get_dc_values(plant, inverterId, dc.get())) < 0 ||
		(ret = pvlib_get_status(plant, inverterId, status.get())) < 0) {
		std::string errorMsg = bt::str(bt::format("Error reading inverter %1% data. Error: %2%")
				% inverter->name % ret);
		LOG(Error) << errorMsg;
		errorSig(errorMsg);
		return;
	}

	if (status->status != PVLIB_STATUS_OK) {
		std::string errorMsg = bt::str(bt::format("Status of %1% not OK but %2%")
				% inverter->name % status->status);
		LOG(Error) << errorMsg;
		errorSig(errorMsg);
	}

	if (!isValid(ac->totalPower) || ac->totalPower == 0) {
		//handle invalid data
		pt::ptime curTime = pt::second_clock::universal_time();
		pt::time_duration diffSunset  = sunset - curTime;
		pt::time_duration diffSunrise = curTime - sunrise;

		if (diffSunset >= pt::hours(1) && diffSunrise >= pt::hours(1)) {
			std::string errorMsg = bt::str(bt::format("Total power of %1% 0 during the day!")
					% inverter->name);
			LOG(Error) << errorMsg;
			errorSig(errorMsg);
			return;
		} else if (diffSunset <= pt::hours(0)) {
			//sunset and 0 power so we can log day data
			logDayData(plant, inverterId);

			//close inverter
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
			return;
		} else {
			//wait till day end or day start
			return;
		}
	}

	logSpotData(inverter, ac.get(), dc.get());
}

void Datalogger::logData() {
	Plants plantsCopy(plants); //Copy plants: so we can delete plant from original plant

	//log spot data
	for (auto plantEntry : plantsCopy) {
		pvlib_plant* plant  = plantEntry.first;
		Inverters inverters = plantEntry.second;
		for (int64_t inverterId : inverters) {
			logData(plant, inverterId);
		}
	}

	//Average data from last interval and store it in database
	pt::ptime time = util::roundDown(pt::second_clock::universal_time(), updateInterval);
	if (pt::to_time_t(time) % timeout.total_seconds() == 0) {
		std::unordered_map<int64_t, SpotData> spotDatas = averageSpotData(curSpotDataList, timeout);

		//read current dayyield data
		addDayYieldData(plants, spotDatas);

		//persist spot data
		std::vector<SpotData> spotDataVec;
		spotDataVec.reserve(spotDatas.size());
		for (const auto& entry : spotDatas) {
			spotDataVec.push_back(entry.second);
		}
		for (SpotData& sd : spotDataVec) {
			LOG(Info) << "Persisting spot data: " << sd;
			odb::transaction t (db->begin ());
			db->persist(sd);
			t.commit();
		}

		spotDataSig(spotDataVec);
		curSpotDataList.clear();
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

Datalogger::Status Datalogger::getStatus() {
	std::lock_guard<std::mutex> lock(mutex);
	if (dataloggerStatus == OK && !active) {
		return PAUSED;
	} else {
		return dataloggerStatus;
	}
}

void Datalogger::work() {
	for (;;) {
		timeout = pt::seconds(std::stoi(readConfig(db, "timeout")));
		LOG(Info) << "Timeout: " << timeout;

		if (timeout.total_seconds() < 60) {
			LOG(Error) << "Timeout must be at least 60 seconds!";
			stop();
		}
		if ((timeout.seconds()) != 0) {
			LOG(Error) << "Timeout must be a multiple of 60 seconds";
			stop();
		}

		float longitude = std::stof(readConfig(db, "longitude"));
		float latitude = std::stof(readConfig(db, "latitude"));
		LOG(Info) << "Location longitude " << longitude << " latitude: " << latitude;


		sunriseSunsetCalculator = std::unique_ptr<SunriseSunset>(
				new SunriseSunset(longitude, latitude));
		int julianDay = bg::day_clock::universal_day().julian_day();
		sunset  = sunriseSunsetCalculator->sunset(julianDay);
		sunrise = sunriseSunsetCalculator->sunrise(julianDay);

		this->updateInterval = pt::seconds(20);


		active = true;
		dataloggerStatus = OK;
		openPlants();
		if (plants.empty()) {
			quit = true;
		}

		if (!quit) {
			updateArchiveData();
			logger();
		}

		if (quit) {
			LOG(Info) << "Pausing datalogger";
			closePlants();


			active = false;

			std::unique_lock<std::mutex> lock(mutex);
			userEventSignal.wait(lock, [this]() { return !quit; });
			LOG(Info) << "Continuing datalogger";
		}
	}
}

void Datalogger::logger()
{
	try {
		while (!quit) {
			if (plants.empty()) {
				//no more plants are open => wait for next day

				int nextJulianDay = bg::day_clock::universal_day().julian_day() + 1;
				sunrise = sunriseSunsetCalculator->sunrise(nextJulianDay);
				sunset  = sunriseSunsetCalculator->sunset(nextJulianDay);

				dayEndSig();
				dataloggerStatus = NIGHT;

				LOG(Info) << "Waiting for next days sunrise: " << sunrise;
				sleepUntill(sunrise);

				if (quit) return;

				dataloggerStatus = OK;
				openPlants();

				updateArchiveData();
			}

			pt::ptime curTime = pt::second_clock::universal_time();
			pt::ptime nextUpdate = util::roundUp(curTime, timeout);

			LOG(Debug) << "Sunset: " << pt::to_simple_string(sunset);


			LOG(Debug) << "current time: " << pt::to_simple_string(curTime);
			LOG(Debug) << "time till wait: " << pt::to_simple_string(nextUpdate);

			nextUpdate = util::roundUp(curTime, updateInterval);
			sleepUntill(nextUpdate);
			if (quit) return;

			logData();
		}
	} catch (const PvlogException& ex) {
		dataloggerStatus = ERROR;
		LOG(Error) << ex.what();
		errorSig(std::string("Got pvlogexception exception: ") + ex.what());
	}
}
