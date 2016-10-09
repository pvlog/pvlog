#ifndef SRC_PVLOG_MODELS_SPOTDATA_H_
#define SRC_PVLOG_MODELS_SPOTDATA_H_

#include <cstdint>
#include <unordered_map>
#include <memory>
#include <ostream>

#include <jsoncpp/json/value.h>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <odb/core.hxx>

#include "odbHelper.h"
#include "Phase.h"
#include "DcInput.h"
#include "Inverter.h"
#include "Utility.h"

namespace model {

#pragma db object
struct SpotData {
	#pragma db id auto
	unsigned int id;

	#pragma db not_null
	std::shared_ptr<Inverter> inverter;

	#pragma db index
	boost::posix_time::ptime time;

	int32_t power;
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
//		o << "Inverter: " << sd.inverter->id << ": \n";
//		o << "time: " << sd.time << " power: " << sd.power << " frequency: "
//		  << sd.frequency << "\n" << "ac Phases: \n";
//
//		for (const auto& ent : sd.phases) {
//			o << ent.first << ": (power: " << ent.second.power << " voltage: "
//			  << ent.second.voltage << " current: " << ent.second.current << ")\n";
//		}
//
//		o << "dc Inputs:\n";
//		for (const auto& ent : sd.dcInputs) {
//			o << ent.first << ": (power: " << ent.second.power << " voltage: "
//			  << ent.second.voltage << " current: " << ent.second.current << ")\n";
//		}

		return o;
	}
};

using SpotDataPtr = std::shared_ptr<SpotData>;

inline Json::Value toJson(const SpotData& spotData) {
	using util::toJson;
	Json::Value json;

	json["inverter"]  = static_cast<Json::Int64>(spotData.inverter->id);
	//json["time"]      = static_cast<Json::Int64>(boost::posix_time::to_time_t(spotData.time));
	json["power"]     = spotData.power;
	json["frequency"] = toJson(spotData.frequency);

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

		phases[std::to_string(input)] = toJson(dcInput);
	}

	json["phases"]    = phases;
	json["dc_inputs"] = dcInputs;

	return json;
}

} //namespace model {

#endif /* SRC_PVLOG_MODELS_SPOTDATA_H_ */
