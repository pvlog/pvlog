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


inline Json::Value toJson(const DayData& dayData) {
	Json::Value json;

	json["inverter"] =  static_cast<Json::Int64>(dayData.inverter->id);
	json["date"]     = boost::gregorian::to_iso_string(dayData.date);
	json["yield"] = static_cast<Json::Int64>(dayData.dayYield);

	return json;
}

} //namespace model {


#endif /* SRC_PVLOG_MODELS_DAYDATA_H_ */
