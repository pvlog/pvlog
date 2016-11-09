#ifndef SRC_PVLOG_MODELS_PLANT_H_
#define SRC_PVLOG_MODELS_PLANT_H_

#include <cstdint>
#include <string>
#include <memory>
#include <utility>
#include <vector>

#include <odb/core.hxx>
#include <odb/nullable.hxx>
#include <jsoncpp/json/value.h>

#include "Inverter.h"

namespace model {

class Inverter;

#pragma db object
struct Plant {
	#pragma db id
	int64_t id;

	std::string name;
	std::string connection;
	std::string protocol;

	std::string connectionParam; //for example address
	std::string protocolParam; //for example password

	#pragma db value_not_null inverse(plant)
	std::vector<std::shared_ptr<Inverter>> inverters;

	Plant(std::string name,
	      std::string connection,
	      std::string protocol,
	      std::string connectionParam,
	      std::string protocolParam) :
			id(0),
			name(std::move(name)),
			connection(std::move(connection)),
			protocol(std::move(protocol)),
			connectionParam(std::move(connectionParam)),
			protocolParam(std::move(protocolParam)) {
		//nothing to go
	}

	Plant() : id(0) {}

};

using PlantPtr = std::shared_ptr<Plant>;

inline Json::Value toJson(const Plant& plant) {
	Json::Value json;

	json["id"]       = static_cast<Json::Int64>(plant.id);
	json["name"]     = plant.name;
	json["connection"] = plant.connection;
	json["protocol"]   = plant.protocol;
	json["connection_param"] = plant.connectionParam;
	json["protocol_param"]   = plant.protocolParam;

	return json;
}

} //namespace model {

#endif /* SRC_PVLOG_MODELS_PLANT_H_ */
