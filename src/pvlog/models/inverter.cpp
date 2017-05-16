/*
 * This file is part of Pvlog.
 *
 * Copyright (C) 2017 pvlogdev@gmail.com
 *
 * Pvlog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pvlog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pvlog.  If not, see <http://www.gnu.org/licenses/>.
 */

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
