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

#ifndef SRC_PVLOG_MODELS_PHASE_H_
#define SRC_PVLOG_MODELS_PHASE_H_

#include <cstdint>

#include <jsoncpp/json/value.h>
#include <boost/optional.hpp>
#include <odb/core.hxx>

#include "version.h"
#include "utility.h"

namespace model {

#pragma db value
struct Phase {
	int32_t power;
	boost::optional<int32_t> voltage;
	boost::optional<int32_t> current;
};

inline Json::Value toJson(const Phase& phase) {
	Json::Value json;

	json["power"] = phase.power;
	if (phase.voltage) {
		json["voltage"] = phase.voltage.get();
	}
	if (phase.current) {
		json["current"] = phase.current.get();
	}

	return json;
}

} //namespace model {

#endif /* SRC_PVLOG_MODELS_PHASE_H_ */
