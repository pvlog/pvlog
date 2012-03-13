#ifndef DATE_H
#define DATE_H

#include <cstring>
#include <ctime>

#include "PvlogException.h"

class Date {
	Date(int julianDay)
	{
		time = static_cast<time_t>((julianDay - 2440587.5) * 86400.0);
	}

	Date(int year, int month, int day)
	{
		struct tm tm;
		memset(&tm, 0, sizeof(tm));
		tm.tm_year = year;
		tm.tm_mon  = month;
		tm.tm_mday = day;
		tm.tm_isdst = -1; //use database to determine daylight saving
		time = mktime(&tm);
	}

	int julianDay() const
	{
		return ((double(time) + 2440587.5 * 86400.0) / 86400.0);
	}

	int year() const
	{
		struct tm *lm = localtime(&time);
    	if (lm == NULL)
    		PVLOG_EXCEPT("localtime failed!");
    	return lm->tm_year;
	}

	int month() const
	{
		struct tm *lm = localtime(&time);
    	if (lm == NULL)
    		PVLOG_EXCEPT("localtime failed!");
    	return lm->tm_mon;
	}

	int dayOfMonth() const
	{
		struct tm *lm = localtime(&time);
    	if (lm == NULL)
    		PVLOG_EXCEPT("localtime failed!");
    	return lm->tm_mday;
	}

    bool operator == (const Date& date) const
    {
    	return (this->time == date.time);
    }

    bool operator <= (const Date& date) const
    {
    	return (this->time <= date.time);
    }

    bool operator >= (const Date& date) const
    {
    	return (this->time <= date.time);
    }

    bool operator < (const Date& date) const
    {
    	return (this->time < date.time);
    }

    bool operator > (const Date& date) const
    {
    	return (this->time < date.time);
    }
private:
	time_t time;
};

#endif // #ifndef DATE_H
