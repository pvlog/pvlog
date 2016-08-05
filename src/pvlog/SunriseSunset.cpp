#include "SunriseSunset.h"

#include <cmath>
#include <utility>
#include <limits>
#include <iostream>
#include <iomanip>

static const double PI = 3.1415926535897932384626433832795028841968;

static double sinDeg(double in)
{
	return sin(in * PI / 180.0);
}

static double asinDeg(double in)
{
	return asin(in) * (180.0 / PI);
}

static double cosDeg(double in)
{
	return cos(in * PI / 180.0);
}

static double acosDeg(double in)
{
	return acos(in) * (180.0 / PI);
}

static double currentJulianCycle(double julianDate, double longitude)
{
	double n = julianDate - 2451545.0009 - (longitude / 360.0);
	return floor(n + 0.5);
}

static double approximateSolarNoon(double curJulianCycle, double longitude)
{
	return 2451545.0009 + (longitude / 360.0) + curJulianCycle;
}

static double solarMeanAnomaly(double solarNoon)
{
	return fmod(357.5291 + 0.98560028 * (solarNoon - 2451545.0), 360);
}

static double equationOfCenter(double solarMeanAnomaly)
{
	double m = solarMeanAnomaly;
	return 1.9148 * sinDeg(m) + 0.0200 * sinDeg(2 * m) + 0.0003 * sinDeg(3 * m);
}

static double elipticLongitude(double solarMeanAnomaly, double equationOfCenter)
{
	double m = solarMeanAnomaly;
	double c = equationOfCenter;
	return fmod((m + 102.9372 + c + 180), 360);
}

static double solarTransit(double solarNoon, double solarMeanAnomaly, double elipticLongitude)
{
	double m = solarMeanAnomaly;
	return solarNoon + 0.0053 * sinDeg(m) - 0.0069 * sinDeg(2 * elipticLongitude);
}

static double declinationOfSun(double elipticLongitude)
{
	return asinDeg(sinDeg(elipticLongitude) * sinDeg(23.45));
}

static double hourAngle(double declinationOfSun, double latitude)
{
	double z = sinDeg(-0.83) - sinDeg(latitude) * sinDeg(declinationOfSun);
	double n = cosDeg(latitude) * cosDeg(declinationOfSun);
	return acosDeg(z / n);
}

static std::pair<double, double> calcSunriseSunset(double julianDate,
                                                   double longitude,
                                                   double latitude)
{
	double JulianCycle;
	double SolarNoon;
	double SolarMeanAnomaly;
	double EquationOfCenter;
	double ElipticLongitude;
	double SolarTransit;
	double DeclinationOfSun;
	double HourAngle;

	JulianCycle = currentJulianCycle(julianDate, longitude);
	SolarNoon = approximateSolarNoon(JulianCycle, longitude);
	SolarMeanAnomaly = solarMeanAnomaly(SolarNoon);
	EquationOfCenter = equationOfCenter(SolarMeanAnomaly);
	ElipticLongitude = elipticLongitude(SolarMeanAnomaly, EquationOfCenter);
	SolarTransit = solarTransit(SolarNoon, SolarMeanAnomaly, ElipticLongitude);
	DeclinationOfSun = declinationOfSun(ElipticLongitude);
	HourAngle = hourAngle(DeclinationOfSun, latitude);

	double sunset = 2451545.0009 + (HourAngle + longitude) / 360.0 + JulianCycle + 0.0053 * sinDeg(
	        SolarMeanAnomaly) - 0.0069 * sinDeg(2 * ElipticLongitude);

	double sunrise = SolarTransit - (sunset - SolarTransit);

	return std::make_pair(sunrise, sunset);
}

SunriseSunset::SunriseSunset(float longitude, float latitude) :
	longitude(longitude), latitude(latitude), lastJulianDay(-1)
{
	//nothing to do
}

void SunriseSunset::calculate(int julianDay)
{
	std::pair<double, double> sun = calcSunriseSunset(static_cast<double> (julianDay), longitude,
	        latitude);
	curSunrise = DateTime(sun.first);
	curSunset = DateTime(sun.second);
}

DateTime SunriseSunset::sunrise(int julianDay)
{
	if ((lastJulianDay == -1) || lastJulianDay != julianDay) {
		calculate(julianDay);
		lastJulianDay = julianDay;
		return curSunrise;
	} else {
		return curSunrise;
	}
}

DateTime SunriseSunset::sunset(int julianDay)
{
	if ((lastJulianDay == -1) || lastJulianDay != julianDay) {
		calculate(julianDay);
		lastJulianDay = julianDay;
		return curSunset;
	} else {
		return curSunset;
	}
}
