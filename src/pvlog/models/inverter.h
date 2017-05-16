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

#ifndef SRC_PVLOG_MODELS_INVERTER_H_
#define SRC_PVLOG_MODELS_INVERTER_H_

#include <cstdint>
#include <string>
#include <memory>
#include <utility>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/optional.hpp>
#include <jsoncpp/json/value.h>
#include <odb/core.hxx>

#include "version.h"

#include "plant.h"

namespace model {

class Plant;

#pragma db object
struct Inverter {
	#pragma db id
	int64_t id;

	#pragma db not_null
	std::weak_ptr<Plant> plant;

	std::string name;
	int32_t wattpeak;
	int phaseCount;
	int trackerCount;

	boost::optional<boost::posix_time::ptime> dayArchiveLastRead;
	boost::optional<boost::posix_time::ptime> eventArchiveLastRead;

	Inverter(int64_t id, std::string name, int32_t wattpeak, int phaseCount, int trackerCount) :
			id(id),
			name(name),
			wattpeak(wattpeak),
			phaseCount(phaseCount),
			trackerCount(trackerCount) {
		//nothing to do
	}

	Inverter() : id(0), wattpeak(0), phaseCount(0), trackerCount(0) {}

};

using InverterPtr = std::shared_ptr<Inverter>;

Json::Value toJson(const Inverter& inverter);

Inverter inverterFromJson(const Json::Value& value);

} //namespace model


#endif /* SRC_PVLOG_MODELS_INVERTER_H_ */
