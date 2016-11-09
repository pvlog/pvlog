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

	boost::optional<boost::posix_time::ptime> archiveLastRead;

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

inline Json::Value toJson(const Inverter& inverter) {
	Json::Value json;

	json["id"]       = static_cast<Json::Int64>(inverter.id);
	//json["plant_id"] = static_cast<Json::Int64>(inverter.plant->id);
	json["name"]     = inverter.name;
	json["wattpeak"] = inverter.wattpeak;
	json["phase_number"]   = inverter.phaseCount;
	json["tracker_number"] = inverter.trackerCount;

	return json;
}

} //namespace model


#endif /* SRC_PVLOG_MODELS_INVERTER_H_ */
