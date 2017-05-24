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

#ifndef SRC_PVLOG_MODELS_DAYDATA_H_
#define SRC_PVLOG_MODELS_DAYDATA_H_

#include <memory>

#include <jsoncpp/json/value.h>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "version.h"

#include "inverter.h"

namespace model {

#pragma db object
struct DayData {
	#pragma db id auto
	int id;

	#pragma db not_null
	std::shared_ptr<Inverter> inverter;

	boost::gregorian::date date;

	int64_t dayYield;

	DayData(std::shared_ptr<Inverter> inverter, boost::gregorian::date date, int64_t dayYield) :
			id(0),
			inverter(inverter),
			date(date),
			dayYield(dayYield) {
		//nothing to do
	}

	DayData() : id(0), dayYield(0) {
		//hothing to do
	}
};

#pragma db view object(DayData) object(Inverter)\
	query((?) + "GROUP BY year, month, " + Inverter::id)
struct DayDataMonth {
	#pragma db column("sum(" + DayData::dayYield + ")")
	int64_t yield;

	//FIXME: support other databases than sqlite
	#pragma db column("strftime('%m'," + DayData::date + ") AS month")
	int month;

	//FIXME: support other databases than sqlite
	#pragma db column("strftime('%Y'," + DayData::date + ") AS year")
	int year;

	#pragma db column(Inverter::id)
	int64_t inverterId;
};

#pragma db view object(DayData) object(Inverter)\
	query((?) + "GROUP BY date ORDER BY yield desc LIMIT 20")
struct TopNDay {
	#pragma db column("sum(" + DayData::dayYield + ") AS yield")
	int64_t yield;

	#pragma db column(DayData::date)
	boost::gregorian::date date;
};

#pragma db view object(DayData) object(Inverter)\
	query((?) + "GROUP BY date ORDER BY yield asc LIMIT 20")
struct LowNDay {
	#pragma db column("sum(" + DayData::dayYield + ") AS yield")
	int64_t yield;

	#pragma db column(DayData::date)
	boost::gregorian::date date;
};



#pragma db view\
	query("SELECT CAST(strftime('%m', date) AS INTEGER) AS month, CAST(strftime('%d', date) AS INTEGER) AS day, "\
			"MAX(sum_dayyield) AS max, AVG(sum_dayyield) AS avg, MIN(sum_dayyield) AS min, count(*) AS count FROM "\
			"(SELECT date, SUM(day_yield) AS sum_dayyield "\
			"FROM day_data GROUP BY date) WHERE " + (?) + " GROUP BY month, day")
struct DayStats {
	#pragma db type("INTEGER")
	int month;

	#pragma db type("INTEGER")
	int day;

	#pragma db type("INTEGER")
	int64_t max;

	#pragma db type("INTEGER")
	int64_t avg;

	#pragma db type("INTEGER")
	int64_t min;

	#pragma db type("INTEGER")
	int count;
};

#pragma db view object(DayData) object(Inverter)\
	query((?) + "GROUP BY year, month ORDER BY yield desc LIMIT 20")
struct TopNMonth {
	#pragma db column("sum(" + DayData::dayYield + ") AS yield")
	int64_t yield;

	//FIXME: support other databases than sqlite
	#pragma db column("strftime('%m'," + DayData::date + ") AS month")
	int month;

	//FIXME: support other databases than sqlite
	#pragma db column("strftime('%Y'," + DayData::date + ") AS year")
	int year;
};

#pragma db view object(DayData) object(Inverter)\
	query((?) + "GROUP BY year, month ORDER BY yield asc LIMIT 20")
struct LowNMonth{
	#pragma db column("sum(" + DayData::dayYield + ") AS yield")
	int64_t yield;

	//FIXME: support other databases than sqlite
	#pragma db column("strftime('%m'," + DayData::date + ") AS month")
	int month;

	//FIXME: support other databases than sqlite
	#pragma db column("strftime('%Y'," + DayData::date + ") AS year")
	int year;
};


#pragma db view\
	query("SELECT month, MAX(sum_dayyield) AS max, AVG(sum_dayyield) AS avg, MIN(sum_dayyield) AS min FROM, count(*) AS count "\
			"(SELECT strftime('%m', date) AS month, strftime('%Y', date) AS year,  SUM(day_yield) AS sum_dayyield, count(*) as count "\
			"FROM day_data GROUP BY month, year HAVING count >= 28) GROUP BY month;")
struct MonthStats {
	//FIXME: support other databases than sqlite
	#pragma db type("INTEGER")
	int month;

	#pragma db type("INTEGER")
	int64_t max;

	#pragma db type("INTEGER")
	int64_t avg;

	#pragma db type("INTEGER")
	int64_t min;

	#pragma db type("INTEGER")
	int count;
};

#pragma db view object(DayData) object(Inverter)\
		query((?) + "GROUP BY year," + Inverter::id)
struct DayDataYear {
	#pragma db column("sum(" + DayData::dayYield + ") AS yield")
	int64_t yield;

	//FIXME: support other databases than sqlite
	#pragma db column("strftime('%Y'," + DayData::date + ") AS year")
	int year;

	#pragma db column(Inverter::id)
	int64_t inverterId;
};

Json::Value toJson(const DayStats& dayStats);

Json::Value toJson(const MonthStats& monthStats);

Json::Value toJson(const DayData& dayData);

} //namespace model {


#endif /* SRC_PVLOG_MODELS_DAYDATA_H_ */
