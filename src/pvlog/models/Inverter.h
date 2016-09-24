#ifndef SRC_PVLOG_MODELS_INVERTER_H_
#define SRC_PVLOG_MODELS_INVERTER_H_

#include <cstdint>
#include <string>
#include <memory>
#include <utility>

#include <odb/core.hxx>

#include "Plant.h"

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

	Inverter(int64_t id, std::string name, int32_t wattpeak, int phaseCount, int trackerCount) :
			id(id),
			name(name),
			wattpeak(wattpeak),
			phaseCount(phaseCount),
			trackerCount(trackerCount) {
		//nothing to do
	}

	Inverter() {}

};

} //namespace model


#endif /* SRC_PVLOG_MODELS_INVERTER_H_ */
