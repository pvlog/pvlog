#ifndef SUNRISE_SUNSET_H
#define SUNRISE_SUNSET_H

#include "DateTime.h"

class Location;

class SunriseSunset {
public:
	explicit SunriseSunset(float longitude, float latitude);

	DateTime sunrise(int julianDay);

	DateTime sunset(int julianDay);

private:
	void calculate(int julianDay);

	float longitude;
	float latitude;
	int   lastJulianDay;
	DateTime  curSunrise;
	DateTime  curSunset;
};

#endif // SUNRISE_SUNSET_H
