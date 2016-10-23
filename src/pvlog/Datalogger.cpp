#include "Datalogger.h"

#include <thread>
#include <chrono>

#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/optional.hpp>

#include <odb/query.hxx>

#include "Log.h"
#include "SunriseSunset.h"
#include "Pvlib.h"
#include "Utility.h"
#include "TimeUtil.h"
#include "Log.h"
#include "models/Config.h"
#include "models/ConfigService.h"
#include "models/Plant.h"
#include "models/SpotData.h"
#include "models/DayData.h"
#include "DayData_odb.h"
#include "Config_odb.h"
#include "Plant_odb.h"
#include "Inverter_odb.h"
#include "SpotData_odb.h"

using model::Config;
using model::Plant;
using model::SpotData;
using model::Phase;
using model::DcInput;
using model::Inverter;
using model::InverterPtr;
using model::DayData;

using std::unique_ptr;
using std::shared_ptr;
using pvlib::Pvlib;
using pvlib::Ac;
using pvlib::Dc;
using pvlib::Status;
using pvlib::Stats;
using pvlib::invalid;
using pvlib::isValid;

using namespace pvlib;

namespace bg = boost::gregorian;
namespace pt = boost::posix_time;

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
	if (!sumed && !val) {
		sumed = sumed.get() + val.get();

	} else if (!sumed || !val) {
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

			auto valueIt = sum.dcInputs.find(numPhase);
			if (valueIt == sum.dcInputs.end()) {
				PVLOG_EXCEPT("Invalid data!");
			}

			valueIt->second.power = phase.power;
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
		db->update(*res);
	} else {
		db->persist(dayData);
	}
}


DataLogger::DataLogger(odb::core::database* database, Pvlib* pvlib) :
		quit(false), db(database), pvlib(pvlib)
{
	PVLOG_NOT_NULL(database);
	PVLOG_NOT_NULL(pvlib);

	timeout = pt::seconds(std::stoi(readConfig(db, "timeout")));
	LOG(Info) << "Timeout: " << timeout;

	if (timeout.seconds() < 60) {
		PVLOG_EXCEPT("Timeout must be at least 60 seconds!");
	}
	if ((timeout.seconds() % 60) != 0) {
		PVLOG_EXCEPT("Timeout must be a multiple of 60 seconds");
	}

	float longitude = std::stof(readConfig(db, "longitude"));
	float latitude = std::stof(readConfig(db, "latitude"));

	LOG(Info) << "Location longitude " << longitude << " latitude: " << latitude;


	sunriseSunset = std::unique_ptr<SunriseSunset>(
			new SunriseSunset(longitude, latitude));

	openPlants();

	this->updateInterval = pt::seconds(20);
}

DataLogger::~DataLogger() {
	//nothing to do
}

void DataLogger::openPlants() {
	std::unordered_set<std::string> connections = pvlib->supportedConnections();
	std::unordered_set<std::string> protocols = pvlib->supportedProtocols();

	odb::session s;
	odb::transaction t (db->begin ());
	odb::result<Plant> r  = db->query<Plant>();

	for (odb::result<Plant>::iterator it(r.begin()); it != r.end (); ++it) {
		const Plant& plant = *it;

		LOG(Info) << "Opening plant " << plant.name << " ["
				<< plant.connection << ", " << plant.protocol << "]";

		if (connections.find(plant.connection) == connections.end()) {
			LOG(Error) << "plant: " << plant.name
					<< "has unsupported connection: " << plant.connection;
		}
		if (protocols.find(plant.protocol) == protocols.end()) {
			LOG(Error) << "plant: " << plant.name
					<< "has unsupported protocol: " << plant.protocol;
		}
		pvlib->openPlant(plant.name, plant.connection, plant.protocol);

		LOG(Info) << "Connecting to plant " << plant.name << " ["
				<< plant.connectionParam << ", " << plant.protocolParam << "]";
		pvlib->connect(plant.name, plant.connectionParam, plant.protocolParam);

		LOG(Info) << "Successfully connected plant " << plant.name << " ["
				<< plant.connectionParam << ", " << plant.protocolParam << "]";

		std::unordered_set<int64_t> inverterIds = pvlib->getInverters(plant.name);

		for (const auto& inverter : plant.inverters) {
			if (inverterIds.count(inverter->id) != 1) {
				LOG(Error) << "Could not open inverter: " << inverter->name;
			} else {
				openInverter.emplace(inverter->id, inverter);
			}
		}

		for (int64_t inverterId : inverterIds) {
			if (openInverter.count(inverterId) != 1) {
				LOG(Warning) << "Found inverter not in database with id: "  << inverterId;
			}
		}
	}
	t.commit();

	//TODO: Error handling check if all inverters are open
	//What to do if not all inverters are available?
}

