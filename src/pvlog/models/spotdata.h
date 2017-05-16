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

#ifndef SRC_PVLOG_MODELS_SPOTDATA_H_
#define SRC_PVLOG_MODELS_SPOTDATA_H_

#include <cstdint>
#include <unordered_map>
#include <memory>
#include <ostream>

#include <jsoncpp/json/value.h>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <odb/core.hxx>

#include "utility.h"
#include "odbHelper.h"
#include "version.h"

#include "phase.h"
#include "dcinput.h"
#include "inverter.h"

namespace model {

#pragma db object
struct SpotData {
	#pragma db id auto
	unsigned int id;

	#pragma db not_null
	std::shared_ptr<Inverter> inverter;

	#pragma db index type("INTEGER")
	boost::posix_time::ptime time;

	int32_t power; //power in W

	boost::optional<int32_t> dayYield; //current day Yield in Wh

	boost::optional<int32_t> frequency;

	#pragma db table("phase")      \
	           id_column("id")     \
	           key_column("phase") \
	           value_column("")
	std::unordered_map<int, Phase> phases;


	#pragma db table("dc_input")   \
	        id_column("id")        \
	        key_column("input") \
	        value_column("")
	std::unordered_map<int, DcInput> dcInputs;

	friend std::ostream& operator<< (std::ostream& o, const SpotData& sd) {
		o << "Inverter: " << sd.inverter->id << ": \n";
		o << "time: " << boost::posix_time::to_simple_string(sd.time) << " power: " << sd.power << "W, frequency: "
		  << sd.frequency << "Hz, dayYield "  << sd.dayYield << "Wh\n" << "ac Phases: \n";

		for (const auto& ent : sd.phases) {
			o << ent.first << ": (power: " << ent.second.power << " voltage: "
			  << ent.second.voltage << "mV, current: " << ent.second.current << "mA)\n";
		}

		o << "dc Inputs:\n";
		for (const auto& ent : sd.dcInputs) {
			o << ent.first << ": (power: " << ent.second.power << " voltage: "
			  << ent.second.voltage << "mV, current: " << ent.second.current << "mA)\n";
		}

		return o;
	}
};

using SpotDataPtr = std::shared_ptr<SpotData>;

inline Json::Value toJson(const SpotData& spotData) {
	Json::Value json;

	json["power"]     = spotData.power;
	if (spotData.frequency) {
		json["frequency"] = spotData.frequency.get();
	}
	if (spotData.dayYield) {
		json["dayYield"]  = spotData.dayYield.get();
	}

	Json::Value phases;
	for (const auto& phaseEntry : spotData.phases) {
		int phaseNum       = phaseEntry.first;
		const Phase& phase = phaseEntry.second;

		phases[std::to_string(phaseNum)] = toJson(phase);
	}

	Json::Value dcInputs;
	for (const auto& dcInputEntry : spotData.dcInputs) {
		int input              = dcInputEntry.first;
		const DcInput& dcInput = dcInputEntry.second;

		dcInputs[std::to_string(input)] = toJson(dcInput);
	}

	json["phases"]    = phases;
	json["dc_inputs"] = dcInputs;

	return json;
}

} //namespace model {

#endif /* SRC_PVLOG_MODELS_SPOTDATA_H_ */
