#ifndef SRC_PVLOG_MODELS_SPOTDATA_H_
#define SRC_PVLOG_MODELS_SPOTDATA_H_

#include <cstdint>
#include <unordered_map>
#include <ostream>

#include <odb/core.hxx>
#include <odb/nullable.hxx>

#include "odbHelper.h"
#include "Phase.h"
#include "DcInput.h"
#include "Inverter.h"

namespace model {

#pragma db object
struct SpotData {
	#pragma db id auto
	unsigned int id;

	#pragma db not_null
	std::shared_ptr<Inverter> inverter;

	#pragma db index
	time_t time;
	uint32_t power;
	odb::nullable<uint32_t> frequency;

	#pragma db table("phase")      \
	           id_column("id")     \
	           key_column("phase") \
	           value_column("")
	std::unordered_map<int, Phase> phases;


	#pragma db table("dc_input")   \
	        id_column("id")        \
	        key_column("input") \
	        value_column("")
	std::unordered_map<int, DcInput> dcInput;

	friend std::ostream& operator<< (std::ostream& o, const SpotData& sd) {
		o << "time: " << sd.time << " power: " << sd.power << " frequency: "
		  << sd.frequency << "\n" << "ac Phases: \n";

		for (const auto& ent : sd.phases) {
			o << ent.first << ": (power: " << ent.second.power << " voltage: "
			  << ent.second.voltage << " current: " << ent.second.current << ")\n";
		}

		o << "dc Inputs:\n";
		for (const auto& ent : sd.dcInput) {
			o << ent.first << ": (power: " << ent.second.power << " voltage: "
			  << ent.second.voltage << " current: " << ent.second.current << ")\n";
		}

		return o;
	}
};

} //namespace model {

#endif /* SRC_PVLOG_MODELS_SPOTDATA_H_ */
