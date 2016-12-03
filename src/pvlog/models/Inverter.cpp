#include "Inverter.h"

namespace model {

Json::Value toJson(const Inverter& inverter) {
	Json::Value json;

	json["id"]       = static_cast<Json::Int64>(inverter.id);
	//json["plant_id"] = static_cast<Json::Int64>(inverter.plant->id);
	json["name"]     = inverter.name;
	json["wattpeak"] = inverter.wattpeak;
	json["phases"]   = inverter.phaseCount;
	json["trackers"] = inverter.trackerCount;

	return json;
}

inline Inverter inverterFromJson(const Json::Value& value) {
	Inverter inverter;

	inverter.id   = value["id"].asInt64();
	inverter.name = value["name"].asString();
	inverter.wattpeak     = value["wattpeak"].asInt();
	inverter.phaseCount   = value["phases"].asInt();
	inverter.trackerCount = value["trackers"].asInt();

	return inverter;
}

} //namespace model