void DataLogger::closePlants() {
	pvlib->close();
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

void DataLogger::sleepUntill(pt::ptime time) {
	using system_clock = std::chrono::system_clock;

	time_t unixTime = pt::to_time_t(time);
	system_clock::time_point sleep = system_clock::from_time_t(unixTime);

	std::this_thread::sleep_until(sleep);
}

void DataLogger::logDayData()
{
	LOG(Debug) << "logging day yield";
	Pvlib::const_iterator end = pvlib->end();
	for (Pvlib::const_iterator it = pvlib->begin(); it != end; ++it) {
		Stats stats;
		try {
			pvlib->getStats(&stats, it);
		} catch (const PvlogException& ex) {
			LOG(Error) << "Failed getting statistics of inverter" << *it << ": " << ex.what();
			return;
		}

		if (isValid(stats.dayYield)) {
			InverterPtr inverter = openInverter.at(*it);

			bg::date curDate(bg::day_clock::local_day());
			DayData dayData(inverter, curDate, stats.dayYield);

			updateOrInsert(db, dayData);
		} else {
			//FIXME: What to do???
		}
	}
	LOG(Debug) << "logged day yield";
}

void DataLogger::logData()
{
	Pvlib::const_iterator end = pvlib->end();
	for (Pvlib::const_iterator it = pvlib->begin(); it != end; ++it) {
		Ac ac;
		Dc dc;
		Status status;
		try {

			pvlib->getAc(&ac, it);
			pvlib->getDc(&dc, it);
			pvlib->getStatus(&status, it);
		} catch (const PvlogException& ex) {
			LOG(Error) << "Failed getting statistics of inverter" << *it << ": " << ex.what();
			//FIXME: What to do???
			continue; //Ignore inverter
		}

		std::shared_ptr<Inverter> inverter = openInverter.at(*it);

		if (!isValid(ac.totalPower)) {
			//TODO: handle invalid power: for now just ignore it!!!
			LOG(Error) << "Total power of " << inverter->name << " not valid";
			continue;
		}

		if (status.status != PVLIB_STATUS_OK) {
			//TODO: handle error: for know just ignore it!!!
			LOG(Error) << "Status of " << inverter->name << "not OK but " << status.status;
		}

		SpotData spotData = fillSpotData(ac, dc);
		spotData.inverter = inverter;
		spotData.time = util::roundUp(pt::second_clock::universal_time(), updateInterval);

		curSpotData[inverter->id].push_back(spotData);

		if (pt::to_time_t(spotData.time) % timeout.total_seconds() == 0) {
			LOG(Debug) << "logging current power, voltage, ...";
			for (const auto& entry : curSpotData) {
				try {
					SpotData averagedSpotData = average(entry.second);
					averagedSpotData.time = util::roundUp(pt::second_clock::universal_time(), timeout);

					LOG(Trace) << "Persisting spot data: " << averagedSpotData;
					odb::transaction t (db->begin ());
					db->persist(averagedSpotData);
					t.commit();
				} catch (const PvlogException& e) {
					LOG(Error) << "Error averaging spot data: " << e.what() ;
				}
			}
			curSpotData.clear();
		}
	}
}

void DataLogger::work()
{
	try {

		int julianDay = bg::day_clock::universal_day().julian_day();
		pt::ptime sunset  = sunriseSunset->sunset(julianDay);

		while (!quit) {
			pt::ptime curTime = pt::second_clock::universal_time();
			pt::ptime nextUpdate = util::roundUp(curTime, timeout);

			LOG(Debug) << "Sunset: " << pt::to_simple_string(sunset);
			LOG(Debug) << "current time: " << pt::to_simple_string(curTime);
			LOG(Debug) << "time till wait: " << pt::to_simple_string(nextUpdate);

			if (nextUpdate >= sunset) {
				logDayData();

				closePlants();

				int nextJulianDay = bg::day_clock::universal_day().julian_day() + 1;
				pt::ptime nextSunrise = sunriseSunset->sunrise(nextJulianDay);
				sunset = sunriseSunset->sunset(nextJulianDay);
				sleepUntill(nextSunrise);

				if (quit) return;

				openPlants();
			}

			nextUpdate = util::roundUp(curTime, updateInterval);
			sleepUntill(nextUpdate);
			if (quit) return;

			logData();
		}
	} catch (const PvlogException& ex) {
		LOG(Error) << ex.what();
	}
}
