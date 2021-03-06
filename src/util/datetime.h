#ifndef DATE_TIME_H
#define DATE_TIME_H

#include <pvlogexception.h>
#include <ctime>
#include <cstring>
#include <string.h>
#include <utility>
#include <cmath>


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

	DateTime(int year, int month, int day, int hour, int min, int second)
	{
		struct tm time;
		memset(&time, 0, sizeof(time));

		time.tm_year = year;
		time.tm_mon  = month;
		time.tm_hour = hour;
		time.tm_min  = min;
		time.tm_sec  = second;

		this->time = mktime(&time);
		if (this->time == -1) {
			PVLOG_EXCEPT("mktime failed!");
		}
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
		struct timespec time;
		time.tv_sec  = seconds;
		time.tv_nsec = 0;
		if (::nanosleep(&time, NULL) > 0) return false;
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

	int year() const
	{
		struct tm* ltime = localtime(&time);
		if (ltime== NULL) PVLOG_EXCEPT("localtime failed!");
		return ltime->tm_year + 1900;
	}

	int month() const
	{
		struct tm* ltime = localtime(&time);
		if (ltime == NULL) PVLOG_EXCEPT("localtime failed!");
		return ltime->tm_mon;
	}

	int monthDay() const
	{
		struct tm* ltime = localtime(&time);
		if (ltime == NULL) PVLOG_EXCEPT("localtime failed!");
		return ltime->tm_mday;
	}

	double julianDate() const
	{
		return time / 86400.0 + 2440587.5;
	}

	int julianDay() const
	{
		return static_cast<int>(std::round(julianDate()));
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
		char* tm = ctime(&time);
		if (tm == 0) PVLOG_EXCEPT("ctime failed!");
		std::string str(tm);
		if (str.size() > 0) {
			str.resize(str.size() - 1);
		}
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
