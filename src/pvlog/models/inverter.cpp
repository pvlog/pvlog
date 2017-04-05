#include "inverter.h"
#include "plant.h"

namespace model {

Json::Value toJson(const Inverter& inverter) {
	Json::Value json;

	json["id"]       = static_cast<Json::Int64>(inverter.id);
	json["plantId"]  = static_cast<Json::Int64>(inverter.plant.lock()->id);
	json["name"]     = inverter.name;
	json["wattpeak"] = inverter.wattpeak;
	json["phases"]   = inverter.phaseCount;
	json["trackers"] = inverter.trackerCount;

	return json;
}

Inverter inverterFromJson(const Json::Value& value) {
	Inverter inverter;

	inverter.id   = value["id"].asInt64();
	inverter.name = value["name"].asString();
	inverter.wattpeak     = value["wattpeak"].asInt();
	inverter.phaseCount   = value["phases"].asInt();
	inverter.trackerCount = value["trackers"].asInt();

	return inverter;
}

} //namespace model
