#ifndef SRC_PVLOG_MODELS_DAYDATA_H_
#define SRC_PVLOG_MODELS_DAYDATA_H_

#include <memory>

#include <jsoncpp/json/value.h>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "Inverter.h"

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

Json::Value toJson(const DayData& dayData) {
	Json::Value json;

	json["inverter"] =  static_cast<Json::Int64>(dayData.inverter->id);
	json["date"]     = boost::gregorian::to_iso_string(dayData.date);
	json["day_yield"] = static_cast<Json::Int64>(dayData.dayYield);

	return json;
}

} //namespace model {


#endif /* SRC_PVLOG_MODELS_DAYDATA_H_ */
