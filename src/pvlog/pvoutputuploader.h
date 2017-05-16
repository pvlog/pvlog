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

#ifndef PVOUTPUT_UPLOADER_H
#define PVOUTPUT_UPLOADER_H

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <Poco/Net/Context.h>

#include "models/spotdata.h"
#include "models/daydata.h"

namespace odb {
	class database;
}

class PvoutputUploader {
public:
	PvoutputUploader(odb::database* db);

	void uploadSpotData(const std::vector<model::SpotData>& spotDatas);

	void uploadDayData(const std::vector<model::DayData>& dayData);

protected:
	void uploadSpotDataSum(boost::posix_time::ptime time, int32_t power, int32_t yield,
			const std::string& systemId, const std::string& apiKey);

	void uploadDayDataSum(boost::gregorian::date date, int32_t yield,
			const std::string& systemId, const std::string& apiKey);
private:
	odb::database* db;
	Poco::Net::Context::Ptr ptrContext;
};

#endif //#ifndef PVOUTPUT_UPLOADER_H
