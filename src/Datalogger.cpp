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

	//database->plantLocation(longitude, latitude);
	Location location(-10.9, 49.7);
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

bool DataLogger::waitForLogTime(int timeout)
{
	time_t curTime = DateTime::currentUnixTime();
	time_t time = curTime + timeout;

	time = ((time + 30) / 60) * 60; //round to minutes

	return DateTime::sleepUntil(DateTime(time));
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
			ac.time = DateTime::currentUnixTime();
			pvlib->getDc(&dc, it);
			dc.time = DateTime::currentUnixTime();
		} catch (const PvlogException& ex) {
			LOG(Error) << "Failed getting statistics of inverter" << *it << ": " << ex.what();
		}

		database->storeAc(ac, *it);
		database->storeDc(dc, *it);
	}
	LOG(Debug) << "logged current power, voltage, ...";
}

void DataLogger::work()
{
	try {
		int newTimeout = timeout;
		while (!quit) {
			LOG(Debug) << "Data logger work cycle, timeout = " << newTimeout;
			DateTime curTime;
			DateTime sunset = sunriseSunset->sunset(curTime.julianDay());
			LOG(Debug) << "Sunset: " << sunset.timeString();
			LOG(Debug) << "current time: " << curTime.timeString();

			if (curTime + DateTime(newTimeout) >= sunset) {
				logDayData();
				while (waitForDay() == false) {
					if (quit) return;
				}
			}

			if (quit) return;

			LOG(Debug) << "Waiting for log time: " << newTimeout << "seconds";
			while (waitForLogTime(newTimeout) == false) {
				if (quit) return;
			}
			if (quit) return;

			DateTime startTime;
			logData();
			DateTime endTime;

			time_t timeNeeded = endTime - startTime;
			if (timeNeeded > timeout) {
				newTimeout = 0;
				LOG(Warning) << "Timeout < time needed to query data from inverters!";
			} else {
				newTimeout = timeout - timeNeeded;
			}
		}
	} catch (const PvlogException& ex) {
		LOG(Error) << ex.what();
	}
}
