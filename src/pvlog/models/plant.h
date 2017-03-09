#ifndef SRC_PVLOG_MODELS_PLANT_H_
#define SRC_PVLOG_MODELS_PLANT_H_

#include <inverter.h>
#include <cstdint>
#include <string>
#include <memory>
#include <utility>
#include <vector>

#include <odb/core.hxx>
#include <odb/nullable.hxx>
#include <jsoncpp/json/value.h>


namespace model {

class Inverter;

#pragma db object
struct Plant {
	#pragma db id auto
	int64_t id;

	std::string name;
	std::string connection;
	std::string protocol;

	std::string connectionParam; //for example address
	std::string protocolParam; //for example password

	#pragma db inverse(plant)
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

	Plant() : id(-1) {}

};

using PlantPtr = std::shared_ptr<Plant>;

Json::Value toJson(const Plant& plant);

Plant plantFromJson(const Json::Value& value);

} //namespace model {

#endif /* SRC_PVLOG_MODELS_PLANT_H_ */
