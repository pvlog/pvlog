/*
 * This file is part of Pvlog.
 *
 * Copyright (C) 2017 pvlogdev@gmail.com
 *
 * Pvlog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pvlog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pvlog.  If not, see <http://www.gnu.org/licenses/>.
 */

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
