#include <plant.h>

namespace model {

Json::Value toJson(const Plant& plant) {
	Json::Value json;

	json["id"]       = static_cast<Json::Int64>(plant.id);
	json["name"]     = plant.name;
	json["connection"] = plant.connection;
	json["protocol"]   = plant.protocol;
	json["connectionParam"] = plant.connectionParam;
	json["protocolParam"]   = plant.protocolParam;

	return json;
}

Plant plantFromJson(const Json::Value& value) {
	Plant plant;

	if (value.isMember("id") && !value["id"].isNull()) {
		plant.id = value["id"].asInt64();
	}
	plant.name       = value["name"].asString();
	plant.connection = value["connection"].asString();
	plant.protocol   = value["protocol"].asString();
	plant.connectionParam = value["connectionParam"].asString();
	plant.protocolParam   = value["protocolParam"].asString();

	return plant;
}


} //namespace model
