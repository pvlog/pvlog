#ifndef DATE_TIME_H
#define DATE_TIME_H

#include <ctime>
#include <cstring>
#include <utility>

#include "PvlogException.h"

class DateTime {
private:
	time_t time;
public:
	explicit DateTime(time_t time) :
		time(time)
	{
		/* nothing to do */
	}

	explicit DateTime(int time) :
		time(time)
	{
		/* nothing to do */
	}

	explicit DateTime(double julianDay)
	{
		time = (julianDay - 2440587.5) * 86400.0;
	}

	DateTime()
	{
		time = ::time(NULL);
		if (time == -1) {
			PVLOG_EXCEPT("\"time\" failed!");
		}
	}

	/**
	 * sleep for specified amount of seconds.
	 *
	 * @return false if interrupted else true.
	 */
	static bool sleep(time_t seconds)
	{
		if (::sleep(seconds) > 0) return false;
		else return true;
	}

	static bool sleepUntil(const DateTime& time)
	{
		DateTime curTime;
		if (time <= curTime) return true;
		else return sleep(time.time - curTime.time);
	}

	static time_t currentUnixTime()
	{
		time_t time = ::time(NULL);
		if (time == -1) {
			PVLOG_EXCEPT("\"time\" failed!");
		}
		return time;
	}

	int year()
	{
		struct tm *gm = gmtime(&time);
		if (gm == NULL) PVLOG_EXCEPT("gmtime failed");
		return gm->tm_year + 1900;
	}

	int month()
	{
		struct tm *gm = gmtime(&time);
		if (gm == NULL) PVLOG_EXCEPT("gmtime failed");
		return gm->tm_mon;
	}

	int monthDay()
	{
		struct tm *gm = gmtime(&time);
		if (gm == NULL) PVLOG_EXCEPT("gmtime failed");
		return gm->tm_mday;
	}

	double julianDate()
	{
		return ((double(time) + 2440587.5 * 86400.0) / 86400.0);
	}

	int julianDay()
	{
		return static_cast<int> ((time + 2440587.5 * 86400.0) / 86400.0);
	}

	time_t unixTime() const
	{
		return time;
	}

	/**
	 * Return localtime string.
	 * Format:  Www Mmm dd hh:mm:ss yyyy
	 * Where Www is the weekday, Mmm the month in letters, dd the day of the month, hh:mm:ss the time, and yyyy the year.
	 */
	std::string timeString()
	{
		std::string str(ctime(&time));
		str.resize(str.size() - 1);
		return str;
	}

	DateTime& operator +=(const DateTime& time)
	{
		this->time += time.time;
		return *this;
	}

	DateTime operator +(const DateTime& time)
	{
		return DateTime(this->time + time.time);
	}

	time_t operator -(const DateTime& time) const
	{
		return this->time - time.time;
	}

	DateTime& operator -=(const DateTime& time)
	{
		this->time -= time.time;
		return *this;
	}

	bool operator ==(const DateTime& time) const
	{
		return (this->time == time.time);
	}

	bool operator <=(const DateTime& time) const
	{
		return (this->time <= time.time);
	}

	bool operator >=(const DateTime& time) const
	{
		return (this->time >= time.time);
	}

	bool operator <(const DateTime& time) const
	{
		return (this->time < time.time);
	}

	bool operator >(const DateTime& time) const
	{
		return (this->time > time.time);
	}

};

#endif // #ifdef DATE_TIME_H
