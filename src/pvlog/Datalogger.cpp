#include "Datalogger.h"

#include <boost/date_time/gregorian/gregorian_types.hpp>

#include "Log.h"
#include "SunriseSunset.h"
#include "Pvlib.h"
#include "Utility.h"
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
void setIfValid(odb::nullable<T>& s, T t) {
	if (isValid(t)) {
		s = t;
	}
}

} //namespace {

static SpotData fillSpotData(const Ac& ac, const Dc& dc) {
	SpotData spotData;

	setIfValid(spotData.power, ac.totalPower);
	setIfValid(spotData.frequency, ac.frequency);
	for (int i = 0; i < ac.lineNum; ++i) {
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

		spotData.dcInput.emplace(i + 1, dcInput);
	}

	return spotData;
}


DataLogger::DataLogger(odb::core::database* database, Pvlib* pvlib, int timeout) :
		quit(false), db(database), pvlib(pvlib)
{
	PVLOG_NOT_NULL(database);
	PVLOG_NOT_NULL(pvlib);

	if (timeout < 60) {
		PVLOG_EXCEPT("Timeout must be at least 60 seconds!");
	}
	if ((timeout % 60) != 0) {
		PVLOG_EXCEPT("Timeout must be a multiple of 60 seconds");
	}

	float longitude = std::stof(readConfig(db, "longitude"));
	float latitude = std::stof(readConfig(db, "latitude"));

	sunriseSunset = std::unique_ptr<SunriseSunset>(
			new SunriseSunset(longitude, latitude));

	openPlants();

	this->timeout = timeout;
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

bool DataLogger::waitForDay()
{
	/*	if (sunriseSunset->isDay(DateTime().julianDay())) {
	 return true;
	 }
	 */
	LOG(Debug) << "Waiting for day begin...";
	DateTime time = sunriseSunset->sunrise(DateTime().julianDay() + 1);
	return DateTime::sleepUntil(time);
}

bool DataLogger::waitForLogTime(DateTime timeToWait)
{
	return DateTime::sleepUntil(timeToWait);
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

			bg::date curDate(bg::day_clock::universal_day());
			DayData dayData(inverter, curDate, stats.dayYield);

			odb::transaction t(db->begin());
			db->persist(dayData);
			t.commit();
		} else {
			//FIXME: What to do???
		}
	}
	LOG(Debug) << "logged day yield";
}

void DataLogger::logData()
{
	LOG(Debug) << "logging current power, voltage, ...";
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


		spotData.time = (std::time(nullptr) / timeout) * timeout;

		LOG(Trace) << "spot data: " << spotData;
		{
			odb::transaction t (db->begin ());
			db->persist(spotData);
			t.commit();
		}

	}
	LOG(Debug) << "logged current power, voltage, ...";
}

void DataLogger::work()
{
	try {
		while (!quit) {
			DateTime curTime;
			time_t time = ((curTime.unixTime() + timeout) / timeout) * timeout;

			DateTime sunset = sunriseSunset->sunset(curTime.julianDay());
			LOG(Debug) << "Sunset: " << sunset.timeString();
			LOG(Debug) << "current time: " << curTime.timeString();
			LOG(Debug) << "time till wait: " << DateTime(time).timeString();

			if (time >= sunset.unixTime()) {
				logDayData();

				closePlants();

				while (waitForDay() == false) {
					if (quit) return;
				}

				openPlants();
			}

			if (quit) return;

			time = ((DateTime::currentUnixTime() + timeout) / timeout) * timeout;
			while (waitForLogTime(DateTime(time)) == false) {
				if (quit) return;
			}
			if (quit) return;

			logData();
		}
	} catch (const PvlogException& ex) {
		LOG(Error) << ex.what();
	}
}
