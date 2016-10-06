#ifndef SUNRISE_SUNSET_H
#define SUNRISE_SUNSET_H

#include <boost/date_time/posix_time/posix_time.hpp>

class Location;

class SunriseSunset {
public:
	explicit SunriseSunset(float longitude, float latitude);

	boost::posix_time::ptime sunrise(int julianDay);

	boost::posix_time::ptime sunset(int julianDay);

private:
//	void calculate(int julianDay);

	float longitude;
	float latitude;
	int lastJulianDay;
//	boost::posix_time::ptime curSunrise;
//	boost::posix_time::ptime curSunset;
};

#endif // SUNRISE_SUNSET_H
