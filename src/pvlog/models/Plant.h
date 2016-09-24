#ifndef SRC_PVLOG_MODELS_PLANT_H_
#define SRC_PVLOG_MODELS_PLANT_H_

#include <cstdint>
#include <string>
#include <memory>
#include <utility>
#include <vector>

#include <odb/core.hxx>
#include <odb/nullable.hxx>

#include "Inverter.h"

namespace model {

class Inverter;

#pragma db object
struct Plant {
	#pragma db id
	int64_t id;

	std::string name;
	std::string connection;
	std::string conectionParameter;
	std::string protocol;
	std::string protocolParameter;
	std::string password;

	#pragma db value_not_null inverse(plant)
	std::vector<std::shared_ptr<Inverter>> inverters;

	Plant(std::string name,
	      std::string connection,
	      std::string connectionParameter,
	      std::string protocol,
	      std::string protocolParameter,
	      std::string password) :
			id(0),
			name(std::move(name)),
			connection(std::move(connection)),
			conectionParameter(std::move(connectionParameter)),
			protocol(std::move(protocol)),
			protocolParameter(std::move(protocolParameter)),
			password(std::move(password)) {
		//nothing to go
	}

	Plant() {}

};

} //namespace model {

#endif /* SRC_PVLOG_MODELS_PLANT_H_ */
