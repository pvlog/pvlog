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

#ifndef SRC_PVLOG_MODELS_EVENT_H_
#define SRC_PVLOG_MODELS_EVENT_H_

#include <string>

#include <jsoncpp/json/value.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <odb/core.hxx>

#include "inverter.h"
#include "timeutil.h"
#include "version.h"

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
			id(0),
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
