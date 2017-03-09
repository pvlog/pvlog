#include <cmath>
#include <utility>
#include <limits>
#include <iostream>
#include <iomanip>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <sunrisesunset.h>

using boost::posix_time::ptime;
using boost::posix_time::from_time_t;

namespace {
const double PI = 3.1415926535897932384626433832795028841968;
//*********************************************************************/

// Convert radian angle to degrees

double radToDeg(double angleRad) {
	return (180.0 * angleRad / PI);
}

//*********************************************************************/

// Convert degree angle to radians

double degToRad(double angleDeg) {
	return (PI * angleDeg / 180.0);
}

//***********************************************************************/
//* Name: calcTimeJulianCent
//* Type: Function
//* Purpose: convert Julian Day to centuries since J2000.0.
//* Arguments:
//* jd : the Julian Day to convert
//* Return value:
//* the T value corresponding to the Julian Day
//***********************************************************************/

double calcTimeJulianCent(double jd) {
	double T = (jd - 2451545.0) / 36525.0;
	return T;
}

//***********************************************************************/
//* Name: calcJDFromJulianCent
//* Type: Function
//* Purpose: convert centuries since J2000.0 to Julian Day.
//* Arguments:
//* t : number of Julian centuries since J2000.0
//* Return value:
//* the Julian Day corresponding to the t value
//***********************************************************************/

double calcJDFromJulianCent(double t) {
	double JD = t * 36525.0 + 2451545.0;
	return JD;
}

//***********************************************************************/
//* Name: calGeomMeanLongSun
//* Type: Function
//* Purpose: calculate the Geometric Mean Longitude of the Sun
//* Arguments:
//* t : number of Julian centuries since J2000.0
//* Return value:
//* the Geometric Mean Longitude of the Sun in degrees
//***********************************************************************/

double calcGeomMeanLongSun(double t) {
	double L0 = 280.46646 + t * (36000.76983 + 0.0003032 * t);
	while (L0 > 360.0) {
		L0 -= 360.0;
	}
	while (L0 < 0.0) {
		L0 += 360.0;
	}
	return L0;   // in degrees
}

//***********************************************************************/
//* Name: calGeomAnomalySun
//* Type: Function
//* Purpose: calculate the Geometric Mean Anomaly of the Sun
//* Arguments:
//* t : number of Julian centuries since J2000.0
//* Return value:
//* the Geometric Mean Anomaly of the Sun in degrees
//***********************************************************************/

double calcGeomMeanAnomalySun(double t) {
	double M = 357.52911 + t * (35999.05029 - 0.0001537 * t);
	return M;    // in degrees
}

//***********************************************************************/
//* Name: calcEccentricityEarthOrbit
//* Type: Function
//* Purpose: calculate the eccentricity of earth's orbit
//* Arguments:
//* t : number of Julian centuries since J2000.0
//* Return value:
//* the unitless eccentricity
//***********************************************************************/

double calcEccentricityEarthOrbit(double t) {
	double e = 0.016708634 - t * (0.000042037 + 0.0000001267 * t);
	return e;    // unitless
}

//***********************************************************************/
//* Name: calcSunEqOfCenter
//* Type: Function
//* Purpose: calculate the equation of center for the sun
//* Arguments:
//* t : number of Julian centuries since J2000.0
//* Return value:
//* in degrees
//***********************************************************************/

double calcSunEqOfCenter(double t) {
	double m = calcGeomMeanAnomalySun(t);

	double mrad = degToRad(m);
	double sinm = sin(mrad);
	double sin2m = sin(mrad + mrad);
	double sin3m = sin(mrad + mrad + mrad);

	double C = sinm * (1.914602 - t * (0.004817 + 0.000014 * t))
			+ sin2m * (0.019993 - 0.000101 * t) + sin3m * 0.000289;
	return C;    // in degrees
}

//***********************************************************************/
//* Name: calcSunTrueLong
//* Type: Function
//* Purpose: calculate the true longitude of the sun
//* Arguments:
//* t : number of Julian centuries since J2000.0
//* Return value:
//* sun's true longitude in degrees
//***********************************************************************/

double calcSunTrueLong(double t) {
	double l0 = calcGeomMeanLongSun(t);
	double c = calcSunEqOfCenter(t);

	double O = l0 + c;
	return O;    // in degrees
}

//***********************************************************************/
//* Name: calcSunTrueAnomaly
//* Type: Function
//* Purpose: calculate the true anamoly of the sun
//* Arguments:
//* t : number of Julian centuries since J2000.0
//* Return value:
//* sun's true anamoly in degrees
//***********************************************************************/

double calcSunTrueAnomaly(double t) {
	double m = calcGeomMeanAnomalySun(t);
	double c = calcSunEqOfCenter(t);

	double v = m + c;
	return v;    // in degrees
}

//***********************************************************************/
//* Name: calcSunRadVector
//* Type: Function
//* Purpose: calculate the distance to the sun in AU
//* Arguments:
//* t : number of Julian centuries since J2000.0
//* Return value:
//* sun radius vector in AUs
//***********************************************************************/

double calcSunRadVector(double t) {
	double v = calcSunTrueAnomaly(t);
	double e = calcEccentricityEarthOrbit(t);

	double R = (1.000001018 * (1 - e * e)) / (1 + e * cos(degToRad(v)));
	return R;    // in AUs
}

//***********************************************************************/
//* Name: calcSunApparentLong
//* Type: Function
//* Purpose: calculate the apparent longitude of the sun
//* Arguments:
//* t : number of Julian centuries since J2000.0
//* Return value:
//* sun's apparent longitude in degrees
//***********************************************************************/

double calcSunApparentLong(double t) {
	double o = calcSunTrueLong(t);

	double omega = 125.04 - 1934.136 * t;
	double lambda = o - 0.00569 - 0.00478 * sin(degToRad(omega));
	return lambda;   // in degrees
}

//***********************************************************************/
//* Name: calcMeanObliquityOfEcliptic
//* Type: Function
//* Purpose: calculate the mean obliquity of the ecliptic
//* Arguments:
//* t : number of Julian centuries since J2000.0
//* Return value:
//* mean obliquity in degrees
//***********************************************************************/

double calcMeanObliquityOfEcliptic(double t) {
	double seconds = 21.448 - t * (46.8150 + t * (0.00059 - t * (0.001813)));
	double e0 = 23.0 + (26.0 + (seconds / 60.0)) / 60.0;
	return e0;   // in degrees
}

//***********************************************************************/
//* Name: calcObliquityCorrection
//* Type: Function
//* Purpose: calculate the corrected obliquity of the ecliptic
//* Arguments:
//* t : number of Julian centuries since J2000.0
//* Return value:
//* corrected obliquity in degrees
//***********************************************************************/

double calcObliquityCorrection(double t) {
	double e0 = calcMeanObliquityOfEcliptic(t);

	double omega = 125.04 - 1934.136 * t;
	double e = e0 + 0.00256 * cos(degToRad(omega));
	return e;    // in degrees
}

//***********************************************************************/
//* Name: calcSunRtAscension
//* Type: Function
//* Purpose: calculate the right ascension of the sun
//* Arguments:
//* t : number of Julian centuries since J2000.0
//* Return value:
//* sun's right ascension in degrees
//***********************************************************************/

double calcSunRtAscension(double t) {
	double e = calcObliquityCorrection(t);
	double lambda = calcSunApparentLong(t);

	double tananum = (cos(degToRad(e)) * sin(degToRad(lambda)));
	double tanadenom = (cos(degToRad(lambda)));
	double alpha = radToDeg(atan2(tananum, tanadenom));
	return alpha;    // in degrees
}

//***********************************************************************/
//* Name: calcSunDeclination
//* Type: Function
//* Purpose: calculate the declination of the sun
//* Arguments:
//* t : number of Julian centuries since J2000.0
//* Return value:
//* sun's declination in degrees
//***********************************************************************/

double calcSunDeclination(double t) {
	double e = calcObliquityCorrection(t);
	double lambda = calcSunApparentLong(t);

	double sint = sin(degToRad(e)) * sin(degToRad(lambda));
	double theta = radToDeg(asin(sint));
	return theta;    // in degrees
}

//***********************************************************************/
//* Name: calcEquationOfTime
//* Type: Function
//* Purpose: calculate the difference between true solar time and mean
//*  solar time
//* Arguments:
//* t : number of Julian centuries since J2000.0
//* Return value:
//* equation of time in minutes of time
//***********************************************************************/

double calcEquationOfTime(double t) {
	double epsilon = calcObliquityCorrection(t);
	double l0 = calcGeomMeanLongSun(t);
	double e = calcEccentricityEarthOrbit(t);
	double m = calcGeomMeanAnomalySun(t);

	double y = tan(degToRad(epsilon) / 2.0);
	y *= y;

	double sin2l0 = sin(2.0 * degToRad(l0));
	double sinm = sin(degToRad(m));
	double cos2l0 = cos(2.0 * degToRad(l0));
	double sin4l0 = sin(4.0 * degToRad(l0));
	double sin2m = sin(2.0 * degToRad(m));

	double Etime = y * sin2l0 - 2.0 * e * sinm + 4.0 * e * y * sinm * cos2l0
			- 0.5 * y * y * sin4l0 - 1.25 * e * e * sin2m;

	return radToDeg(Etime) * 4.0;   // in minutes of time
}

//***********************************************************************/
//* Name: calcHourAngleSunrise
//* Type: Function
//* Purpose: calculate the hour angle of the sun at sunrise for the
//*  latitude
//* Arguments:
//* lat : latitude of observer in degrees
//* solarDec : declination angle of sun in degrees
//* Return value:
//* hour angle of sunrise in radians
//***********************************************************************/

double calcHourAngleSunrise(double lat, double solarDec) {
	double latRad = degToRad(lat);
	double sdRad = degToRad(solarDec);

	//double HAarg = (cos(degToRad(90.833)) / (cos(latRad) * cos(sdRad)) - tan(latRad) * tan(sdRad));

	double HA = (acos(
			cos(degToRad(90.833)) / (cos(latRad) * cos(sdRad))
					- tan(latRad) * tan(sdRad)));

	return HA;   // in radians
}

//***********************************************************************/
//* Name: calcHourAngleSunset
//* Type: Function
//* Purpose: calculate the hour angle of the sun at sunset for the
//*  latitude
//* Arguments:
//* lat : latitude of observer in degrees
//* solarDec : declination angle of sun in degrees
//* Return value:
//* hour angle of sunset in radians
//***********************************************************************/

double calcHourAngleSunset(double lat, double solarDec) {
	double latRad = degToRad(lat);
	double sdRad = degToRad(solarDec);

	//double HAarg = (cos(degToRad(90.833)) / (cos(latRad) * cos(sdRad)) - tan(latRad) * tan(sdRad));

	double HA = (acos(
			cos(degToRad(90.833)) / (cos(latRad) * cos(sdRad))
					- tan(latRad) * tan(sdRad)));

	return -HA;  // in radians
}

//***********************************************************************/
//* Name: calcSolNoonUTC
//* Type: Function
//* Purpose: calculate the Universal Coordinated Time (UTC) of solar
//*  noon for the given day at the given location on earth
//* Arguments:
//* t : number of Julian centuries since J2000.0
//* longitude : longitude of observer in degrees
//* Return value:
//* time in minutes from zero Z
//***********************************************************************/

double calcSolNoonUTC(double t, double longitude) {
	// First pass uses approximate solar noon to calculate eqtime
	double tnoon = calcTimeJulianCent(
			calcJDFromJulianCent(t) + longitude / 360.0);
	double eqTime = calcEquationOfTime(tnoon);
	double solNoonUTC = 720 + (longitude * 4) - eqTime; // min

	double newt = calcTimeJulianCent(
			calcJDFromJulianCent(t) - 0.5 + solNoonUTC / 1440.0);

	eqTime = calcEquationOfTime(newt);
	// double solarNoonDec = calcSunDeclination(newt);
	solNoonUTC = 720 + (longitude * 4) - eqTime; // min

	return solNoonUTC;
}

//***********************************************************************/
//* Name: calcSunriseUTC
//* Type: Function
//* Purpose: calculate the Universal Coordinated Time (UTC) of sunrise
//*  for the given day at the given location on earth
//* Arguments:
//* JD : julian day
//* latitude : latitude of observer in degrees
//* longitude : longitude of observer in degrees
//* Return value:
//* time in minutes from zero Z
//***********************************************************************/

double calcSunriseUTC(double JD, double latitude, double longitude) {
	double t = calcTimeJulianCent(JD);

	// *** Find the time of solar noon at the location, and use
	// that declination. This is better than start of the
	// Julian day

	double noonmin = calcSolNoonUTC(t, longitude);
	double tnoon = calcTimeJulianCent(JD + noonmin / 1440.0);

	// *** First pass to approximate sunrise (using solar noon)

	double eqTime = calcEquationOfTime(tnoon);
	double solarDec = calcSunDeclination(tnoon);
	double hourAngle = calcHourAngleSunrise(latitude, solarDec);

	double delta = longitude - radToDeg(hourAngle);
	double timeDiff = 4 * delta;    // in minutes of time
	double timeUTC = 720 + timeDiff - eqTime;   // in minutes

	// alert("eqTime = " + eqTime + "\nsolarDec = " + solarDec + "\ntimeUTC = " + timeUTC);

	// *** Second pass includes fractional jday in gamma calc

	double newt = calcTimeJulianCent(
			calcJDFromJulianCent(t) + timeUTC / 1440.0);
	eqTime = calcEquationOfTime(newt);
	solarDec = calcSunDeclination(newt);
	hourAngle = calcHourAngleSunrise(latitude, solarDec);
	delta = longitude - radToDeg(hourAngle);
	timeDiff = 4 * delta;
	timeUTC = 720 + timeDiff - eqTime; // in minutes

	// alert("eqTime = " + eqTime + "\nsolarDec = " + solarDec + "\ntimeUTC = " + timeUTC);

	return timeUTC;
}

//***********************************************************************/
//* Name:    calcSunsetUTC                              */
//* Type:    Function                                   */
//* Purpose: calculate the Universal Coordinated Time (UTC) of sunset   */
//*         for the given day at the given location on earth    */
//* Arguments:                                      */
//*   JD  : julian day                                  */
//*   latitude : latitude of observer in degrees                */
//*   longitude : longitude of observer in degrees              */
//* Return value:                                       */
//*   time in minutes from zero Z                           */
//***********************************************************************/

double calcSunsetUTC(double JD, double latitude, double longitude) {
	double t = calcTimeJulianCent(JD);

	// *** Find the time of solar noon at the location, and use
	//     that declination. This is better than start of the
	//     Julian day

	double noonmin = calcSolNoonUTC(t, longitude);
	double tnoon = calcTimeJulianCent(JD + noonmin / 1440.0);

	// First calculates sunrise and approx length of day

	double eqTime = calcEquationOfTime(tnoon);
	double solarDec = calcSunDeclination(tnoon);
	double hourAngle = calcHourAngleSunset(latitude, solarDec);

	double delta = longitude - radToDeg(hourAngle);
	double timeDiff = 4 * delta;
	double timeUTC = 720 + timeDiff - eqTime;

	// first pass used to include fractional day in gamma calc

	double newt = calcTimeJulianCent(
			calcJDFromJulianCent(t) + timeUTC / 1440.0);
	eqTime = calcEquationOfTime(newt);
	solarDec = calcSunDeclination(newt);
	hourAngle = calcHourAngleSunset(latitude, solarDec);

	delta = longitude - radToDeg(hourAngle);
	timeDiff = 4 * delta;
	timeUTC = 720 + timeDiff - eqTime; // in minutes

	return timeUTC;
}

} //namespace {

