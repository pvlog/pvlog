#include "DataLogger.h"
#include "Log.h"
#include "SunriseSunset.h"
#include "Pvlib.h"
#include "Database.h"
#include "Utility.h"

DataLogger::DataLogger(Database* database, Pvlib* pvlib, int timeout) :
	quit(false),
	database(database),
	pvlib(pvlib)
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
	sunriseSunset = new SunriseSunset(location.longitude, location.latitude);

	this->timeout = timeout;
}



bool DataLogger::waitForDay()
{
/*	if (sunriseSunset->isDay(DateTime().julianDay())) {
		return true;
	}
*/
	DateTime time = sunriseSunset->sunrise(DateTime().julianDay() + 1);
	return DateTime::sleepUntil(time);
}

bool DataLogger::waitForLogTime(int timeout)
{
	time_t curTime = DateTime::currentTime();
	time_t time = curTime + timeout;

	time = ((time + 30) / 60) * 60; //round to minutes
	return DateTime::sleepUntil(DateTime(time));
}

void DataLogger::logDayData()
{
	Pvlib::const_iterator end = pvlib->end();
	for (Pvlib::const_iterator it = pvlib->begin(); it != end; ++it) {
		Pvlib::Stats stats;
		try {
			pvlib->getStats(&stats, it);
		}
		catch (const PvlogException& ex) {
			log_error("Failed getting statistics of inverter %d: %s",
				it->inverter(), ex.what());
			return;
		}

		database->storeStats(stats, *it);
	}
}

void DataLogger::logData()
{
	Pvlib::const_iterator end = pvlib->end();
	for (Pvlib::const_iterator it = pvlib->begin(); it != end; ++it) {
		Pvlib::Ac ac;
		Pvlib::Dc dc;
		try {
			pvlib->getAc(&ac, it);
			pvlib->getDc(&dc, it);
		}
		catch (const PvlogException& ex) {
			log_error("Failed getting ac/dc  data of inverter %d: %s",
				it->inverter(), ex.what());
		}

		database->storeAc(ac, *it);
		database->storeDc(dc, *it);
	}
}

void DataLogger::work()
{
	try {
		int newTimeout = timeout;
		while (!quit) {
			if ((DateTime() + DateTime(newTimeout)) >= sunriseSunset->sunset(DateTime().julianDay())) {
				logDayData();
				while (waitForDay() == false) {
					if (quit) return;
				}
			}
			if (quit) return;
			while (waitForLogTime(timeout) == false) {
				if (quit) return;
			}
			if (quit) return;

			DateTime startTime;
			logData();
			DateTime endTime;

			time_t timeNeeded = endTime - startTime;
			if (timeNeeded > timeout) {
				newTimeout = 0;
				log_warning("Timout < time needed to query data from inverters!\n");
			} else {
				newTimeout = timeout - timeNeeded;
			}
		}
	}
	catch (const PvlogException& ex) {
		log_failure(ex.what());
	}
}
