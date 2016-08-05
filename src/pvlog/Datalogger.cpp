#include "Datalogger.h"
#include "Log.h"
#include "SunriseSunset.h"
#include "Pvlib.h"
#include "Database.h"
#include "Utility.h"
#include "Log.h"

DataLogger::DataLogger(Database* database, Pvlib* pvlib, int timeout) :
	quit(false), database(database), pvlib(pvlib)
{
	PVLOG_NOT_NULL(database);
	PVLOG_NOT_NULL(pvlib);

	 if (timeout < 60) {
		 PVLOG_EXCEPT("Timeout must be at least 60 seconds!");
	 }
	 if ((timeout % 60) != 0) {
		 PVLOG_EXCEPT("Timeout must be a multiple of 60 seconds");
	 }

	Database::Location location = database->readLocation();
	sunriseSunset = std::unique_ptr<SunriseSunset>(new SunriseSunset(location.longitude,
	        location.latitude));

	this->timeout = timeout;
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
		Pvlib::Stats stats;
		try {
			pvlib->getStats(&stats, it);
		} catch (const PvlogException& ex) {
			LOG(Error) << "Failed getting statistics of inverter" << *it << ": " << ex.what();
			return;
		}

		database->storeStats(stats, *it);
	}
	LOG(Debug) << "logged day yield";
}

void DataLogger::logData()
{
	LOG(Debug) << "logging current power, voltage, ...";
	Pvlib::const_iterator end = pvlib->end();
	for (Pvlib::const_iterator it = pvlib->begin(); it != end; ++it) {
		Pvlib::Ac ac;
		Pvlib::Dc dc;
		try {
			pvlib->getAc(&ac, it);
			pvlib->getDc(&dc, it);
		} catch (const PvlogException& ex) {
			LOG(Error) << "Failed getting statistics of inverter" << *it << ": " << ex.what();
			return;
		}

		//round time to multiple of timeout
		time_t time = (DateTime::currentUnixTime() / timeout) * timeout;

        ac.time = time;
        dc.time = time;

        LOG(Debug) << "ac:\n" << ac << "\n";
        LOG(Debug) << "dc:\n" << dc << "\n";
		database->storeAc(ac, *it);
		database->storeDc(dc, *it);
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
				while (waitForDay() == false) {
					if (quit) return;
				}
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
