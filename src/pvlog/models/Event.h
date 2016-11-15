#ifndef SRC_PVLOG_MODELS_EVENT_H_
#define SRC_PVLOG_MODELS_EVENT_H_

#include <string>

#include <jsoncpp/json/value.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <odb/core.hxx>

#include "Inverter.h"

namespace model {

#pragma db object
struct Event {
	#pragma db id auto
	int id;

	#pragma db not_null
	std::shared_ptr<Inverter> inverter;

	boost::posix_time::ptime time;
	int32_t number;
	std::string message;

	Event(InverterPtr inverter, boost::posix_time::ptime time, int32_t number, std::string message) :
			inverter(inverter),
			time(time),
			number(number),
			message(std::move(message)) {
		//nothing to do
	}

	Event() :
			id(0),
			number(0) {
		//nothing to do
	}
};

inline Json::Value toJson(const Event& e) {
	Json::Value json;

	json["time"]    = static_cast<Json::Int64>(boost::posix_time::to_time_t(e.time));
	json["number"]  = e.number;
	json["message"] = e.message;

	return json;
}

} //namespace model {


#endif /* SRC_PVLOG_MODELS_EVENT_H_ */
