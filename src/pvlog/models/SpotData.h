#ifndef SRC_PVLOG_MODELS_SPOTDATA_H_
#define SRC_PVLOG_MODELS_SPOTDATA_H_

#include <cstdint>
#include <unordered_map>

#include <odb/core.hxx>
#include <odb/nullable.hxx>

#include "Phase.h"
#include "DcInput.h"

namespace model {

#pragma db object
struct SpotData {
	#pragma db id auto
	unsigned int id;

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
};

} //namespace model {

#endif /* SRC_PVLOG_MODELS_SPOTDATA_H_ */