//static std::pair<double, double> calcSunriseSunset(int julianDay,
//		double longitude, double latitude) {
//	double sunrise = calcSunriseUTC(julianDay, latitude, longitude);
//	double sunset = calcSunsetUTC(julianDay, latitude, longitude);
//
//	return std::make_pair(julianDay - 0.5 + sunrise / 1440,
//			julianDay - 0.5 + sunset / 1440);
//}

SunriseSunset::SunriseSunset(float longitude, float latitude) :
		longitude(longitude), latitude(latitude), lastJulianDay(-1)
{
	//nothing to do
}

//void SunriseSunset::calculate(int julianDay) {
//	std::pair<double, double> sun = calcSunriseSunset(julianDay, longitude,
//			latitude);
//	curSunrise = from_time_t((sun.first - 2440587.5) * 86400.0);
//	curSunset = from_time_t((sun.second - 2440587.5) * 86400.0);;
//}

ptime SunriseSunset::sunrise(int julianDay) {
	double sunrise = julianDay - 0.5 + calcSunriseUTC(julianDay, latitude, longitude) / 1440;

	//convert to time_t
	time_t sunriseUnix = (sunrise - 2440587.5) * 86400.0;
	return from_time_t(sunriseUnix);
}

ptime SunriseSunset::sunset(int julianDay) {
	double sunset = julianDay - 0.5 + calcSunsetUTC(julianDay, latitude, longitude) / 1440;

	//convert to time_t
	time_t sunsetUnix = (sunset - 2440587.5) * 86400.0;
	return from_time_t(sunsetUnix);
}
