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

#ifndef SRC_PVLOG_MODELS_DCINPUT_H_
#define SRC_PVLOG_MODELS_DCINPUT_H_

#include <cstdint>

#include <jsoncpp/json/value.h>
#include <boost/optional.hpp>
#include <odb/core.hxx>

#include "utility.h"
#include "version.h"

namespace model {

#pragma db value
struct DcInput {
	boost::optional<int32_t> power;
	boost::optional<int32_t> voltage;
	boost::optional<int32_t> current;
};

inline Json::Value toJson(const DcInput& dcInput) {
	Json::Value json;

	if (dcInput.power) {
		json["power"] = dcInput.power.get();
	}
	if (dcInput.voltage) {
		json["voltage"] = dcInput.voltage.get();
	}
	if (dcInput.current) {
		json["current"] = dcInput.current.get();
	}

	return json;
}

} //namespace model {

#endif /* SRC_PVLOG_MODELS_DCINPUT_H_ */
